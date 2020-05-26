#pragma once

#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <random>

#include "FastNoise/FastNoise.h"
#include "data.h"

const int CHUNK_SIZE_X = 16;
const int CHUNK_SIZE_Y = 16;
const int CHUNK_SIZE_Z = 16;

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
		chunkXPos(setX), chunkYPos(setY), chunkZPos(setZ),
		chunkCenterX(setX + chunkSizeX / 2),
		chunkCenterZ(setZ + chunkSizeZ / 2) {}
	
	void generateChunk(FastNoise& noise);
	void generateChunkMesh(std::vector<std::vector<Chunk>>& chunks, int chunkX, int chunkZ);
	void drawChunk();

	inline int getChunkXPos() { return chunkXPos; }
	inline int getChunkYPos() { return chunkYPos; }
	inline int getChunkZPos() { return chunkZPos; }

	inline int getChunkCenterX() { return chunkCenterX; }
	inline int getChunkCenterZ() { return chunkCenterZ; }

	static const int chunkSizeX = 16;
	static const int chunkSizeY = 32;
	static const int chunkSizeZ = 16;

	bool viewable = true;
	bool generated = false;

private:
	int chunkXPos = 0;
	int chunkYPos = 0;
	int chunkZPos = 0;

	int chunkCenterX = 0;
	int chunkCenterZ = 0;

	int indicesCounter = 0;

	GLuint VAO, VBO, texVBO, EBO;

	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	std::vector <std::vector<std::vector<BLOCKS>>> blocks;
	//BLOCKS*** blocks;

	void addFace(BLOCKS blockType, ChunkFace face, int x, int y, int z);
	void regenerateChunkMesh();
};


