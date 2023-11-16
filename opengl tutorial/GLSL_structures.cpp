#include "GLSL_structures.h"
#include <stdlib.h>
#include <assert.h>

GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { PRIMITIVE_TYPE_SPHERE, 0, 0, 0, position, 21231., rotation, 0, radius, 0 };
}

GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { PRIMITIVE_TYPE_CYLINDER, 0, 0, 0, position, 213213., rotation,0, height / 2, radius };
}

GlslSceneMemory::GlslSceneMemory(int prim_count, int vec2_count, int vec3_count)
{
	this->primitives_buffer = (GLSL_Primitive*)calloc(prim_count, sizeof(GLSL_Primitive));
	this->vec2_buffer = (float*)calloc(vec2_count, 2);
	this->vec3_buffer = (float*)calloc(vec3_count, 3);

	this->prim_buffer_size = prim_count;
	this->vec2_buffer_size = vec2_count;
	this->vec3_buffer_size = vec3_count;

	this->prim_buffer_count = this->vec2_count = this->vec3_count = 0;
}


std::vector<Vector<2>> getVec2Size(const std::unique_ptr<Object>& obj)
{
	switch(obj->get)
}

int getVec3Size(const std::unique_ptr<Object>& obj)
{

}

void GlslSceneMemory::addObject(const std::unique_ptr<Object>& obj)
{
	if (obj->getId() == ObjectType::COMPOSED_OBJECT)
		assert(false);

	
}


GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj)
{
	switch (obj->getId())
	{
	case ObjectType::SPHERE:
		return buildSphere(((Sphere*)obj.get())->getRadius(), obj->getPosition(), obj->getRotation());
	case ObjectType::CYLINDER:
		return buildCylinder(((Cylinder*)obj.get())->getHalfHeight() * 2, ((Cylinder*)obj.get())->getRadius(), obj->getPosition(), obj->getRotation());
	default:
		assert(false);
	}
}

GLSL_vec3::GLSL_vec3(const Vector<3>& v) : x(v.x()), y(v.y()), z(v.z())
{

}

GLSL_Quat::GLSL_Quat(const Quat& q) : a0(q.a0()), a1(q.a1()), a2(q.a2()), a3(q.a3())
{
}
