#include "gl_utils.h"
#include <iostream>
#include <fstream>
#include <vector>
GLuint createSharedBufferObject(void* data, int data_size, int binding)
{
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data_size, data, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return ssbo;
}

void printProgramLog(int program)
{
	int logl;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH,
		&logl);
	char* log = new char[logl];
	glGetProgramInfoLog(program,
		logl,
		&logl,
		log);
	std::cout << log;
	delete[] log;
}

std::string readFile(const std::string& name)
{
	std::ifstream file(name, std::ios_base::in);
	std::string str{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	return str;
}


bool checkCompilation(int shader)
{
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success != GL_FALSE)
		return true;
	GLint logSize = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
	std::vector<GLchar> errorLog(logSize);
	glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);
	if (logSize > 0)
		std::cout << &errorLog[0];
	return false;
}

void loadShader(int prog, const std::vector<std::string> & names, GLuint type)
{
	unsigned int vert = glCreateShader(type);
	std::string src = "";
	for (auto& it : names)
		src += readFile(it);
	const char* vert_src = src.c_str();
	glShaderSource(vert, 1, &vert_src, NULL);
	glCompileShader(vert);

	if (!checkCompilation(vert))
	{
		system("pause");
		glDeleteShader(vert);
	}
	glAttachShader(prog, vert);
}

void loadShader(int prog, const std::string& name, GLuint type)
{
	unsigned int vert = glCreateShader(type);
	std::string src = readFile(name);
	const char* vert_src = src.c_str();
	glShaderSource(vert, 1, &vert_src, NULL);
	glCompileShader(vert);

	if (!checkCompilation(vert))
	{
		system("pause");
		glDeleteShader(vert);
	}
	glAttachShader(prog, vert);
}