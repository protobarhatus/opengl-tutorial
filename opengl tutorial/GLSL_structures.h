#pragma once
#include <memory>
#include "ComposedObject.h"

struct GLSL_Primitive;
class GlslSceneMemory
{
	GLSL_Primitive* primitives_buffer;
	float* vec2_buffer;
	float* vec3_buffer;

	int prim_buffer_size, vec2_buffer_size, vec3_buffer_size;
	int prim_buffer_count, vec2_count, vec3_count;
public:
	GlslSceneMemory(int prim_count, int vec2_count, int vec3_count);
	void addObject(const std::unique_ptr<Object>& obj);
};

struct GLSL_vec3
{
	float x, y, z;
	GLSL_vec3(const Vector<3>& v);
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
	//������� ��������� � data_array ��� ��������
	unsigned int data_count;
	unsigned int normal_index;
	GLSL_vec3 position;
	float trash_2;
	GLSL_Quat rotation;
	unsigned int normals_count;
	//�������� ������ ��� ������ � ��������, ������ ��� ��������, ������ ��� �����
	float sc1;
	//������ ��� ��������
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