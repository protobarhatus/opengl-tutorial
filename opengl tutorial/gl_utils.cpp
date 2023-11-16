#include "gl_utils.h"


GLuint createSharedBufferObject(void* data, int data_size, int binding)
{
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data_size, data, GL_STATIC_READ); //sizeof(data) only works for statically sized C/C++ arrays.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return ssbo;
}