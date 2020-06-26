#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <ctime>
#include <random>
#include <mutex>

#include "FastNoise/FastNoise.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Chunk.h"
#include "ChunkManager.h"

// Window dimensions
const GLint WIDTH = 1920, HEIGHT = 1080;
int bufferWidth, bufferHeight;

GLFWwindow* mainWindow = nullptr;

ChunkManager chunkManager;
Camera camera;
Shader shaderProgram;
Texture textureAtlas;

GLuint uniformViewProjectionLocation;
GLuint uniformModelLocation;

float lastTime = 0.0f;
bool keyStates[1024]{ false };
bool wireView = false;

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
        std::lock_guard<std::mutex> mtx(ChunkManager::chunkMutex);
        glfwMakeContextCurrent(mainWindow);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glfwMakeContextCurrent(NULL);
        wireView = true;
    }
        
    else if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_ALT && wireView)
    {
        std::lock_guard<std::mutex> mtx(ChunkManager::chunkMutex);
        glfwMakeContextCurrent(mainWindow);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glfwMakeContextCurrent(NULL);
        wireView = false;
    }

    else if (action == GLFW_PRESS && key == GLFW_KEY_BACKSPACE)
    {
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        camera.setPosition(r * ChunkManager::chunkAmountX * Chunk::chunkSizeX, 
            32.0f, r * ChunkManager::chunkAmountZ * Chunk::chunkSizeZ);
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
    uniformViewProjectionLocation = glGetUniformLocation(shaderProgram.getID(), "viewProjection");
    uniformModelLocation = glGetUniformLocation(shaderProgram.getID(), "model");

    camera = Camera(glm::vec3(ChunkManager::chunkAmountX * (Chunk::chunkSizeX / 2), 32.0f, ChunkManager::chunkAmountZ * (Chunk::chunkSizeZ / 2)), 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 40.0f, 0.05f);

    textureAtlas.loadTexture("Textures/dirt.png", GL_REPEAT, GL_NEAREST_MIPMAP_LINEAR , GL_NEAREST, GL_RGB, GL_RGB);
    textureAtlas.useTexture();
    
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 1.0f, 500.0f);
    glUseProgram(shaderProgram.getID());

    chunkManager = ChunkManager(mainWindow, &camera);
    chunkManager.initializeNoise();
    chunkManager.initializeChunks();
    chunkManager.startUpdatingChunks();

    while (!glfwWindowShouldClose(mainWindow))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        ChunkManager::cameraMutex.lock();
        camera.moveCamera(keyStates, deltaTime);
        ChunkManager::cameraMutex.unlock();

        // draw scene
        ChunkManager::chunkMutex.lock();
        glfwMakeContextCurrent(mainWindow);

        glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set Model View Projection
        glm::mat4 vp = projection * camera.calculateViewMatrix();
        glUniformMatrix4fv(uniformViewProjectionLocation, 1, GL_FALSE, glm::value_ptr(vp));
        chunkManager.drawChunks(uniformModelLocation);

        glfwSwapBuffers(mainWindow);
        glfwMakeContextCurrent(NULL);
        ChunkManager::chunkMutex.unlock();
    }

    chunkManager.stopUpdatingChunks();

    glUseProgram(NULL);
    glfwTerminate();
    return 0;
}
