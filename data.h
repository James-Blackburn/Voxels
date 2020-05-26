#pragma once

#include <GL/glew.h>

const GLfloat frontVertices[] = {
    -0.5f, -0.5f, -0.5f,
    -0.5f, +0.5f, -0.5f,
    +0.5f, +0.5f, -0.5f,
    +0.5f, -0.5f, -0.5f,
};

const GLfloat backVertices[] = {
    -0.5f, -0.5f, +0.5f,
    -0.5f, +0.5f, +0.5f,
    +0.5f, +0.5f, +0.5f,
    +0.5f, -0.5f, +0.5f,
};

const GLfloat topVertices[] = {
    -0.5f, +0.5f, -0.5f,
    -0.5f, +0.5f, +0.5f,
    +0.5f, +0.5f, +0.5f,
    +0.5f, +0.5f, -0.5f,
};

const GLfloat bottomVertices[] = {
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, +0.5f,
    +0.5f, -0.5f, +0.5f,
    +0.5f, -0.5f, -0.5f,
};

const GLfloat rightVertices[] = {
    +0.5f, -0.5f, -0.5f,
    +0.5f, +0.5f, -0.5f,
    +0.5f, +0.5f, +0.5f,
    +0.5f, -0.5f, +0.5f,
};

const GLfloat leftVertices[] = {
    -0.5f, -0.5f, -0.5f,
    -0.5f, +0.5f, -0.5f,
    -0.5f, +0.5f, +0.5f,
    -0.5f, -0.5f, +0.5f,
};


const GLfloat dirtSideTexCoords[] = {
    0.0f, 0.5f,
    0.0f, 1.0f,
    0.5f, 1.0f,
    0.5f, 0.5f,
};

const GLfloat dirtTopTexCoords[] = {
    0.5f, 0.5f,
    0.5f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.5f,
};

const GLfloat dirtBottomTexCoords[] = {
    0.0f, 0.0f,
    0.0f, 0.5f,
    0.5f, 0.5f,
    0.5f, 0.0f,
};

const GLfloat stoneTexCoords[] = {
    0.5f, 0.0f,
    0.5f, 0.5f,
    1.0f, 0.5f,
    1.0f, 0.0f,
};