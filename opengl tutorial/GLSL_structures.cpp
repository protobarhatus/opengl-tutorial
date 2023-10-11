#include "GLSL_structures.h"


GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { PRIMITIVE_TYPE_SPHERE, 0, 0, 123456, position, 21231, rotation, radius, 0 };
}

GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { PRIMITIVE_TYPE_CYLINDER, 0, 0, 123456, position, 213213, rotation, height / 2, radius };
}