#include "Chunk.h"

void Chunk::generateChunk(FastNoise& noise)
{
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
            int height = 8 + (noise.GetNoise(x + chunkXPos, z + chunkZPos) + 1) * 8;
            for (int y = 0; y < chunkSizeY; y++)
            {
                if (y > height)
                    blocks[x][y][z] = BLOCKS::AIR;
                else if (y == height)
                    blocks[x][y][z] = BLOCKS::DIRT_TOP;
                else if (y < height && y > height - 3)
                    blocks[x][y][z] = BLOCKS::DIRT;
                else
                    blocks[x][y][z] = BLOCKS::STONE;
            }
        }
    }
}

void Chunk::generateChunkMesh(std::vector<std::vector<Chunk>>& chunks, int chunkX, int chunkZ)
{
    const int chunksAmountX = chunks.size();
    const int chunksAmountZ = chunks[0].size();
    vertices.clear();
    indices.clear();
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
                            addFace(blocks[x][y][z], ChunkFace::BOTTOM, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                        }
                    }

                    if (topY != chunkSizeY)
                    {
                        if (blocks[x][topY][z] == BLOCKS::AIR)
                            addFace(blocks[x][y][z], ChunkFace::TOP, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    }
                        
                    
                    if (leftX == -1 && chunkX != 0)
                    {
                        if (chunks[chunkX - 1][chunkZ].generated)
                        {
                            if (chunks[chunkX - 1][chunkZ].blocks[chunkSizeX - 1][y][z] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::LEFT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                            }
                        }
                    }
                    else if (leftX == -1)
                        addFace(blocks[x][y][z], ChunkFace::LEFT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    else if (blocks[leftX][y][z] == BLOCKS::AIR)
                    {
                        addFace(blocks[x][y][z], ChunkFace::LEFT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    }
                    
                    if (rightX == chunkSizeX && chunkX != chunksAmountX-1)
                    {
                        if (chunks[chunkX + 1][chunkZ].generated)
                        {
                            if (chunks[chunkX + 1][chunkZ].blocks[0][y][z] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::RIGHT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                            }
                        }
                    }
                    else if (rightX == chunkSizeX)
                        addFace(blocks[x][y][z], ChunkFace::RIGHT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    else if (blocks[rightX][y][z] == BLOCKS::AIR)
                        addFace(blocks[x][y][z], ChunkFace::RIGHT, x + chunkXPos, y + chunkYPos, z + chunkZPos);

                    if (frontZ == -1 && chunkZ != 0)
                    {
                        if (chunks[chunkX][chunkZ - 1].generated)
                        {
                            if (chunks[chunkX][chunkZ - 1].blocks[x][y][chunkSizeZ - 1] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::FRONT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                            }
                        }
                    }
                    else if (frontZ == -1)
                        addFace(blocks[x][y][z], ChunkFace::FRONT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    else if (blocks[x][y][frontZ] == BLOCKS::AIR)
                        addFace(blocks[x][y][z], ChunkFace::FRONT, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    
                    if (behindZ == chunkSizeZ && chunkZ != chunksAmountZ-1)
                    {
                        if (chunks[chunkX][chunkZ + 1].generated)
                        {
                            if (chunks[chunkX][chunkZ + 1].blocks[x][y][0] == BLOCKS::AIR)
                            {
                                addFace(blocks[x][y][z], ChunkFace::BACK, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                            }
                        }
                    }
                    else if (behindZ == chunkSizeZ)
                        addFace(blocks[x][y][z], ChunkFace::BACK, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                    else if (blocks[x][y][behindZ] == BLOCKS::AIR)
                        addFace(blocks[x][y][z], ChunkFace::BACK, x + chunkXPos, y + chunkYPos, z + chunkZPos);
                }
			}
		}
	}
    vertices.shrink_to_fit();
}

void Chunk::deleteChunkMeshData()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &texVBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Chunk::sendChunkMeshData()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, (void*)(sizeof(vertices[0]) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, (void*)(sizeof(vertices[0]) * 5));
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
    const GLfloat* selectedFace = nullptr;
    const GLfloat* selectedBlock = nullptr;
    float ambient = 1.0f;

    if (face == ChunkFace::FRONT)
    {
        selectedFace = frontVertices;
        ambient = 0.6f;
    }
    else if (face == ChunkFace::BACK)
    {
        selectedFace = backVertices;
        ambient = 0.6f;
    }
    else if (face == ChunkFace::TOP)
    {
        selectedFace = topVertices;
    }
    else if (face == ChunkFace::BOTTOM)
    {
        selectedFace = bottomVertices;
        ambient = 0.5f;
    }
    else if (face == ChunkFace::RIGHT)
    {
        selectedFace = rightVertices;
        ambient = 0.6f;
    }
    else if (face == ChunkFace::LEFT)
    {
        selectedFace = leftVertices;
        ambient = 0.6f;
    }

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
        if (blockType == BLOCKS::DIRT || blockType == BLOCKS::DIRT_TOP)
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

    indices.push_back(indicesCounter);
    indices.push_back(indicesCounter + 2);
    indices.push_back(indicesCounter + 1);
    indices.push_back(indicesCounter);
    indices.push_back(indicesCounter + 3);
    indices.push_back(indicesCounter + 2);
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
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &texVBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    VAO = 0, VBO = 0, texVBO = 0, EBO = 0;
    indicesCounter = 0;
    viewable = false;
    generated = false;
    active = false;

    std::vector<GLfloat>().swap(vertices);
    std::vector<unsigned int>().swap(indices);
    std::vector <std::vector<std::vector<BLOCKS>>>().swap(blocks);
}

Chunk::~Chunk()
{
    //clearChunk();
}
