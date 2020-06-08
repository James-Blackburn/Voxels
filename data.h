#pragma once

#include <GL/glew.h>

const GLubyte frontVertices[] = {
    0, 0, 0,
    0, 1, 0,
    1, 1, 0,
    1, 0, 0,
};

const GLubyte backVertices[] = {
    0, 0, 1,
    0, 1, 1,
    1, 1, 1,
    1, 0, 1,
};

const GLubyte topVertices[] = {
    0, 1, 1,
    0, 1, 0,
    1, 1, 0,
    1, 1, 1,
};

const GLubyte bottomVertices[] = {
    0, 0, 0,
    0, 0, 1,
    1, 0, 1,
    1, 0, 0,
};

const GLubyte rightVertices[] = {
    1, 0, 0,
    1, 1, 0,
    1, 1, 1,
    1, 0, 1,
};

const GLubyte leftVertices[] = {
    0, 0, 0,
    0, 1, 0,
    0, 1, 1,
    0, 0, 1,
};

const GLubyte cubeIndices[6][6] =
{
    {0, 1, 2,   0, 2, 3}, // Front
    {0, 2, 1,   0, 3, 2}, // Back
    {0, 2, 1,   0, 3, 2}, // Top
    {0, 2, 1,   0, 3, 2}, // Bottom
    {0, 1, 2,   0, 2, 3}, // Right
    {0, 2, 1,   0, 3, 2}, // Left
};

const GLubyte dirtSideTexCoords[] = {
    0, 1,
    0, 2,
    1, 2,
    1, 1,
};

const GLubyte dirtTopTexCoords[] = {
    1, 1,
    1, 2,
    2, 2,
    2, 1,
};

const GLubyte dirtBottomTexCoords[] = {
    0, 0,
    0, 1,
    1, 1,
    1, 0,
};

const GLubyte stoneTexCoords[] = {
    1, 0,
    1, 1,
    2, 1,
    2, 0,
};