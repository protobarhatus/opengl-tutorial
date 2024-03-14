#pragma once
#include <glad/glad.h> // GLAD: https://github.com/Dav1dde/glad ... GLAD 2 also works via the web-service: https://gen.glad.sh/ (leaving all checkbox options unchecked)

#include <GLFW/glfw3.h>
#include <string>
#include <vector>

GLuint createSharedBufferObject(void* data, int data_size, int binding);
void printProgramLog(int program);

std::string readFile(const std::string& name);

bool checkCompilation(int shader);
void loadShader(int prog, const std::string& name, GLuint type);
void loadShader(int prog, const std::vector<std::string>& names, GLuint type);