#include "Texture.h"

Texture::Texture()
{
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
}

void Texture::loadTexture(const char* fileLoc, GLint wrappingMethod, GLint scalingMethodMin, GLint scalingMethodMag,
	GLenum textureInternalFormat, GLenum textureFormat)
{
	stbi_set_flip_vertically_on_load(1);
	stbi_uc* texData = stbi_load(fileLoc, &width, &height, &bitDepth, 0);
	if (!texData)
	{
		std::cout << "Failed to load: " << fileLoc << std::endl;
		return;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scalingMethodMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scalingMethodMag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2);
	
	glTexImage2D(GL_TEXTURE_2D, 0, textureInternalFormat, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(texData);
}

void Texture::useTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::clearTexture()
{
	glDeleteTextures(1, &textureID);
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
}

Texture::~Texture()
{
	glDeleteTextures(1, &textureID);
}