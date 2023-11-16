#pragma once
#include <glad/glad.h> // GLAD: https://github.com/Dav1dde/glad ... GLAD 2 also works via the web-service: https://gen.glad.sh/ (leaving all checkbox options unchecked)

#include <GLFW/glfw3.h>

GLuint createSharedBufferObject(void* data, int data_size, int binding);
