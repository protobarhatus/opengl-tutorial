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


void loadTexture(int width, int height, const unsigned char* buffer, bool first_time = false, GLuint texture_slot = GL_TEXTURE0, GLuint internal_format = GL_RGBA8, GLuint format = GL_RGBA)
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
	Vertex() : position(0, 0, 0), texture_coords(0, 0) {}
};



bool checkVisibilityOnGpu(const std::unique_ptr<Object>& obj, int from_id, int on_id, int SCREEN_SIZE, int PIXEL_ACCURACY)
{
	Vector<3> from_pos = obj->getObjectOfId(from_id)->getPosition();
	Vector<3> on_pos = obj->getObjectOfId(on_id)->getPosition();
	Vector<3> from_bb = obj->getObjectOfId(from_id)->getBoundingBox();
	Vector<3> on_bb = obj->getObjectOfId(on_id)->getBoundingBox();

	double x_dist = std::max(std::abs(from_pos.x() - on_pos.x()) - from_bb.x() - on_bb.x(), 0.0);
	double y_dist = std::max(abs(from_pos.y() - on_pos.y()) - from_bb.y() - on_bb.y(), 0.0);
	double z_dist = std::max(abs(from_pos.z() - on_pos.z()) - from_bb.z() - on_bb.z(), 0.0);

	double dist = length(Vector<3>(x_dist, y_dist, z_dist));


	const double DENSITY = dist * 2.0 / SCREEN_SIZE * PIXEL_ACCURACY;
	// (1) GLFW: Initialise & Configure
	// -----------------------------------------
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window = glfwCreateWindow(100, 100, "GLFW Test Window – Changing Colours", NULL, NULL);

	glfwMakeContextCurrent(window); // Set the window to be used and then centre that window on the monitor. 

	 //(2) GLAD: Load OpenGL Function Pointers
	// -------------------------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // For GLAD 2 use the following instead: gladLoadGL(glfwGetProcAddress)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	int ray_trace_programm = glCreateProgram();
	loadShader(ray_trace_programm, std::vector<std::string>{ "..\\opengl tutorial\\raytrace.comp", "..\\opengl tutorial\\visibility_detection_main.comp" }, GL_COMPUTE_SHADER);

	glEnable(GL_DEPTH_TEST);

	glLinkProgram(ray_trace_programm);
	glValidateProgram(ray_trace_programm);




	createTexImage(1, 1);
	glActiveTexture(GL_TEXTURE0);

	int object_from_which_look_location = glGetUniformLocation(ray_trace_programm, "object_from_which_look");
	int object_on_what_look_location = glGetUniformLocation(ray_trace_programm, "object_on_what_look");
	int from_object_position_location = glGetUniformLocation(ray_trace_programm, "from_object_position");
	int to_object_position_location = glGetUniformLocation(ray_trace_programm, "to_object_position");
	int from_object_bounding_box_location = glGetUniformLocation(ray_trace_programm, "from_object_bounding_box");
	int on_object_bounding_box_location = glGetUniformLocation(ray_trace_programm, "on_object_bounding_box");
	int DENSITY_location = glGetUniformLocation(ray_trace_programm, "DENSITY");
	int lin_size1_location = glGetUniformLocation(ray_trace_programm, "LIN_SIZE1");
	int lin_size2_location = glGetUniformLocation(ray_trace_programm, "LIN_SIZE2");


	


	

	GlslSceneMemory shader_scene;
	std::map<int, int> id_map;
	shader_scene.setSceneAsComposedObject(obj->copy(), std::set<int>{from_id, on_id}, id_map);
	//shader_scene.dropToFiles("shadered\\");
	shader_scene.bind(ray_trace_programm, ray_trace_programm);


	glUniform1i(object_from_which_look_location, id_map[from_id]);
	glUniform1i(object_on_what_look_location, id_map[on_id]);


	glUniform3f(from_object_position_location, from_pos.x(), from_pos.y(), from_pos.z());
	glUniform3f(to_object_position_location, on_pos.x(), on_pos.y(), on_pos.z());
	glUniform3f(from_object_bounding_box_location, from_bb.x(), from_bb.y(), from_bb.z());
	glUniform3f(on_object_bounding_box_location, on_bb.x(), on_bb.y(), on_bb.z());
	glUniform1f(DENSITY_location, DENSITY);
	int lin_size1_x = int(2 * from_bb.x() / DENSITY) + 1;
	int lin_size1_y = int(2 * from_bb.y() / DENSITY) + 1;
	int lin_size2_x = int(2 * on_bb.x() / DENSITY) + 1;
	int lin_size2_y = int(2 * on_bb.y() / DENSITY) + 1;
	glUniform3i(lin_size1_location, lin_size1_x, lin_size1_y, int(2 * from_bb.z() / DENSITY) + 1);
	glUniform3i(lin_size2_location, lin_size2_x, lin_size2_y, int(2 * on_bb.z() / DENSITY) + 1);


	glUseProgram(ray_trace_programm);

	glDispatchCompute(ceil(lin_size1_x * lin_size2_x / 8), ceil(lin_size1_x * lin_size2_y / 4), 36);
	//std::cout << glGetError() << '\n';;
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	float res[4] = { 0,0,0,0 };
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, res);
	std::cout << res[0];
	system("pause");

	glfwTerminate();

	exit(EXIT_SUCCESS);
}




