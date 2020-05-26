#pragma once

#include <iostream>

#include "GL/glew.h"
#include "stb_image.h"

class Texture
{
public:
	Texture();

	void loadTexture(const char* fileLoc, GLint wrappingMethod, GLint scalingMethodMin, GLint scalingMethodMag,
		GLenum textureInternalFormat, GLenum textureFormat);
	void useTexture();
	void clearTexture();

	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;
};

