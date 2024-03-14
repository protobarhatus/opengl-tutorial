#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "linear_algebra.h"
#include "primitives.h"
#include "Scene.h"
#include "GLSL_structures.h"
#include "ComposedObject.h"
#include "objects_fabric.h"
#include "parser.h"
#include "gl_utils.h"
#include "GLSL_structures.h"


void loadTexture(int width, int height, const unsigned char* buffer, bool first_time=false, GLuint texture_slot = GL_TEXTURE0, GLuint internal_format = GL_RGBA8, GLuint format = GL_RGBA)
{
	if (first_time)
	{
		glActiveTexture(texture_slot);
		// Создаем одну OpenGL текстуру
		GLuint textureID;
		glGenTextures(1, &textureID);

		// "Привязываем" только что созданную текстуру и таким образом все последующие операции будут производиться с ней
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	// Трилинейная фильтрация.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(texture_slot);
	}
	else
	{
		glActiveTexture(texture_slot);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	
	//glBindTexture(GL_TEXTURE_2D, 0);

	// Возвращаем идентификатор текстуры который мы создали
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

GLuint createTexImage(int width, int height, GLuint texture_slot = GL_TEXTURE0, GLuint internal_format = GL_RGBA32F, GLuint format = GL_RGBA, GLuint type = GL_UNSIGNED_BYTE) {

	// Создаем одну OpenGL текстуру
	glActiveTexture(texture_slot);
	GLuint textureID;
	glGenTextures(1, &textureID);

	
	// "Привязываем" только что созданную текстуру и таким образом все последующие операции будут производиться с ней
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Читаем файл и вызываем glTexImage2D с необходимыми параметрами
	//stbi_set_flip_vertically_on_load(1);

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, nullptr);


	// Трилинейная фильтрация.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//glBindTexture(GL_TEXTURE_2D, 0);

	// Возвращаем идентификатор текстуры который мы создали

	glBindImageTexture(0 + (texture_slot - GL_TEXTURE0), textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	return textureID;
	
}

struct Vertex
{
	Vector<3> position;
	Vector<2> texture_coords;
	Vertex(const Vector<3>& pos, const Vector<2>& text) : position(pos), texture_coords(text) {}
	Vertex(float x, float y, float z, float s, float p) : position(x, y, z), texture_coords(s, p) {}
	Vertex() : position(0,0,0), texture_coords(0,0){}
};



void setCube(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& cube_pos)
{
	Vertex data[] = {
		{-1.f, -1.f, 1.f, 48.0f / 512, 48.0f / 256},
		{1, -1, 1,  48.0 / 512, 64.0 / 256},
		{1, -1, -1, 64.0 / 512, 64.0 / 256},
		{-1, -1, -1, 64.0 / 512, 48.0 / 256},

		{-1, -1, 1, 48.0 / 512, 48.0 / 256},
		{1, -1, 1, 48.0 / 512, 64.0 / 256},
		{1, 1, 1,  64.0 / 512, 64.0 / 256},
		{-1, 1, 1, 64.0 / 512, 48.0 / 256},

		{1, -1, 1, 48.0 / 512, 48.0 / 256},
		{1, -1, -1, 48.0 / 512, 64.0 / 256},
		{1, 1, -1, 64.0 / 512, 64.0 / 256},
		{1, 1, 1, 64.0 / 512, 48.0 / 256},

		{-1, -1, -1, 48.0 / 512, 48.0 / 256},
		{1, -1, -1, 48.0 / 512, 64.0 / 256},
		{1, 1, -1, 64.0 / 512, 64.0 / 256},
		{-1, 1, -1, 64.0 / 512, 48.0 / 256},

		{-1, -1, 1, 48.0 / 512, 48.0 / 256},
		{-1, -1, -1, 48.0 / 512, 64.0 / 256},
		{-1, 1, -1, 64.0 / 512, 64.0 / 256},
		{-1, 1, 1, 64.0 / 512, 48.0 / 256},

		{-1, 1, 1, 48.0 / 512, 48.0 / 256},
		{1, 1, 1, 48.0 / 512, 64.0 / 256},
		{1, 1, -1, 64.0 / 512, 64.0 / 256},
		{-1, 1, -1, 64.0 / 512, 48.0 / 256},
	};
	for (auto& it : data)
		it.position = it.position + cube_pos;
	memcpy(buff + size, data, sizeof(data));
	

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
	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, elements, sizeof(elements));

	size += sizeof(data) / sizeof(Vertex);
	ind_size += sizeof(elements) / sizeof(unsigned int);
}

//ужасный костыль чисто нужен на время сейчас т к лень менять сеткуб
void setGreenCube(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& cube_pos)
{
	Vertex data[] = {
		{-1.f, -1.f, 1.f, 40.0f / 512, 4.0 / 256},
		{1, -1, 1,  40.0 / 512, 4.0 / 256},
		{1, -1, -1, 40.0 / 512, 4.0 / 256},
		{-1, -1, -1, 40.0 / 512, 4.0 / 256},

		{-1, -1, 1, 40.0 / 512, 4.0 / 256},
		{1, -1, 1, 40.0 / 512, 4.0 / 256},
		{1, 1, 1,  40.0 / 512, 4.0 / 256},
		{-1, 1, 1, 40.0 / 512, 4.0 / 256},

		{1, -1, 1, 40.0 / 512, 4.0 / 256},
		{1, -1, -1, 40.0 / 512, 4.0 / 256},
		{1, 1, -1, 40.0 / 512, 4.0 / 256},
		{1, 1, 1, 40.0 / 512, 4.0 / 256},

		{-1, -1, -1, 40.0 / 512, 4.0 / 256},
		{1, -1, -1, 40.0 / 512, 4.0 / 256},
		{1, 1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, -1, 40.0 / 512, 4.0 / 256},

		{-1, -1, 1, 40.0 / 512, 4.0 / 256},
		{-1, -1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, 1, 40.0 / 512, 4.0 / 256},

		{-1, 1, 1, 40.0 / 512, 4.0 / 256},
		{1, 1, 1, 40.0 / 512, 4.0 / 256},
		{1, 1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, -1, 40.0 / 512, 4.0 / 256},
	};
	for (auto& it : data)
		it.position = it.position * 0.05 + cube_pos;
	memcpy(buff + size, data, sizeof(data));


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
	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, elements, sizeof(elements));

	size += sizeof(data) / sizeof(Vertex);
	ind_size += sizeof(elements) / sizeof(unsigned int);
}


