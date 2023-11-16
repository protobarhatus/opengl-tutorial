#include "GLSL_structures.h"
#include <stdlib.h>
#include <assert.h>
#include "gl_utils.h"

GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj, int data_index, int normals_index);

GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { PRIMITIVE_TYPE_SPHERE, 0, 0, 0, position, 21231., rotation, 0, radius, 0 };
}

GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { PRIMITIVE_TYPE_CYLINDER, 0, 0, 0, position, 1., rotation,0, height / 2, radius };
}

GLSL_Primitive buildPrizm(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float half_height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index,
	unsigned int normal_index)
{
	return { PRIMITIVE_TYPE_PRIZM, data_index, (unsigned int)base.size(), normal_index, position, 1., rotation, (unsigned int)normals.size(), half_height, 0 };
}

GlslSceneMemory::GlslSceneMemory(int prim_count, int vec2_count, int vec3_count)
{
	this->primitives_buffer = (GLSL_Primitive*)calloc(prim_count, sizeof(GLSL_Primitive));
	this->vec2_buffer = new GLSL_vec2[ vec2_count];
	this->vec3_buffer = new GLSL_vec3[ vec3_count];

	this->prim_buffer_size = prim_count;
	this->vec2_buffer_size = vec2_count;
	this->vec3_buffer_size = vec3_count;

	this->prim_buffer_count = this->vec2_count = this->vec3_count = 0;
}


std::vector<Vector<2>> getVec2Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getId())
	{
	case ObjectType::PRIZM:
		return ((Prizm*)obj.get())->getBase();
	default:
		return {};
	}
}

std::vector<Vector<3>> getVec3Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getId())
	{
	case ObjectType::PRIZM:
		return ((Prizm*)obj.get())->getNormals();
	default:
		return {};
	}
}

void GlslSceneMemory::addObject(const std::unique_ptr<Object>& obj)
{
	if (obj->getId() == ObjectType::COMPOSED_OBJECT)
		assert(false);

	GLSL_Primitive primitive = buildObject(obj, this->vec2_count, this->vec3_count);
	auto vec2_data = getVec2Data(obj);
	auto vec3_data = getVec3Data(obj);

	if (this->prim_buffer_count + 1 >= prim_buffer_size || vec2_count + vec2_data.size() >= vec2_buffer_size ||
		vec3_count + vec3_data.size() >= vec2_buffer_size)
		throw "Out of memory";

	this->primitives_buffer[this->prim_buffer_count++] = primitive;
	for (auto& it : vec2_data)
		this->vec2_buffer[this->vec2_count++] = it;
	for (auto& it : vec3_data)
		this->vec3_buffer[this->vec3_count++] = it;
	
}

void GlslSceneMemory::bind(int programm)
{
	auto prim_buf = createSharedBufferObject(this->primitives_buffer, sizeof(GLSL_Primitive)*this->prim_buffer_count, 1);
	auto data_buf = createSharedBufferObject(this->vec2_buffer, sizeof(GLSL_vec2)*this->vec2_count, 2);
	auto normals_buf = createSharedBufferObject(this->vec3_buffer, sizeof(GLSL_vec3)*this->vec3_count, 3);


	int primitive_count_location = glGetUniformLocation(programm, "primitives_count");
	int data_count_location = glGetUniformLocation(programm, "data_count");
	int normals_count_location = glGetUniformLocation(programm, "normals_count");

	glUniform1i(primitive_count_location, this->prim_buffer_count);
	glUniform1i(data_count_location, this->vec2_count);
	glUniform1i(normals_count_location, this->vec3_count);
	
}


GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj, int data_index, int normals_index)
{
	switch (obj->getId())
	{
	case ObjectType::SPHERE:
		return buildSphere(((Sphere*)obj.get())->getRadius(), obj->getPosition(), obj->getRotation());
	case ObjectType::CYLINDER:
		return buildCylinder(((Cylinder*)obj.get())->getHalfHeight() * 2, ((Cylinder*)obj.get())->getRadius(), obj->getPosition(), obj->getRotation());
	case ObjectType::PRIZM:
		return buildPrizm(((Prizm*)obj.get())->getBase(), ((Prizm*)obj.get())->getNormals(), ((Prizm*)obj.get())->getHalfHeight(),
			obj->getPosition(), obj->getRotation(), data_index, normals_index);
	default:
		assert(false);
	}
}

GLSL_vec3::GLSL_vec3(const Vector<3>& v) : x(v.x()), y(v.y()), z(v.z())
{

}

GLSL_vec3::GLSL_vec3()
{
}

GLSL_Quat::GLSL_Quat(const Quat& q) : a0(q.a0()), a1(q.a1()), a2(q.a2()), a3(q.a3())
{
}

GLSL_vec2::GLSL_vec2(const Vector<2>& v) : x(v.x()), y(v.y())
{

}

GLSL_vec2::GLSL_vec2()
{
}
