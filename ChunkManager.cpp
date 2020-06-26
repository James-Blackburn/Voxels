#include "ChunkManager.h"

FastNoise ChunkManager::terrainSimplex;
FastNoise ChunkManager::terrainPerlin;
FastNoise ChunkManager::caveSimplex;
FastNoise ChunkManager::biomePerlin;

std::mutex ChunkManager::chunkMutex;
std::mutex ChunkManager::cameraMutex;

std::vector<std::vector<int>> ChunkManager::heightmap;
std::vector<std::vector<Chunk>> ChunkManager::chunks;
std::vector<glm::ivec2> ChunkManager::activeChunks;

void ChunkManager::initializeNoise()
{
	terrainSimplex.SetSeed(SEED);
	terrainPerlin.SetSeed(SEED);
	caveSimplex.SetSeed(SEED);
    biomePerlin.SetSeed(SEED);

	terrainSimplex.SetNoiseType(FastNoise::Simplex);
	terrainPerlin.SetNoiseType(FastNoise::Perlin);
	caveSimplex.SetNoiseType(FastNoise::SimplexFractal);
    biomePerlin.SetNoiseType(FastNoise::Perlin);
    biomePerlin.SetFrequency(0.005);
}

void ChunkManager::initializeChunks()
{
    chunks.resize(chunkAmountX);
    for (int x = 0; x < chunkAmountX; x++)
    {
        chunks[x] = std::vector<Chunk>(chunkAmountZ);
        for (int z = 0; z < chunkAmountZ; z++)
        {
            chunks[x][z] = Chunk(x * Chunk::chunkSizeX, 0, z * Chunk::chunkSizeZ);
        }
    }

    // generate heightmap
    heightmap.resize(chunkAmountX * Chunk::chunkSizeX);
    for (int x = 0; x < chunkAmountX * Chunk::chunkSizeX; x++)
    {
        heightmap[x] = std::vector<int>(chunkAmountZ * Chunk::chunkSizeZ);
        for (int z = 0; z < chunkAmountZ * Chunk::chunkSizeZ; z++)
        {
            // Only sample 4x4
            if (x % 4 == 0 && z % 4 == 0)
            {
                BIOMES biome = ChunkManager::getBiome(x, z);
                double terrainSimplexNoise = (ChunkManager::terrainSimplex.GetNoise(x, z) + 1) / 2;
                double terrainPerlinNoise = (ChunkManager::terrainPerlin.GetNoise(x, z) + 1) / 2;

                int height = 0;
                if (biome == BIOMES::HILLS)
                    height = 36 + (terrainSimplexNoise * 8) * (terrainPerlinNoise * 8);
                else if (biome == BIOMES::PLAINS)
                    height = 32 + (terrainSimplexNoise * 8) + (terrainPerlinNoise * 8);

                heightmap[x][z] = height;
            }
            else
            {
                heightmap[x][z] = 0;
            }
        }
    }

    // Use bilinear interpolation to fill in heightmap
    int width = 4;  int height = 4;
    for (int i = 0; i < chunkAmountX * Chunk::chunkSizeX - 4; i += 4)
    {
        for (int j = 0; j < chunkAmountZ * Chunk::chunkSizeZ - 4; j += 4)
        {
            int topLeft = heightmap[i + 4][j + 4];
            int topRight = heightmap[i][j + 4];
            int bottomLeft = heightmap[i + 4][j];
            int bottomRight = heightmap[i][j];

            for (int x = 0; x < 4; x++)
            {
                for (int z = 0; z < 4; z++)
                {
                    if (heightmap[x + i][z + j] != 0) continue;

                    int xDistanceToMaxValue = width - x;
                    int zDistanceToMaxValue = height - z;
                    int xDistanceToMinValue = x;
                    int zDistanceToMinValue = z;

                    int interpolatedValue = 1.0f / (width * height) *
                        (
                            bottomRight * xDistanceToMaxValue * zDistanceToMaxValue +
                            bottomLeft * xDistanceToMinValue * zDistanceToMaxValue +
                            topRight * xDistanceToMaxValue * zDistanceToMinValue +
                            topLeft * xDistanceToMinValue * zDistanceToMinValue
                        );

                    heightmap[x + i][z + j] = interpolatedValue;
                }
            }

        }
    }

}

// Create thread to update chunks
void ChunkManager::startUpdatingChunks()
{
    chunkThread = new std::thread(&ChunkManager::updateChunksThreaded, this);
}

// Join the chunk thread and delete
void ChunkManager::stopUpdatingChunks()
{
    if (chunkThread != nullptr)
    {
        chunkThread->join();
        delete chunkThread;
    }
}

void ChunkManager::updateChunksThreaded()
{
    while (!glfwWindowShouldClose(window))
    {
        std::vector<glm::ivec2> meshesToBuild;
        for (int x = 0; x < chunkAmountX; x++)
        {
            for (int z = 0; z < chunkAmountZ; z++)
            {
                cameraMutex.lock();
                const int dX = (chunks[x][z].getChunkCenterX() / Chunk::chunkSizeX - (int)camera->getCameraPosition().x / Chunk::chunkSizeX);
                const int dZ = (chunks[x][z].getChunkCenterZ() / Chunk::chunkSizeZ - (int)camera->getCameraPosition().z / Chunk::chunkSizeZ);
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
                        chunks[x][z].generateChunk();
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
                        glfwMakeContextCurrent(window);
                        chunks[x][z].deleteChunkMeshData();
                        glfwMakeContextCurrent(NULL);
                    }
                }
                else if (distance > renderDistance + 4)
                {
                    if (chunks[x][z].active)
                    {
                        std::lock_guard<std::mutex> lock(chunkMutex);
                        glfwMakeContextCurrent(window);
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
            glfwMakeContextCurrent(window);
            chunks[x][z].generateChunkMesh(chunks, x, z);
            chunks[x][z].sendChunkMeshData();
            glfwMakeContextCurrent(NULL);
            chunkMutex.unlock();
        }
        activeChunks.shrink_to_fit();
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ChunkManager::drawChunks(GLuint uModelLocation)
{
    for (glm::ivec2& vec : activeChunks)
    {
        Chunk* chunk = &chunks[vec[0]][vec[1]];
        if (chunk->viewable)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(chunk->getChunkXPos(), chunk->getChunkYPos(), chunk->getChunkZPos()));
            glUniformMatrix4fv(uModelLocation, 1, GL_FALSE, glm::value_ptr(model));
            chunk->drawChunk();
        }
    }
}

BIOMES ChunkManager::getBiome(int x, int z)
{
    double biomeNoise = biomePerlin.GetNoise(x, z);
    if (biomeNoise > 0)
        return BIOMES::PLAINS;
    else
        return BIOMES::HILLS;
}

ChunkManager::~ChunkManager()
{
    stopUpdatingChunks();
}