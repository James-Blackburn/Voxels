#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Chunk.h"
#include "Camera.h"

#include <thread>
#include <mutex>
#include <chrono>

enum class BIOMES
{
	PLAINS, HILLS
};

class ChunkManager
{
public:

	static FastNoise terrainSimplex;
	static FastNoise terrainPerlin;
	static FastNoise caveSimplex;
	static FastNoise biomePerlin;

	static const int chunkAmountX = 1024;
	static const int chunkAmountZ = 1024;
	static const int renderDistance = 16;
	static const int SEED = 325;

	static std::vector<std::vector<Chunk>> chunks;
	static std::vector<glm::ivec2> activeChunks;
	static std::vector<std::vector<int>> heightmap;
	static std::mutex chunkMutex;
	static std::mutex cameraMutex;

	ChunkManager() = default;
	ChunkManager(GLFWwindow* window_, Camera* camera_) :
		window(window_), camera(camera_) {}

	~ChunkManager();

	void initializeNoise();
	void initializeChunks();
	void startUpdatingChunks();
	void stopUpdatingChunks();
	void drawChunks(GLuint uModelLocation);

	static BIOMES getBiome(int x, int z);

	inline void setWindow(GLFWwindow* window_) { window = window_; }
	inline void setCamera(Camera* camera_) { camera = camera_; }

private:

	std::thread* chunkThread = nullptr;
	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;

	void updateChunksThreaded();
};