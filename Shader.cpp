#include "Shader.h"

void Shader::addShader(GLenum type, const char* shaderFileLocation)
{
	std::string shaderData = "";
	std::ifstream shaderFile(shaderFileLocation);
	if (shaderFile.is_open())
	{
		std::string line;
		while (std::getline(shaderFile, line))
		{
			shaderData += line + "\n";
		}
	}
	else
	{
		std::cout << "Unable to open file: " << shaderFileLocation << std::endl;
	}

	const char* convertedShaderData = shaderData.c_str();

	GLuint shaderID = glCreateShader(type);
	glShaderSource(shaderID, 1, &convertedShaderData, NULL);
	glCompileShader(shaderID);

	char infoLog[1024];
	int success;

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
		std::cout << infoLog << std::endl;
	}

	shaderList.push_back(shaderID);
}

void Shader::linkProgram()
{
	shaderProgramID = glCreateProgram();
	for (GLuint shaderID : shaderList)
	{
		glAttachShader(shaderProgramID, shaderID);
	}
	
	glLinkProgram(shaderProgramID);
	char infoLog[1024];
	int success;

	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgramID, 1024, 0, infoLog);
		std::cout << infoLog << std::endl;
	}

	for (GLuint shaderID : shaderList)
	{
		glDeleteShader(shaderID);
	}
}