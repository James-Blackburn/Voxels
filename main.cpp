#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <ctime>
#include <random>

#include "FastNoise/FastNoise.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Chunk.h"

// Window dimensions
const GLint WIDTH = 1920, HEIGHT = 1080;
int bufferWidth, bufferHeight;
GLFWwindow* mainWindow = nullptr;
Shader shaderProgram;
Texture textureAtlas;
std::vector<std::vector<Chunk>> chunks;
Camera camera;

GLuint uniformMVPLocation;

float lastTime = 0.0f;
bool keyStates[1024]{ false };
bool wireView = false;

const int chunkAmountX = 32;
const int chunkAmountZ = 32;

const int SEED = 325;

void generateChunks()
{
    double t = glfwGetTime();
    
    FastNoise noise;
    noise.SetSeed(time(NULL));
    noise.SetNoiseType(FastNoise::Simplex);

    for (int x = 0; x < chunkAmountX; x++)
    {
        std::vector<Chunk> chunkLayer;
        for (int z = 0; z < chunkAmountZ; z++)
        {
            chunkLayer.emplace_back(Chunk(x * Chunk::chunkSizeX, 0, z * Chunk::chunkSizeZ));
            const float dX = chunkLayer[z].getChunkCenterX() - camera.getCameraPosition().x;
            const float dZ = chunkLayer[z].getChunkCenterZ() - camera.getCameraPosition().z;
            const float distance = sqrt(dX * dX + dZ * dZ);
            chunkLayer[z].viewable = true;
            chunkLayer[z].generateChunk(noise);
            chunkLayer[z].generated = true;
        }
        chunks.push_back(chunkLayer);
    }
    std::cout << "  Generate Chunks: " << glfwGetTime() - t << std::endl;

    t = glfwGetTime();
    for (int x = 0; x < chunkAmountX; x++)
    {
        for (int z = 0; z < chunkAmountZ; z++)
        {
            if (chunks[x][z].viewable)
                chunks[x][z].generateChunkMesh(chunks, x, z);
        }
    }
    std::cout << "  Generate Chunk Meshes: " << glfwGetTime() - t << std::endl;
}

void handleCursorMovement(GLFWwindow* window, double xPos, double yPos)
{
    static float lastX = xPos;
    static float lastY = yPos;

    float deltaX = xPos - lastX;
    float deltaY = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    camera.mouseMovement(deltaX, deltaY);
}

void handleKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(mainWindow, true);
    else if (action == GLFW_PRESS && key == GLFW_KEY_LEFT_ALT && !wireView)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        wireView = true;
    }
        
    else if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_ALT && wireView)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        wireView = false;
    }
        
    else if (action == GLFW_PRESS)
        keyStates[key] = true;
    else if (action == GLFW_RELEASE)
        keyStates[key] = false;
}


int setupWindow()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        std::cout << "GLFW initialisation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup GLFW window properties
    // OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core Profile = No Backwards Compatability
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Allow Forward Compatibility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", glfwGetPrimaryMonitor(), NULL);
    if (!mainWindow)
    {
        std::cout << "GLFW window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Get Buffer size information
    
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    // Set context for GLEW to use
    glfwMakeContextCurrent(mainWindow);

    // Allow modern extension features
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW initialisation failed" << std::endl;
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, bufferWidth, bufferHeight);
    glEnable(GL_DEPTH_TEST);


    glfwSetCursorPosCallback(mainWindow, handleCursorMovement);
    glfwSetKeyCallback(mainWindow, handleKeyboardInput);
    glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(0);

    return 0;
}


int main()
{
    if (setupWindow() == -1)
    {
        std::cout << "Failed to setup window" << std::endl;
        return -1;
    }

    shaderProgram.addShader(GL_VERTEX_SHADER, "Shaders/VertexShader.glsl");
    shaderProgram.addShader(GL_FRAGMENT_SHADER, "Shaders/FragmentShader.glsl");
    shaderProgram.linkProgram();

    // Set uniform locations
    uniformMVPLocation = glGetUniformLocation(shaderProgram.getID(), "mvp");


    camera = Camera(glm::vec3(chunkAmountX * (Chunk::chunkSizeX / 2), 32.0f, chunkAmountZ * (Chunk::chunkSizeZ / 2)), 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 20.0f, 0.05f);

    textureAtlas.loadTexture("Textures/dirt.png", GL_REPEAT, GL_NEAREST_MIPMAP_LINEAR , GL_NEAREST, GL_RGB, GL_RGB);
    generateChunks();

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 0.1f, 1000.0f);
    glUseProgram(shaderProgram.getID());
    textureAtlas.useTexture();
    while (!glfwWindowShouldClose(mainWindow))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        camera.moveCamera(keyStates, deltaTime);

        glfwPollEvents();
        glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*
        for (std::vector<Chunk>& chunkLayer : chunks)
        {
            for (Chunk& chunk : chunkLayer)
            {
                chunk.viewable = true;
                float dX = chunk.getChunkCenterX() - camera.getCameraPosition().x;
                float dZ = chunk.getChunkCenterZ() - camera.getCameraPosition().z;
                float distance = sqrt(dX * dX + dZ * dZ);
                if (distance > 22.6 * 8)
                    chunk.viewable = false;
            }
        }
        */
        
        
        // set Model View Projection
        glm::mat4 mvp = projection * camera.calculateViewMatrix() * model;
        glUniformMatrix4fv(uniformMVPLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        
        for (std::vector<Chunk>& chunkLayer : chunks)
        {
            for (Chunk& chunk : chunkLayer)
            {
                if (chunk.viewable)
                    chunk.drawChunk();
            }
        }

        glfwSwapBuffers(mainWindow);
        
    }

    glfwTerminate();
    return 0;
}