bool checkVisibilityCpu(const std::unique_ptr<Object>& obj, int from_id, int on_id, int SCREEN_SIZE, int PIXEL_ACCURACY)
{
	Vector<3> from_pos = obj->getObjectOfId(from_id)->getPosition();
	Vector<3> on_pos = obj->getObjectOfId(on_id)->getPosition();
	Vector<3> from_bb = obj->getObjectOfId(from_id)->getBoundingBox();
	Vector<3> on_bb = obj->getObjectOfId(on_id)->getBoundingBox();

	double x_dist = std::max(std::abs(from_pos.x() - on_pos.x()) - from_bb.x() - on_bb.x(), 0.0);
	double y_dist = std::max(abs(from_pos.y() - on_pos.y()) - from_bb.y() - on_bb.y(), 0.0);
	double z_dist = std::max(abs(from_pos.z() - on_pos.z()) - from_bb.z() - on_bb.z(), 0.0);

	double dist = length(Vector<3>(x_dist, y_dist, z_dist));


	const double DENSITY = dist * 2.0 / SCREEN_SIZE * PIXEL_ACCURACY;

	int lin_size1_x = int(2 * from_bb.x() / DENSITY) + 1;
	int lin_size1_y = int(2 * from_bb.y() / DENSITY) + 1;
	int lin_size2_x = int(2 * on_bb.x() / DENSITY) + 1;
	int lin_size2_y = int(2 * on_bb.y() / DENSITY) + 1;

	Vector<3> dirs[6] = { {1,0,0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1} };
	std::pair<Vector<3>, Vector<3>> oct_dirs[6] = { {{0, 1, 0}, {0,0,1}}, {{1,0,0},{0,0,1}}, {{1,0,0},{0,1,0}}, {{0, 1, 0}, {0,0,1}}, {{1,0,0},{0,0,1}}, {{1,0,0},{0,1,0}} };
	for (int i = 0; i < 6; ++i)
	{
		Vector<3> edg_cen = from_pos + dirs[i] * abs(dot(from_bb, dirs[i])) * 1.01;
		for (int x1 = 0; x1 < lin_size1_x; ++x1)
		{
			for (int y1 = 0; y1 < lin_size1_y; ++y1)
			{
				Vector<3> p1 = edg_cen + oct_dirs[i].first * float(dot(from_bb, oct_dirs[i].first) - DENSITY * x1) + oct_dirs[i].second * float(dot(from_bb, oct_dirs[i].second) - DENSITY * y1);
				for (int j = 0; j < 6; ++j)
				{
					Vector<3> edg_cen2 = on_pos + dirs[j] * dot(on_bb, dirs[j]) * 1.01;
					for (int x2 = 0; x2 < lin_size2_x; ++x2)
					{
						for (int y2 = 0; y2 < lin_size2_y; ++y2)
						{
							Vector<3> p2 = edg_cen2 + oct_dirs[j].first * float(dot(on_bb, oct_dirs[j].first) - DENSITY * x2) + oct_dirs[j].second * float(dot(on_bb, oct_dirs[j].second) - DENSITY * y2);
							auto isr = obj->intersectWithRay(p1, p2 - p1);
							if (isr.first && isr.second.obj_id == on_id)
							{
								return true;
							}
						}
					}
					std::cout << (double(i) / 6 + 1.0 / 6 * double(x1) / lin_size1_x + 1.0 / 6 * 1.0 / lin_size1_x * double(y1) / lin_size1_y + 1.0 / 6 * 1.0 / lin_size1_x * 1.0 / lin_size1_y * double(j) / 6) * 100 << "%\n";
				}
			}
		}
	}
	return false;

}




int main()
{
	int SCREEN_SIZE = 1000;
	int PIXEL_ACCURACY = 1;

	bool ON_GPU = false;

	auto obj = parse(readFile("..\\opengl tutorial\\examples/visibility_scene.txt"));
	assert(obj != nullptr);

	int from_id, on_id;
	std::cout << "Ids: ";
	std::cin >> from_id >> on_id;



	bool res = ON_GPU ? checkVisibilityOnGpu(obj, from_id, on_id, SCREEN_SIZE, PIXEL_ACCURACY) : checkVisibilityCpu(obj, from_id, on_id, SCREEN_SIZE, PIXEL_ACCURACY);
	if (res)
		std::cout << "VISIBLIE\n";
	else
		std::cout << "NOT VISIBLE\n";
	system("pause");
	
	
}














/*//для демки обнаружения видимости
auto cbox = makeBox({ 0.5, 0.5, 0.5 }, { 10.6, 20.4, 2.7 }, null_rotation);
Object* cboxp = cbox.get();
auto tbox = makeSphere(0.5, { 10.5, 9.7, 2.9 });
Object* tboxp = tbox.get();
int tbox_id = tbox->getObjId();
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