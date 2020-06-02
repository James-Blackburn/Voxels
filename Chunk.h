#pragma once


#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <random>

#include "FastNoise/FastNoise.h"
#include "data.h"


enum class ChunkFace
{
	FRONT, BACK, TOP, BOTTOM, RIGHT, LEFT
};

enum class BLOCKS
{
	AIR, DIRT, DIRT_TOP, STONE
};

class Chunk
{
public:
	Chunk() = default;
	Chunk(int setX, int setY, int setZ) : 
		chunkXPos(setX), chunkYPos(setY), chunkZPos(setZ) {}
	~Chunk();
	
	void generateChunk(FastNoise& noise);
	void generateChunkMesh(std::vector<std::vector<Chunk>>& chunks, int chunkX, int chunkZ);
	void sendChunkMeshData();
	void deleteChunkMeshData();
	void drawChunk();
	void clearChunk();

	inline int getChunkXPos() { return chunkXPos; }
	inline int getChunkYPos() { return chunkYPos; }
	inline int getChunkZPos() { return chunkZPos; }

	inline int getChunkCenterX() { return chunkXPos + chunkSizeX / 2; }
	inline int getChunkCenterZ() { return chunkZPos + chunkSizeZ / 2; }

	static const int chunkSizeX = 16;
	static const int chunkSizeY = 32;
	static const int chunkSizeZ = 16;

	bool viewable = false;
	bool generated = false;
	bool active = false;
	bool pending = false;
	bool meshGenerated = false;

private:
	int chunkXPos;
	int chunkYPos;
	int chunkZPos;

	int indicesCounter;

	GLuint VAO = 0, VBO = 0, texVBO = 0, EBO = 0;

	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	std::vector <std::vector<std::vector<BLOCKS>>> blocks;

	void addFace(BLOCKS blockType, ChunkFace face, int x, int y, int z);
};