void setConvexPrizm(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& center_pos, const std::vector<Vector<2>>& base, float height)
{
	std::vector<Vertex> data(base.size() * 4);
	for (int i = 0; i < base.size(); ++i)
	{
		data[i] = Vertex(Vector<3>(base[i].x(), -height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(48.0 / 512, 64.0 / 256) : Vector<2>(64.0 / 512, 64.0 / 256));
		data[i + base.size()] = Vertex(Vector<3>(base[i].x(), height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(48.0 / 512, 48.0 / 256) : Vector<2>(64.0 / 512, 48.0 / 256));
	}
	for (int i = 0; i < base.size(); ++i)
	{
		data[base.size() * 2 + i] = Vertex(Vector<3>(base[i].x(), -height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(52.0 / 512, 59.0 / 256) : Vector<2>(52.0 / 512, 59.0 / 256));
		data[base.size() * 2 + i + base.size()] = Vertex(Vector<3>(base[i].x(), height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(52.0 / 512, 59.0 / 256) : Vector<2>(52.0 / 512, 59.0 / 256));
	}
	for (auto& it : data)
		it.position = it.position + center_pos;
	memcpy(buff + size, &data[0], data.size() * sizeof(Vertex));

	std::vector<unsigned int> elements(base.size() * 2 * 3 + 2*(base.size() - 2) * 3);
	for (int i = 0; i < base.size(); ++i)
	{
		elements[i*6] = i;
		elements[i * 6 + 1] = (i + 1) % base.size();
		elements[i * 6 + 2] = i + base.size();
		elements[i * 6 + 3] = i + base.size();
		elements[i * 6 + 4] = base.size() + ((i + 1) % base.size());
		elements[i * 6 + 5] = (i + 1) % base.size();
	}
	for (int i = 0; i < base.size() - 2; ++i)
	{
		elements[base.size() * 6 + i * 6] = base.size() * 2;
		elements[base.size() * 6 + i * 6 + 1] = base.size() * 2 + i + 1;
		elements[base.size() * 6 + i * 6 + 2] = base.size() * 2 + i + 2;
		elements[base.size() * 6 + i * 6 + 3] = base.size() * 2 + base.size();
		elements[base.size() * 6 + i * 6 + 4] = base.size() * 2 + base.size() + i + 1;
		elements[base.size() * 6 + i * 6 + 5] = base.size() * 2 + base.size() + i + 2;
	}
	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, &elements[0], elements.size() * sizeof(unsigned int));

	size += data.size();
	ind_size += elements.size();
}

void setConvexPiramid(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& center_pos, const std::vector<Vector<2>>& base, float height)
{
	std::vector<Vertex> data(base.size() * 2 + 1);
	for (int i = 0; i < base.size(); ++i)
	{
		data[i] = Vertex(Vector<3>(base[i].x(), -height, base[i].y()), i % 2 == 0 ? Vector<2>(48.0 / 512, 64.0 / 256) : Vector<2>(64.0 / 512, 64.0 / 256));
	}
	for (int i = 0; i < base.size(); ++i)
	{
		data[base.size() + i] = Vertex(Vector<3>(base[i].x(), -height, base[i].y()), i % 2 == 0 ? Vector<2>(52.0 / 512, 59.0 / 256) : Vector<2>(52.0 / 512, 59.0 / 256));
	}
	data[base.size() * 2] = Vertex({ 0,0,0 }, { 48.0 / 512, 48.0 / 256 });
	for (auto& it : data)
		it.position = it.position + center_pos;
	memcpy(buff + size, &data[0], data.size() * sizeof(Vertex));

	std::vector<unsigned int> elements(base.size() * 3 + (base.size() - 2) * 3);
	for (int i = 0; i < base.size(); ++i)
	{
		elements[i * 3] = i;
		elements[i * 3 + 1] = (i + 1) % base.size();
		elements[i * 3 + 2] = base.size() * 2;
	}

	for (int i = 0; i < base.size() - 2; ++i)
	{
		elements[base.size() * 3 + i * 3] = base.size();
		elements[base.size() * 3 + i * 3 + 1] = base.size() + i + 1;
		elements[base.size() * 3 + i * 3 + 2] = base.size() + i + 2;
	}

	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, &elements[0], elements.size() * sizeof(unsigned int));

	size += data.size();
	ind_size += elements.size();
}

static inline int at(int i, int j, int width)
{
	return (j * width + i)*4;
}
struct Color
{
	unsigned char r, g, b, a;
};
static void setColor(int i, int j, int width, Color col, std::vector<unsigned char>& canvas)
{
	int ind = at(i, j, width);
	memcpy(&canvas[ind], &col, 4);
}

static double sq(double x)
{
	return x * x;
}

void castRays(int window_width, int window_height, std::vector<unsigned char>& canvas, const Vector<3>& camera_pos, const Scene& scene)
{
	double horizontal_step = 2.0 / window_width;
	double vertical_step = 2.0 / window_height;
	for (int i = 0; i < window_width;++i)
	{
		for (int j = 0; j < window_height; ++j)
		{

			//setColor(i, j, window_width, { 255, (unsigned char)(255 * double(i) / window_width), (unsigned char)(255 * double(j) / window_height), 255 }, canvas);
			//continue;

			if (i == 300 && j == 300)
				std::cout << "A";
			Vector<3> ray_dir(-1 + i * horizontal_step, 1, -1 + j * vertical_step);
			auto cast = scene.intersection(camera_pos, ray_dir);
			unsigned char r = 254 * sqrt((sq(dot(ray_dir, cast.second.n)) / dot(ray_dir, ray_dir)));
			if (cast.first)
				setColor(i, j, window_width, { r, r,r,255 }, canvas);
			else
				setColor(i, j, window_width, { 0,0,0,255 }, canvas);
		}
	}
}
void castRays(int window_width, int window_height, std::vector<unsigned char>& canvas, const Vector<3>& camera_pos, const Object* object)
{
	double horizontal_step = 2.0 / window_width;
	double vertical_step = 2.0 / window_height;
	for (int i = 0; i < window_width; ++i)
	{
		for (int j = 0; j < window_height; ++j)
		{

			//setColor(i, j, window_width, { 255, (unsigned char)(255 * double(i) / window_width), (unsigned char)(255 * double(j) / window_height), 255 }, canvas);
			//continue;

			if (i == 180 - 1 && j == window_height-(240 - 30))
				std::cout << "A";
			if (i == 300 && j == 300)
				std::cout << "A";
			Vector<3> ray_dir(-1 + i * horizontal_step, 1, -1 + j * vertical_step);
			auto cast = object->intersectWithRay(camera_pos, ray_dir);
			//setColor(i, j, window_width, { (unsigned char)(254 * std::max(0., std::min(1., cast.second.n.x()))), (unsigned char)(254 * std::max(0., std::min(1., cast.second.n.y()))), (unsigned char)(254 * std::max(0., std::min(1., cast.second.n.z()))), 1 }, canvas);
			//continue;
			unsigned char r = 254 * sqrt((sq(dot(ray_dir, cast.second.n)) / dot(ray_dir, ray_dir)));
			if (cast.first)
				setColor(i, j, window_width, { r, r,r,255 }, canvas);
			else
				setColor(i, j, window_width, { 0,0,0,255 }, canvas);
		}
	}
}



int main()
{
	
	// (1) GLFW: Initialise & Configure
	// -----------------------------------------
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_DOUBLEBUFFER, 1);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int monitor_width = mode->width; // Monitor's width.
	int monitor_height = mode->height;

	int window_width = 1000;
	int window_height = 1000;

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

	
	
	Vertex data[10];
	unsigned int elements[] = { 0, 1, 2, 0, 2, 3};
	//unsigned int elements[1000];
	int data_size = 0;
	int elements_size = 0;

	data[0] = Vertex(Vector<3>(-1, 0, -1), Vector<2>(0, 0));
	data[1] = Vertex(Vector<3>(-1, 0, 1), Vector<2>(0, 1));
	data[2] = Vertex(Vector<3>(1, 0, 1), Vector<2>(1, 1));
	data[3] = Vertex(Vector<3>(1, 0, -1), Vector<2>(1, 0));
	data_size = 4;
	elements_size = 6;


	glBufferData(GL_ARRAY_BUFFER, data_size*sizeof(Vertex), data, GL_STATIC_DRAW);


	int vertex_dim = sizeof(Vector<3>) / sizeof(double);
	int color_dim = sizeof(Vector<2>) / sizeof(double);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, vertex_dim, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, color_dim, GL_DOUBLE, GL_FALSE, (vertex_dim + color_dim) * sizeof(double),(const void*)offsetof(Vertex, texture_coords));

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_DYNAMIC_DRAW);


	unsigned int prog = glCreateProgram();

	
	loadShader(prog, "shader.vert", GL_VERTEX_SHADER);
	loadShader(prog, "shader.frag", GL_FRAGMENT_SHADER);

	int ray_trace_programm = glCreateProgram();
	loadShader(ray_trace_programm, std::vector<std::string>{ "raytrace.comp", "raytrace_main.comp" }, GL_COMPUTE_SHADER);

	glEnable(GL_DEPTH_TEST);
	glLinkProgram(prog);
	glValidateProgram(prog);
	glUseProgram(prog);

	glLinkProgram(ray_trace_programm);
	printProgramLog(ray_trace_programm);
	glValidateProgram(ray_trace_programm);
	printProgramLog(ray_trace_programm);


	int rotation_location = glGetUniformLocation(prog, "u_cr");
	int position_location = glGetUniformLocation(prog, "u_pos");
	int texture_sampler_location = glGetUniformLocation(prog, "u_texture");
	

	auto texture_id = loadTGA_glfw("terrain-atlas copy.tga");
	glUniform1i(texture_sampler_location, 0);

	Quat rot(1, 0, 0, 0);
	Vector<3> camera_pos(0, -1, 0);
	
	int pos_direction = -1;

	auto cstart = glfwGetTime();
	int counter = 0;

	//std::vector<unsigned char> texture(window_width * window_height * 4, 255);
	
	createTexImage(window_width, window_height);
	createTexImage(window_width, window_height, GL_TEXTURE1, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	glActiveTexture(GL_TEXTURE0);

	int camera_pos_location = glGetUniformLocation(ray_trace_programm, "camera_pos");
	int screen_size_location = glGetUniformLocation(ray_trace_programm, "screen_size");
	int normals_count_location = glGetUniformLocation(ray_trace_programm, "normals_count");
	
	std::vector<Vector<2>> square = { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} };
	Quat null_rotation = Quat(1, 0, 0, 0);
	

	auto obj = parse(readFile("examples/visibility_scene.txt"));
	

	assert(obj != nullptr);
	obj->moveOn({ 0,9,0 });
	/**/obj->globalizeCoordinates();




	GlslSceneMemory shader_scene;
	shader_scene.setSceneAsComposedObject(obj->copy());
	//shader_scene.dropToFiles("shadered\\");
	shader_scene.bind(ray_trace_programm, prog);

	camera_pos.nums[1] = 2;

	

	while (!glfwWindowShouldClose(window)) // Main-Loop
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen with... red, green, blue.

		/*clock_t t1 = clock();
		GLSL__castRays(window_width, window_height, texture, camera_pos, shader_scene);
		//castRays(window_width, window_height, texture, camera_pos, obj.get());
		std::cout << clock() - t1 << '\n';
		loadTexture(window_width, window_height, &texture[0], false);*/
		/**/ { // launch compute shaders!
			glUseProgram(ray_trace_programm);
			glUniform3f(camera_pos_location, camera_pos.x(), camera_pos.y(), camera_pos.z());
			glUniform2i(screen_size_location, window_width, window_height);
			
			glDispatchCompute((GLuint)window_width/8, (GLuint)window_width/4, 1);
			//std::cout << glGetError() << '\n';;
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
		
		
		glUseProgram(prog);

		glUniform4f(rotation_location, rot.a0(), rot.a1(), rot.a2(), rot.a3());
		//glUniform3f(position_location, camera_pos.x(), camera_pos.y(), camera_pos.z());
		glUniform3f(position_location, 0, 0, 1);
		//glDrawElements(GL_TRIANGLES, elements_size - 2, GL_UNSIGNED_INT, 0);

		//glDrawElements(GL_LINES, 3, GL_UNSIGNED_INT, (const void*)((elements_size - 2)*sizeof(unsigned int)));
		glDrawElements(GL_TRIANGLES, elements_size , GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();

		double cam_sp = 0.2;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera_pos.nums[0] += cam_sp;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera_pos.nums[0] -= cam_sp;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera_pos.nums[2] += cam_sp;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera_pos.nums[2] -= cam_sp;
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			camera_pos.nums[1] += cam_sp;
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
			camera_pos.nums[1] -= cam_sp;



		++counter;
		//while (glfwGetTime() - cstart < counter * (1.0 / 60.0))
		//{
		//}
	}

	glDeleteTextures(1, &texture_id);
	/* glfwDestroyWindow(window) // Call this function to destroy a specific window */
	glfwTerminate(); // Destroys all remaining windows and cursors, restores modified gamma ramps, and frees resources.

	exit(EXIT_SUCCESS); // Function call: exit() is a C/C++ function that performs various tasks to help clean up resources.
}














