#include "Chunk.h"

void Chunk::generateChunk(FastNoise& n1, FastNoise& n2, FastNoise& n3, FastNoise& n4)
{
    blocks.clear();     blocks.shrink_to_fit();
    blocks.resize(chunkSizeX);
    for (int x = 0; x < chunkSizeX; x++)
    {
        std::vector<std::vector<BLOCKS>> blockXLayer(chunkSizeY);
        blocks[x] = blockXLayer;
        for (int y = 0; y < chunkSizeY; y++)
        {
            std::vector<BLOCKS> blockYLayer(chunkSizeZ, BLOCKS::AIR);
            blocks[x][y] = blockYLayer;
        }
    }
    
    for (int x = 0; x < chunkSizeX; x++)
    {
        for (int z = 0; z < chunkSizeZ; z++)
        {
            double noise1 = (n1.GetNoise(x + chunkXPos, z + chunkZPos) + 1) / 2;
            double noise2 = (n2.GetNoise(x + chunkXPos, z + chunkZPos) + 1) / 2;
            int height = 32 + (noise1 * 16) + (noise2 * 16);
            for (int y = 0; y < chunkSizeY; y++)
            {
                float gen1 = (n3.GetNoise(x + chunkXPos, z + chunkZPos, y) + 1) / 2;
                float gen2 = (n4.GetNoise(x + chunkXPos, z + chunkZPos, y) + 1) / 2;
                float gen = gen1 * gen2;
                if (y > height)
                    blocks[x][y][z] = BLOCKS::AIR;
                else if (y == height)
                    blocks[x][y][z] = BLOCKS::DIRT_TOP; 
                else if (y < height && y > height - 3)
                    blocks[x][y][z] = BLOCKS::DIRT;
                else
                    blocks[x][y][z] = BLOCKS::STONE;

                if (gen > 0.5f)
                    blocks[x][y][z] = BLOCKS::AIR;

                if (y == 0) // Bedrock
                    blocks[x][y][z] = BLOCKS::STONE;
            }
        }
    }

    heightmap.resize(chunkSizeX);
    for (int x = 0; x < chunkSizeX; x++)
    {
        heightmap[x] = std::vector<GLubyte>(chunkSizeZ);
        for (int z = 0; z < chunkSizeZ; z++)
        {
            for (int y = chunkSizeY - 1; y > 0; y--)
            {
                if (blocks[x][y][z] != BLOCKS::AIR)
                {
                    heightmap[x][z] = y;
                    break;
                }
            }
        }
    }
}

void Chunk::generateChunkMesh(std::vector<std::vector<Chunk>>& chunks, int chunkX, int chunkZ)
{
    const int chunksAmountX = chunks.size();
    const int chunksAmountZ = chunks[0].size();
    indicesCounter = 0;

	for (int x = 0; x < chunkSizeX; x++)
	{
		for (int y = 0; y < chunkSizeY; y++)
		{
			for (int z = 0; z < chunkSizeZ; z++)
			{
                if (blocks[x][y][z] != BLOCKS::AIR)
                {
                    const int leftX = x - 1;
                    const int rightX = x + 1;
                    const int bottomY = y - 1;
                    const int topY = y + 1;
                    const int behindZ = z + 1;
                    const int frontZ = z - 1;

                    if (bottomY != -1)
                    {
                        if (blocks[x][bottomY][z] == BLOCKS::AIR)
                        {
                            addFace(blocks[x][y][z], ChunkFace::BOTTOM, x, y, z);
                        }
                    }

                    if (topY != chunkSizeY)
                    {
                        if (blocks[x][topY][z] == BLOCKS::AIR)
                            addFace(blocks[x][y][z], ChunkFace::TOP, x, y, z);
                    }
                        
                    
                    if (leftX == -1 && chunkX != 0)
                    {
                        if (chunks[chunkX - 1][chunkZ].generated)
                        {
                            if (chunks[chunkX - 1][chunkZ].blocks[chunkSizeX - 1][y][z] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::LEFT, x, y, z);
                            }
                        }
                    }
                    else if (leftX == -1)
                        addFace(blocks[x][y][z], ChunkFace::LEFT, x, y, z);
                    else if (blocks[leftX][y][z] == BLOCKS::AIR)
                    {
                        addFace(blocks[x][y][z], ChunkFace::LEFT, x, y, z);
                    }
                    
                    if (rightX == chunkSizeX && chunkX != chunksAmountX-1)
                    {
                        if (chunks[chunkX + 1][chunkZ].generated)
                        {
                            if (chunks[chunkX + 1][chunkZ].blocks[0][y][z] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::RIGHT, x, y, z);
                            }
                        }
                    }
                    else if (rightX == chunkSizeX)
                        addFace(blocks[x][y][z], ChunkFace::RIGHT, x, y, z);
                    else if (blocks[rightX][y][z] == BLOCKS::AIR)
                        addFace(blocks[x][y][z], ChunkFace::RIGHT, x, y, z);

                    if (frontZ == -1 && chunkZ != 0)
                    {
                        if (chunks[chunkX][chunkZ - 1].generated)
                        {
                            if (chunks[chunkX][chunkZ - 1].blocks[x][y][chunkSizeZ - 1] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::FRONT, x, y, z);
                            }
                        }
                    }
                    else if (frontZ == -1)
                        addFace(blocks[x][y][z], ChunkFace::FRONT, x, y, z);
                    else if (blocks[x][y][frontZ] == BLOCKS::AIR)
                        addFace(blocks[x][y][z], ChunkFace::FRONT, x, y, z);
                    
                    if (behindZ == chunkSizeZ && chunkZ != chunksAmountZ-1)
                    {
                        if (chunks[chunkX][chunkZ + 1].generated)
                        {
                            if (chunks[chunkX][chunkZ + 1].blocks[x][y][0] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::BACK, x, y, z);
                            }
                        }
                    }
                    else if (behindZ == chunkSizeZ)
                        addFace(blocks[x][y][z], ChunkFace::BACK, x, y, z);
                    else if (blocks[x][y][behindZ] == BLOCKS::AIR)
                        addFace(blocks[x][y][z], ChunkFace::BACK, x, y, z);
                }
			}
		}
	}
    vertices.shrink_to_fit();
    indices.shrink_to_fit();
}

