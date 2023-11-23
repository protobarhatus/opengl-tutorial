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
	float x, y, z;
	GLSL_vec3(const Vector<3>& v);
	GLSL_vec3();
};
struct GLSL_vec4
{
	float x, y, z, t;
	GLSL_vec4(const Vector<3>& v);
	GLSL_vec4(const Vector<4>& v);
	GLSL_vec4();
};

struct GLSL_mat3
{
	GLSL_vec4 mat[3];
	GLSL_mat3() {}
	GLSL_mat3(const Matrix<3>& m);
};

struct GLSL_ComposedObject
{
	int operation;
};

class GlslSceneMemory
{
	GLSL_Primitive* primitives_buffer;
	GLSL_vec2* vec2_buffer;
	GLSL_vec4* vec3_buffer;
	int* int_buffer;
	GLSL_mat3* mat3_buffer;
	int prim_buffer_size, vec2_buffer_size, vec3_buffer_size, int_buffer_size, mat3_buffer_size;
	int prim_buffer_count, vec2_count, vec3_count, int_buffer_count, mat3_buffer_count;

	void addObject(const std::unique_ptr<Object>& obj);
	typedef int ComposedObjectRepresentation;
	std::vector<GLSL_ComposedObject> composed_object_nodes_buffer;
	
public:
	GlslSceneMemory(int prim_count, int vec2_count, int vec3_count, int int_buffer_count, int mat3_buffer_count);
	void setSceneAsComposedObject(const std::unique_ptr<Object>& obj);
	
	void bind(int programm, int current_program);
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
	//rdivh
	float sc3;
	GLSL_Quat rotation;
	unsigned int normals_count;
	//половина высоты для призмы и цилиндра, высота для пирамиды, радиус для сферы
	float sc1;
	//радиус для цилиндра
	float sc2;
	unsigned int int_index;
};



GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation);
GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation);
GLSL_Primitive buildCone(float height, float radius, float rdivh, GLSL_vec3 position, GLSL_Quat rotation);

GLSL_Primitive buildPrizm(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float half_height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index,
	unsigned int normal_index);

GLSL_Primitive buildPiramid(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index, unsigned int normal_index);
GLSL_Primitive buildBox(GLSL_vec3 hsize, GLSL_vec3 position, GLSL_Quat rotation);