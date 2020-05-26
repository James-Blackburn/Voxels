#pragma once

#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Shader
{
public:
	Shader() = default;
	~Shader() = default;

	void addShader(GLenum type, const char* shaderFileLoc);
	void linkProgram();
	GLuint getID() { return shaderProgramID; }

private:
	GLuint shaderProgramID;
	std::vector<GLuint> shaderList;
};