void Chunk::deleteChunkMeshData()
{
    if (VAO == 0 || VBO == 0 || EBO == 0) return;
    glBindVertexArray(NULL);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    vertices.clear();   vertices.shrink_to_fit();
    indices.clear();    indices.shrink_to_fit();
    VAO = 0, VBO = 0, EBO = 0;
}

void Chunk::sendChunkMeshData()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertices[0]) * 6, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertices[0]) * 6, (void*)(sizeof(vertices[0]) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertices[0]) * 6, (void*)(sizeof(vertices[0]) * 5));
    glBindBuffer(GL_ARRAY_BUFFER, NULL);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

    glBindVertexArray(NULL);
}

void Chunk::addFace(BLOCKS blockType, ChunkFace face, int x, int y, int z)
{
    static int verticesAmount = sizeof(frontVertices) / sizeof(frontVertices[0]);
    const GLubyte* selectedFace = nullptr;
    const GLubyte* selectedBlock = nullptr;
    GLubyte ambient = 10;

    if (face == ChunkFace::FRONT)
    {
        selectedFace = frontVertices;
        ambient = 6;
    }
    else if (face == ChunkFace::BACK)
    {
        selectedFace = backVertices;
        ambient = 6;
    }
    else if (face == ChunkFace::TOP)
    {
        selectedFace = topVertices;
    }
    else if (face == ChunkFace::BOTTOM)
    {
        selectedFace = bottomVertices;
        ambient = 4;
    }
    else if (face == ChunkFace::RIGHT)
    {
        selectedFace = rightVertices;
        ambient = 6;
    }
    else if (face == ChunkFace::LEFT)
    {
        selectedFace = leftVertices;
        ambient = 6;
    }

    int lightLevel = heightmap[x][z] - y;
    if (lightLevel > 15)    lightLevel = 15;
    ambient = ambient * pow(0.8, lightLevel);

    if (face == ChunkFace::LEFT || face == ChunkFace::RIGHT 
        || face == ChunkFace::FRONT || face == ChunkFace::BACK)
    {
        if (blockType == BLOCKS::DIRT_TOP)
            selectedBlock = dirtSideTexCoords;
        else if (blockType == BLOCKS::DIRT)
            selectedBlock = dirtBottomTexCoords;
        else if (blockType == BLOCKS::STONE)
            selectedBlock = stoneTexCoords;
    } 
    else if (face == ChunkFace::TOP)
    {
        if (blockType == BLOCKS::DIRT)
            selectedBlock = dirtBottomTexCoords;
        else if (blockType == BLOCKS::DIRT_TOP)
            selectedBlock = dirtTopTexCoords;
        else if (blockType == BLOCKS::STONE)
            selectedBlock = stoneTexCoords;
    }
    else if (face == ChunkFace::BOTTOM)
    {
        if (blockType == BLOCKS::DIRT || blockType == BLOCKS::DIRT_TOP)
            selectedBlock = dirtBottomTexCoords;
        else if (blockType == BLOCKS::STONE)
            selectedBlock = stoneTexCoords;
    }

    int texCoords = 0;
    for (int i = 0; i < verticesAmount; i += 3)
    {
        vertices.push_back(selectedFace[i] + x);
        vertices.push_back(selectedFace[i + 1] + y);
        vertices.push_back(selectedFace[i + 2] + z);
        vertices.push_back(selectedBlock[texCoords]);
        vertices.push_back(selectedBlock[texCoords + 1]);
        vertices.push_back(ambient);
        texCoords += 2;
    }

    
    for (int i = 0; i < 6; i++)
    {
        indices.push_back(indicesCounter + cubeIndices[face][i]);
    }
    indicesCounter += 4;
}

void Chunk::drawChunk()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
    glBindVertexArray(NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

void Chunk::clearChunk()
{
    blocks.clear();     blocks.shrink_to_fit();

    indicesCounter = 0;
    viewable = false;
    generated = false;
    active = false;
}