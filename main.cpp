#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <ctime>
#include <random>
#include <thread>
#include <mutex>

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

const int chunkAmountX = 2048;
const int chunkAmountZ = 2048;

const int SEED = 325;

FastNoise noise;

std::vector<glm::vec2> pendingMeshes;
std::vector<glm::vec2> activeChunks;

std::mutex chunkMutex;

void generateChunks()
{
    double t = glfwGetTime();
    noise.SetSeed(SEED);
    noise.SetNoiseType(FastNoise::Simplex);

    chunks.resize(chunkAmountX);
    for (int x = 0; x < chunkAmountX; x++)
    {
        chunks[x] = std::vector<Chunk>(chunkAmountZ);
        for (int z = 0; z < chunkAmountZ; z++)
        {
            chunks[x][z] = Chunk(x * Chunk::chunkSizeX, 0, z * Chunk::chunkSizeZ);
            const int dX = (chunks[x][z].getChunkCenterX() / Chunk::chunkSizeX - (int)camera.getCameraPosition().x / Chunk::chunkSizeZ);
            const int dZ = (chunks[x][z].getChunkCenterZ() / Chunk::chunkSizeZ - (int)camera.getCameraPosition().z / Chunk::chunkSizeZ);
            const double distance = sqrt(dX * dX + dZ * dZ);
            if (distance <= 12)
            {
                chunks[x][z].active = true;
                activeChunks.push_back(glm::vec2(x, z));
            }
            if (distance > 8 && distance <= 12)
            {
                chunks[x][z].generateChunk(noise);
                chunks[x][z].generated = true;
            }
            else if (distance <= 8)
            {
                chunks[x][z].viewable = true;
                chunks[x][z].generateChunk(noise);
                chunks[x][z].generated = true;
            }
        }
    }
    std::cout << "  Generate Chunks: " << glfwGetTime() - t << std::endl;

    t = glfwGetTime();
    for (int x = 0; x < chunkAmountX; x++)
    {
        for (int z = 0; z < chunkAmountZ; z++)
        {
            if (chunks[x][z].viewable && chunks[x][z].generated && chunks[x][z].active)
            {
                chunks[x][z].generateChunkMesh(chunks, x, z);
                chunks[x][z].sendChunkMeshData();
            }
                
        }
    }
    std::cout << "  Generate Chunk Meshes: " << glfwGetTime() - t << std::endl;
}

void updateChunks()
{
    while (!glfwWindowShouldClose(mainWindow))
    {
        for (int x = 0; x < chunkAmountX; x++)
        {
            for (int z = 0; z < chunkAmountZ; z++)
            {
                chunkMutex.lock();
                const int dX = (chunks[x][z].getChunkCenterX() / Chunk::chunkSizeX - (int)camera.getCameraPosition().x / Chunk::chunkSizeX);
                const int dZ = (chunks[x][z].getChunkCenterZ() / Chunk::chunkSizeZ - (int)camera.getCameraPosition().z / Chunk::chunkSizeZ);
                chunkMutex.unlock();
                const double distance = sqrt(dX * dX + dZ * dZ);
                
                if (distance <= 12 && chunks[x][z].active == false)
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
                    activeChunks.push_back(glm::vec2(x, z));
                    chunks[x][z].active = true;
                }
                if (distance > 8 && distance <= 12 && chunks[x][z].generated == false)
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
                    chunks[x][z].generateChunk(noise);
                    chunks[x][z].generated = true;
                }
                if (distance <= 8 && chunks[x][z].generated == false)
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
                    chunks[x][z].generateChunk(noise);
                    chunks[x][z].generated = true;
                    chunks[x][z].pending = true;
                }
                if (distance <= 8 && chunks[x][z].pending == false)
                {
                    chunks[x][z].pending = true;
                }
                if (distance > 8 && chunks[x][z].viewable)
                {
                    chunks[x][z].viewable = false;
                }
                if (distance > 12 && chunks[x][z].active)
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
                    chunks[x][z].clearChunk();
                    chunks[x][z].generated = false;
                    chunks[x][z].active = false;

                    for (int i = 0; i < activeChunks.size(); i++)
                    {
                        if (activeChunks[i][0] == x && activeChunks[i][1] == z)
                        {
                            activeChunks.erase(activeChunks.begin() + i);
                            break;
                        }
                    }
                }
            }
        }

        for (int x = 0; x < chunkAmountX; x++)
        {
            for (int z = 0; z < chunkAmountZ; z++)
            {
                if (chunks[x][z].pending)
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
                    pendingMeshes.push_back(glm::vec2(x, z));
                    chunks[x][z].generateChunkMesh(chunks, x, z);
                    chunks[x][z].pending = false;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 100.0f, 0.05f);

    textureAtlas.loadTexture("Textures/dirt.png", GL_REPEAT, GL_NEAREST_MIPMAP_LINEAR , GL_NEAREST, GL_RGB, GL_RGB);
    generateChunks();
    

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 1.0f, 500.0f);
    glUseProgram(shaderProgram.getID());
    textureAtlas.useTexture();
    std::thread chunkThread(updateChunks);
    while (!glfwWindowShouldClose(mainWindow))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        camera.moveCamera(keyStates, deltaTime);

        chunkMutex.lock();
        for (int i=0; i<pendingMeshes.size(); i++)
        {
            Chunk* chunk = &chunks[pendingMeshes[i][0]][pendingMeshes[i][1]];
            chunk->deleteChunkMeshData();
            chunk->sendChunkMeshData();
            chunk->viewable = true;
        }
        pendingMeshes.clear();
        chunkMutex.unlock();

        // set Model View Projection
        glm::mat4 mvp = projection * camera.calculateViewMatrix() * model;
        glUniformMatrix4fv(uniformMVPLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        chunkMutex.lock();
        for (glm::vec2& vec : activeChunks)
        {
            if (chunks[vec[0]][vec[1]].viewable)
                chunks[vec[0]][vec[1]].drawChunk();
        }
        chunkMutex.unlock();

        glfwSwapBuffers(mainWindow);
        
    }
    chunkThread.join();
    glfwTerminate();
    return 0;
}
