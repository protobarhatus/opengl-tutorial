#pragma once

struct GLSL_vec3
{
	float x, y, z;
};

struct GLSL_Quat
{
	float a0, a1, a2, a3;
};

struct GLSL_Primitive
{
	unsigned int type;
	unsigned int data_index;
	//������� ��������� � data_array ��� ��������
	unsigned int data_count;
	unsigned int trash_for_padding;
	GLSL_vec3 position;
	unsigned int trash_2;
	GLSL_Quat rotation;
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