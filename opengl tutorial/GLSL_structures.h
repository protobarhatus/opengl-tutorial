#pragma once
#include <memory>
#include "ComposedObject.h"

struct GLSL_Primitive;

struct GLSL_vec2
{
	float x, y;
	GLSL_vec2(const Vector<2>& v);
	GLSL_vec2();
};
struct GLSL_vec3
{
	float x, y, z, trash;
	GLSL_vec3(const Vector<3>& v);
	GLSL_vec3();
};

class GlslSceneMemory
{
	GLSL_Primitive* primitives_buffer;
	GLSL_vec2* vec2_buffer;
	GLSL_vec3* vec3_buffer;

	int prim_buffer_size, vec2_buffer_size, vec3_buffer_size;
	int prim_buffer_count, vec2_count, vec3_count;


public:
	GlslSceneMemory(int prim_count, int vec2_count, int vec3_count);
	void addObject(const std::unique_ptr<Object>& obj);

	void bind(int programm);
};



struct GLSL_Quat
{
	float a0, a1, a2, a3;
	GLSL_Quat(const Quat& q);
};

struct GLSL_Primitive
{
	unsigned int type;
	unsigned int data_index;
	//сколько элементов в data_array она занимает
	unsigned int data_count;
	unsigned int normal_index;
	GLSL_vec3 position;
	float trash_2;
	GLSL_Quat rotation;
	unsigned int normals_count;
	//половина высоты для призмы и цилиндра, высота для пирамиды, радиус для сферы
	float sc1;
	//радиус для цилиндра
	float sc2;
};
enum PrimitiveTypes
{
	PRIMITIVE_TYPE_PRIZM,
	PRIMITIVE_TYPE_PIRAMID,
	PRIMITIVE_TYPE_SPHERE,
	PRIMITIVE_TYPE_CYLINDER
};


GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation);
GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation);