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
const GLint WIDTH = 1366, HEIGHT = 768;
int bufferWidth, bufferHeight;
GLFWwindow* mainWindow = nullptr;
Shader shaderProgram;
Texture textureAtlas;
std::vector<std::vector<Chunk>> chunks;
Camera camera;

GLuint uniformViewProjectionLocation;
GLuint uniformModelLocation;

float lastTime = 0.0f;
bool keyStates[1024]{ false };
bool wireView = false;

const int chunkAmountX = 1024;
const int chunkAmountZ = 1024;
const int renderDistance = 16;

const int SEED = 325;

FastNoise n1;
FastNoise n2;
FastNoise n3;
FastNoise n4;

std::vector<glm::ivec2> activeChunks;
std::mutex chunkMutex;
std::mutex cameraMutex;

void generateChunks()
{
    double t = glfwGetTime();
    n1.SetSeed(SEED);   n2.SetSeed(SEED);   n3.SetSeed(SEED);   n4.SetSeed(SEED);
    n1.SetNoiseType(FastNoise::Simplex);
    n2.SetNoiseType(FastNoise::Perlin);
    n3.SetNoiseType(FastNoise::Simplex);
    n4.SetNoiseType(FastNoise::Perlin);

    chunks.resize(chunkAmountX);
    for (int x = 0; x < chunkAmountX; x++)
    {
        chunks[x] = std::vector<Chunk>(chunkAmountZ);
        for (int z = 0; z < chunkAmountZ; z++)
        {
            chunks[x][z] = Chunk(x * Chunk::chunkSizeX, 0, z * Chunk::chunkSizeZ);
        }
    }
}

void updateChunks()
{
    while (!glfwWindowShouldClose(mainWindow))
    {
        std::vector<glm::ivec2> meshesToBuild;
        for (int x = 0; x < chunkAmountX; x++)
        {
            for (int z = 0; z < chunkAmountZ; z++)
            {
                cameraMutex.lock();
                const int dX = (chunks[x][z].getChunkCenterX() / Chunk::chunkSizeX - (int)camera.getCameraPosition().x / Chunk::chunkSizeX);
                const int dZ = (chunks[x][z].getChunkCenterZ() / Chunk::chunkSizeZ - (int)camera.getCameraPosition().z / Chunk::chunkSizeZ);
                cameraMutex.unlock();
                const double distance = sqrt(dX * dX + dZ * dZ);

                if (distance <= renderDistance + 4)
                {
                    if (chunks[x][z].active == false)
                    {
                        std::lock_guard<std::mutex> lock(chunkMutex);
                        activeChunks.push_back(glm::ivec2(x, z));
                        chunks[x][z].active = true;
                    }

                    if (chunks[x][z].generated == false)
                    {
                        chunks[x][z].generateChunk(n1, n2, n3, n4);
                        chunks[x][z].generated = true;
                    }

                    if (distance <= renderDistance && !chunks[x][z].viewable)
                    {
                        chunks[x][z].viewable = true;
                        meshesToBuild.push_back(glm::ivec2(x, z));
                    }
                    else if (distance > renderDistance && chunks[x][z].viewable)
                    {
                        std::lock_guard<std::mutex> lock(chunkMutex);
                        chunks[x][z].viewable = false;
                        glfwMakeContextCurrent(mainWindow);
                        chunks[x][z].deleteChunkMeshData();
                        glfwMakeContextCurrent(NULL);
                    }
                }
                else if (distance > renderDistance + 4)
                {
                    if (chunks[x][z].active)
                    {
                        std::lock_guard<std::mutex> lock(chunkMutex);
                        glfwMakeContextCurrent(mainWindow);
                        chunks[x][z].deleteChunkMeshData();
                        glfwMakeContextCurrent(NULL);
                        chunks[x][z].clearChunk();
                        

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
        }
        
        // Build meshes
        for (glm::ivec2& vec : meshesToBuild)
        {
            int x = vec[0]; int z = vec[1];
            chunkMutex.lock();
            glfwMakeContextCurrent(mainWindow);
            chunks[x][z].generateChunkMesh(chunks, x, z);
            chunks[x][z].sendChunkMeshData();
            glfwMakeContextCurrent(NULL);
            chunkMutex.unlock();
        }
        activeChunks.shrink_to_fit();
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
        std::lock_guard<std::mutex> mtx(chunkMutex);
        glfwMakeContextCurrent(mainWindow);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glfwMakeContextCurrent(NULL);
        wireView = true;
    }
        
    else if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_ALT && wireView)
    {
        std::lock_guard<std::mutex> mtx(chunkMutex);
        glfwMakeContextCurrent(mainWindow);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glfwMakeContextCurrent(NULL);
        wireView = false;
    }

    else if (action == GLFW_PRESS && key == GLFW_KEY_BACKSPACE)
    {
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        camera.setPosition(r * chunkAmountX * Chunk::chunkSizeX, 32.0f, r * chunkAmountZ * Chunk::chunkSizeZ);
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


    mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", NULL, NULL);
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

    camera = Camera(glm::vec3(chunkAmountX * (Chunk::chunkSizeX / 2), 32.0f, chunkAmountZ * (Chunk::chunkSizeZ / 2)), 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 40.0f, 0.05f);

    textureAtlas.loadTexture("Textures/dirt.png", GL_REPEAT, GL_NEAREST_MIPMAP_LINEAR , GL_NEAREST, GL_RGB, GL_RGB);
    

    
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 1.0f, 500.0f);
    glUseProgram(shaderProgram.getID());
    textureAtlas.useTexture();
    generateChunks();
    std::thread chunkThread(updateChunks);
    while (!glfwWindowShouldClose(mainWindow))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        cameraMutex.lock();
        camera.moveCamera(keyStates, deltaTime);
        cameraMutex.unlock();

        // set Model View Projection
        glm::mat4 vp = projection * camera.calculateViewMatrix();

        chunkMutex.lock();
        
        glfwMakeContextCurrent(mainWindow);
        glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniformMatrix4fv(uniformViewProjectionLocation, 1, GL_FALSE, glm::value_ptr(vp));
        for (glm::ivec2& vec : activeChunks)
        {
            Chunk* chunk = &chunks[vec[0]][vec[1]];
            if (chunk->viewable)
            {
                glm::mat4 model(1.0f);
                model = glm::translate(model, glm::vec3(chunk->getChunkXPos(), chunk->getChunkYPos(), chunk->getChunkZPos()));
                glUniformMatrix4fv(uniformModelLocation, 1, GL_FALSE, glm::value_ptr(model));
                chunk->drawChunk();
            }
        }
        glfwSwapBuffers(mainWindow);
        glfwMakeContextCurrent(NULL);
        
        chunkMutex.unlock();
        
    }
    glUseProgram(NULL);
    chunkThread.join();
    glfwTerminate();
    return 0;
}