/*//для демки обнаружения видимости
auto cbox = makeBox({ 0.5, 0.5, 0.5 }, { 10.6, 20.4, 2.7 }, null_rotation);
Object* cboxp = cbox.get();
auto tbox = makeSphere(0.5, { 10.5, 9.7, 2.9 });
Object* tboxp = tbox.get();
int tbox_id = tbox->getId();
auto object = parse(readFile("examples/visibility_scene.txt"));
object = objectsUnion(objectsUnion(std::move(cbox), std::move(tbox), { 0,0,0 }, null_rotation), std::move(object), { 0,0,0 }, null_rotation);
object->moveOn({ 0, 5, 0 });
object->globalizeCoordinates();
cboxp->setColor({ 1, 0, 0 });


const double DENSITY = length(cboxp->getPosition() - tboxp->getPosition()) * 2.0 / window_width * 1;

Vector<3> step_obj1 = { 0.5 * 2. , 0.5 * 2., 0.5 * 2. };
Vector<3> step_obj2 = { 0.5 * 2., 0.5  * 2., 0.5 * 2. };
auto checkVisibility = [&object, tbox_id, cboxp, tboxp, DENSITY, step_obj1, step_obj2](const Vector<3>& c1, const Vector<3>& c2)->bool {
	std::vector<Vector<3>> dirs = { {1,0,0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1} };
	std::vector<std::pair<Vector<3>, Vector<3>>> oct_dirs = { {{0, 1, 0}, {0,0,1}}, {{1,0,0},{0,0,1}}, {{1,0,0},{0,1,0}}, {{0, 1, 0}, {0,0,1}}, {{1,0,0},{0,0,1}}, {{1,0,0},{0,1,0}} };


	for (int i = 0; i < dirs.size(); ++i)
	{
		const int LIN_SIZE1 = 1.0 / DENSITY;
		double step_x = dot(step_obj1, oct_dirs[i].first) / (LIN_SIZE1 - 1);
		double step_y = dot(step_obj1, oct_dirs[i].second) / (LIN_SIZE1 - 1);
		Vector<3> edg_cen = c1 + dirs[i] * dot(step_obj1, dirs[i]) * 1.01 * 0.5;
		for (int x1 = 0; x1 < LIN_SIZE1; ++x1)
		{
			for (int y1 = 0; y1 < LIN_SIZE1; ++y1)
			{
				Vector<3> p1 = edg_cen + oct_dirs[i].first * (dot(step_obj1, oct_dirs[i].first) * 0.5 - step_x * x1) + oct_dirs[i].second * (dot(step_obj1, oct_dirs[i].second) * 0.5 - step_y * y1);

				for (int j = 0; j < dirs.size(); ++j)
				{
					const int LIN_SIZE2 = 1.0 / DENSITY;
					double step_x2 = dot(step_obj2, oct_dirs[j].first) / (LIN_SIZE2 - 1);
					double step_y2 = dot(step_obj2, oct_dirs[j].second) / (LIN_SIZE2 - 1);
					Vector<3> edg_cen2 = c2 + dirs[j] * dot(step_obj2, dirs[j]) * 1.01 * 0.5;
					for (int x2 = 0; x2 < LIN_SIZE2; ++x2)
					{
						for (int y2 = 0; y2 < LIN_SIZE2; ++y2)
						{
							Vector<3> p2 = edg_cen2 + oct_dirs[j].first * (dot(step_obj2, oct_dirs[j].first) * 0.5 - step_x2 * x2) + oct_dirs[j].second * (dot(step_obj2, oct_dirs[j].second) * 0.5 - step_y2 * y2);

							auto isr = object->intersectWithRay(p1, p2 - p1);
							if (isr.first && length((p1 + (p2 - p1) * isr.second.t) - tboxp->getPosition()) < 0.51)
							{
								printf("{%f, %f, %f}; {%f, %f, %f}", p1.x(), p1.y(), p1.z(), (p1 + (p2 - p1) * isr.second.t).x(), (p1 + (p2 - p1) * isr.second.t).y(), (p1 + (p2 - p1) * isr.second.t).z());
								return true;
							}

						}
					}
					std::cout << (double(i) / dirs.size() + 1.0 / dirs.size() * double(x1) / LIN_SIZE1 + 1.0 / dirs.size() * 1.0 / LIN_SIZE1 * double(y1) / LIN_SIZE1 + 1.0 / dirs.size() * 1.0 / LIN_SIZE1 * 1.0 / LIN_SIZE1 * double(j) / dirs.size()) * 100 << "%\n";
				}
			}
		}
	}
	return false;
};

//if (checkVisibility(cboxp->getPosition(), tboxp->getPosition()))
//	std::cout << "VISIBLE\n";
//else
//	std::cout << "NOT VISIBLE\n";
//system("pause");

bool visibility = false;*/