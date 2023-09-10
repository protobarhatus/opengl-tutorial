#include <glad/glad.h> // GLAD: https://github.com/Dav1dde/glad ... GLAD 2 also works via the web-service: https://gen.glad.sh/ (leaving all checkbox options unchecked)

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


std::string readFile(const std::string& name)
{
	std::ifstream file(name, std::ios_base::in);
	std::string str{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	return str;
}

void loadShader(int prog, const std::string& name, GLuint type)
{
	unsigned int vert = glCreateShader(type);
	std::string src = readFile(name);
	const char* vert_src = src.c_str();
	glShaderSource(vert, 1, &vert_src, NULL);
	glCompileShader(vert);
	glAttachShader(prog, vert);
}

struct Vector
{
	float x, y, z;
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
};

Vector operator+(const Vector& a, const Vector& b)
{
	return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
}

float dot(const Vector& a, const Vector& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector cross(const Vector& a, const Vector& b)
{
	return Vector(a.y * b.z - a.z * b.y, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
}

Vector operator*(const Vector& a, float b)
{
	return Vector(a.x * b, a.y * b, a.z * b);
}

Vector operator*(float a, const Vector& b)
{
	return b * a;
}
Vector normalize(const Vector& b)
{
	return b * (1.0/ sqrt(dot(b, b)));
}
class Quat;
Quat operator*(const Quat& a, const Quat& b);
struct Quat
{
	float a0;
	Vector a;
	Quat(float a0, float a1, float a2, float a3) : a0(a0), a(a1, a2, a3) {}
	Quat(float a0, const Vector& a) : a0(a0), a(a) {}
	void rotate(double angle, const Vector& n) {
		*this = *this * Quat(cos(angle / 2), sin(angle / 2) * normalize(n));
	}
};

Quat operator*(const Quat& a, const Quat& b)
{
	return Quat(a.a0 * b.a0 - dot(a.a, b.a), a.a0 * b.a + a.a * b.a0 + cross(a.a, b.a));
}

GLuint loadTGA_glfw(const char* imagepath) {

	// Создаем одну OpenGL текстуру
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Привязываем" только что созданную текстуру и таким образом все последующие операции будут производиться с ней
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Читаем файл и вызываем glTexImage2D с необходимыми параметрами
	//stbi_set_flip_vertically_on_load(1);
	int width, height, bpp;
	unsigned char* texture = stbi_load(imagepath, &width, &height, &bpp, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	stbi_image_free(texture);


	// Трилинейная фильтрация.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	// Возвращаем идентификатор текстуры который мы создали

	
	return textureID;
}



int main()
{
	// (1) GLFW: Initialise & Configure
	// -----------------------------------------
	if (!glfwInit())
		exit(EXIT_FAILURE);

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_DOUBLEBUFFER, 1);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int monitor_width = mode->width; // Monitor's width.
	int monitor_height = mode->height;

	int window_width = 600;
	int window_height = 600;

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "GLFW Test Window – Changing Colours", NULL, NULL);
	// GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Drawing Basic Shapes - Buffer Objects & Shaders", glfwGetPrimaryMonitor(), NULL); // Full Screen Mode ("Alt" + "F4" to Exit!)

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); // Set the window to be used and then centre that window on the monitor. 
	glfwSetWindowPos(window, (monitor_width - window_width) / 2, (monitor_height - window_height) / 2);

	glfwSwapInterval(1); // Set VSync rate 1:1 with monitor's refresh rate.

	 //(2) GLAD: Load OpenGL Function Pointers
	// -------------------------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // For GLAD 2 use the following instead: gladLoadGL(glfwGetProcAddress)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	const int vertex_dim = 3;
	const int color_dim = 2;
	float data[] = {
		-1, -1, 1, 48.0/512, 48.0/256,
		1, -1, 1,  48.0 / 512, 64.0 / 256,
		1, -1, -1, 64.0 / 512, 64.0 / 256,
		-1, -1, -1, 64.0 / 512, 48.0 / 256,

		-1, -1, 1, 48.0 / 512, 48.0 / 256,
		1, -1, 1, 48.0 / 512, 64.0 / 256,
		1, 1, 1,  64.0 / 512, 64.0 / 256,
		-1, 1, 1, 64.0 / 512, 48.0 / 256,

		1, -1, 1, 48.0 / 512, 48.0 / 256,
		1, -1, -1, 48.0 / 512, 64.0 / 256,
		1, 1, -1, 64.0 / 512, 64.0 / 256,
		1, 1, 1, 64.0 / 512, 48.0 / 256,

		-1, -1, -1, 48.0 / 512, 48.0 / 256,
		1, -1, -1, 48.0 / 512, 64.0 / 256,
		1, 1, -1, 64.0 / 512, 64.0 / 256,
		-1, 1, -1, 64.0 / 512, 48.0 / 256,

		-1, -1, 1, 48.0 / 512, 48.0 / 256,
		-1, -1, -1, 48.0 / 512, 64.0 / 256,
		-1, 1, -1, 64.0 / 512, 64.0 / 256,
		-1, 1, 1, 64.0 / 512, 48.0 / 256,

		-1, 1, 1, 48.0 / 512, 48.0 / 256,
		1, 1, 1, 48.0 / 512, 64.0 / 256,
		1, 1, -1, 64.0 / 512, 64.0 / 256,
		-1, 1, -1, 64.0 / 512, 48.0 / 256,
	};

	unsigned int elements[] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 15, 14,
		12, 14, 13,

		16, 19, 18,
		16, 18, 17,

		20, 21, 22,
		20, 22, 23
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, vertex_dim, GL_FLOAT, GL_FALSE, (vertex_dim+color_dim)*sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, color_dim, GL_FLOAT, GL_FALSE, (vertex_dim + color_dim) * sizeof(float),(const void*)(sizeof(float)*vertex_dim));

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_DYNAMIC_DRAW);


	unsigned int prog = glCreateProgram();

	
	loadShader(prog, "shader.vert", GL_VERTEX_SHADER);
	loadShader(prog, "shader.frag", GL_FRAGMENT_SHADER);
	
	glEnable(GL_DEPTH_TEST);
	glLinkProgram(prog);
	glValidateProgram(prog);
	glUseProgram(prog);

	int rotation_location = glGetUniformLocation(prog, "u_cr");
	int position_location = glGetUniformLocation(prog, "u_pos");
	int texture_sampler_location = glGetUniformLocation(prog, "u_texture");
	

	auto texture_id = loadTGA_glfw("terrain-atlas.tga");
	glUniform1i(texture_sampler_location, 0);

	Quat rot(1, 0, 0, 0);
	Vector pos(0, 0, -2);
	int pos_direction = -1;

	glUniform1i(texture_sampler_location, 0);

	while (!glfwWindowShouldClose(window)) // Main-Loop
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen with... red, green, blue.
		
		glUniform4f(rotation_location, rot.a0, rot.a.x, rot.a.y, rot.a.z);
		glUniform3f(position_location, pos.x, pos.y, pos.z);
		glDrawElements(GL_TRIANGLES, sizeof(elements)/sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

		glUniform3f(position_location, pos.x + 2, pos.y, pos.z);
		glDrawElements(GL_TRIANGLES, sizeof(elements) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);
		rot.rotate(3.14 / 600, { 0, 2, 0 });
		pos.z += 0.01 * pos_direction;
		if (-pos.z >= 6.0)
			pos_direction *= -1;
		if (-pos.z <= 1.8)
			pos_direction *= -1;


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &texture_id);
	/* glfwDestroyWindow(window) // Call this function to destroy a specific window */
	glfwTerminate(); // Destroys all remaining windows and cursors, restores modified gamma ramps, and frees resources.

	exit(EXIT_SUCCESS); // Function call: exit() is a C/C++ function that performs various tasks to help clean up resources.
}