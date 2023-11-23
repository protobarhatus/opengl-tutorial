#include "GLSL_structures.h"
#include <stdlib.h>
#include <assert.h>
#include "gl_utils.h"

GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj, int data_index, int normals_index, int int_index);

GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { (unsigned int)ObjectType::SPHERE, 0, 0, 0, position, 21231., rotation, 0, radius, 0 };
}

GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { (unsigned int)ObjectType::CYLINDER, 0, 0, 0, position, 1., rotation,0, height / 2, radius };
}

GLSL_Primitive buildPrizm(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float half_height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index,
	unsigned int normal_index)
{
	return { (unsigned int)ObjectType::PRIZM, data_index, (unsigned int)base.size(), normal_index, position, 1., rotation, (unsigned int)normals.size(), half_height, 0 };
}

GLSL_Primitive buildCone(float height, float radius, float rdivh, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { (unsigned int)ObjectType::CONE, 0, 0, 0, position, rdivh, rotation, 0, height, radius };
}

GLSL_Primitive buildPiramid(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index, unsigned int normal_index)
{
	return { (unsigned int)ObjectType::PIRAMID, data_index, (unsigned int)base.size(), normal_index, position, 1., rotation, (unsigned int)normals.size(), height, 0 };
}

GLSL_Primitive buildBox(GLSL_vec3 hsize, GLSL_vec3 position, GLSL_Quat rotation)
{
	return { (unsigned int)ObjectType::BOX, 0, 0, 0, position, hsize.z, rotation,0, hsize.x, hsize.y };
}
GLSL_Primitive buildPolyhedron(const std::unique_ptr<Object>& obj, unsigned int data_index, unsigned int normals_index, unsigned  int int_index)
{
	const Polyhedron* poly = (const Polyhedron*)obj.get();
	unsigned int dat_size = 0;
	for (auto& it : poly->getPolygons())
		dat_size += it.size();
	return { (unsigned int)ObjectType::POLYHEDRON, data_index, (unsigned int)poly->getPoints().size(), normals_index, obj->getPosition(), 1., obj->getRotation(), (unsigned int)(poly->getPoints().size() + poly->getNormals().size()), 0, 0, int_index };
}


GlslSceneMemory::GlslSceneMemory()
{

}


std::vector<Vector<2>> getVec2Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getId())
	{
	case ObjectType::PRIZM:
		return ((Prizm*)obj.get())->getBase();
	case ObjectType::PIRAMID:
		return ((Piramid*)obj.get())->getBase();
	case ObjectType::POLYHEDRON:
	{
		std::vector<Vector<2>> data;
		for (auto& it : ((Polyhedron*)obj.get())->getPolygons())
			for (auto& it1 : it)
				data.push_back(it1);
		return data;
	}
	default:
		return {};
	}
}

OperationTypeInShader mapOperations(ComposedObject::Operation op)
{
	switch (op)
	{
	case ComposedObject::Operation::PLUS:
		return OperationTypeInShader::OBJECTS_ADD;
	case ComposedObject::Operation::MINUS:
		return OperationTypeInShader::OBJECTS_SUB;
	case ComposedObject::Operation::MULT:
		return OperationTypeInShader::OBJECTS_MULT;
	}
}

void GlslSceneMemory::setComposedObject(const std::unique_ptr<Object>& obj, int buffer_position)
{
	if (this->composed_object_nodes_buffer.size() <= buffer_position)
		this->composed_object_nodes_buffer.resize(buffer_position + 1);
	if (obj->getId() == ObjectType::COMPOSED_OBJECT)
	{
		const ComposedObject* obj_p = (const ComposedObject*)obj.get();
		assert(equal(obj_p->getPosition(), { 0,0,0 }));
		assert(equal(obj_p->getRotation(), Quat(1, 0, 0, 0)));
		this->composed_object_nodes_buffer[buffer_position] = { (int)mapOperations(obj_p->getOperation()) };
		setComposedObject(obj_p->getLeft(), 2 * (buffer_position + 1) - 1);
		setComposedObject(obj_p->getRight(), 2 * (buffer_position + 1));
	}
	else
	{
		this->composed_object_nodes_buffer[buffer_position] = { (int)this->primitives_buffer.size() };
		this->addObject(obj);
	}
}

void GlslSceneMemory::setSceneAsComposedObject(const std::unique_ptr<Object>& obj)
{
	setComposedObject(obj, 0);
}

std::vector<Vector<3>> getVec3Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getId())
	{
	case ObjectType::PRIZM:
		return ((Prizm*)obj.get())->getNormals();
	case ObjectType::PIRAMID:
		return ((Piramid*)obj.get())->getNormals();
	case ObjectType::POLYHEDRON:
	{
		auto res = ((Polyhedron*)obj.get())->getPoints();
		for (auto& it : ((Polyhedron*)obj.get())->getNormals())
			res.push_back(it);
		return res;
	}
	default:
		return {};
	}
}

std::vector<int> getIntData(const std::unique_ptr<Object>& obj, int matrix_buffer_count)
{
	switch (obj->getId())
	{
	case ObjectType::POLYHEDRON:
	{
		Polyhedron* poly = (Polyhedron*)obj.get();
		std::vector<int> res;
		res.push_back(matrix_buffer_count);
		for (auto& it : poly->getEdges())
		{
			res.push_back(it.size());
			for (auto& it1 : it)
				res.push_back(it1);
		}
		return res;
	}
	default:
		return {};
		
	}
}

std::vector<GLSL_mat3> getMat3Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getId())
	{
	case ObjectType::POLYHEDRON:
	{
		auto dat = ((Polyhedron*)obj.get())->getCoords();
		std::vector<GLSL_mat3> res;
		for (auto& it : dat)
			res.push_back(it);
		return res;}
	default:
		return {};
	
	}
}

void GlslSceneMemory::addObject(const std::unique_ptr<Object>& obj)
{
	if (obj->getId() == ObjectType::COMPOSED_OBJECT)
		assert(false);

	GLSL_Primitive primitive = buildObject(obj, this->vec2_buffer.size(), this->vec3_buffer.size(), this->int_buffer.size());
	auto vec2_data = getVec2Data(obj);
	auto vec3_data = getVec3Data(obj);
	auto int_data = getIntData(obj, this->mat3_buffer.size());
	auto mat3_data = getMat3Data(obj);


	this->primitives_buffer.push_back(primitive);
	for (auto& it : vec2_data)
		this->vec2_buffer.push_back(it);
	for (auto& it : vec3_data)
		this->vec3_buffer.push_back(it);
	for (auto& it : int_data)
		this->int_buffer.push_back(it);
	for (auto& it : mat3_data)
		this->mat3_buffer.push_back(it);
}

void GlslSceneMemory::bind(int programm, int current_program)
{
	if (this->primitives_buffer.size() > 0)
		auto prim_buf = createSharedBufferObject(&this->primitives_buffer[0], sizeof(GLSL_Primitive)*this->primitives_buffer.size(), 1);
	if (this->vec2_buffer.size() > 0)
		auto data_buf = createSharedBufferObject(&this->vec2_buffer[0], sizeof(GLSL_vec2)*this->vec2_buffer.size(), 2);
	if (this->vec3_buffer.size() > 0)
		auto normals_buf = createSharedBufferObject(&this->vec3_buffer[0], sizeof(GLSL_vec4)*this->vec3_buffer.size(), 3);
	if (this->int_buffer.size() > 0)
		auto int_buf = createSharedBufferObject(&this->int_buffer[0], sizeof(int) * this->int_buffer.size(), 4);
	if (this->mat3_buffer.size() > 0)
		auto mat3_buf = createSharedBufferObject(&this->mat3_buffer[0], sizeof(GLSL_mat3) * this->mat3_buffer.size(), 5);
	if (this->composed_object_nodes_buffer.size() > 0)
		auto composed_buf = createSharedBufferObject(&this->composed_object_nodes_buffer[0], sizeof(GLSL_ComposedObject) * this->composed_object_nodes_buffer.size(), 6);


	int primitive_count_location = glGetUniformLocation(programm, "primitives_count");
	int data_count_location = glGetUniformLocation(programm, "data_count");
	int normals_count_location = glGetUniformLocation(programm, "normals_count");
	int composed_object_nodes_count_location = glGetUniformLocation(programm, "composed_object_nodes_count");

	glUseProgram(programm);
	glUniform1i(primitive_count_location, this->primitives_buffer.size());
	glUniform1i(data_count_location, vec2_buffer.size());
	glUniform1i(normals_count_location, this->vec3_buffer.size());
	glUniform1i(composed_object_nodes_count_location, this->composed_object_nodes_buffer.size());
	glUseProgram(current_program);
	
}


GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj, int data_index, int normals_index, int int_index)
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
	case ObjectType::CONE:
		return buildCone(((Cone*)obj.get())->getHeight(), ((Cone*)obj.get())->getRadius(), ((Cone*)obj.get())->getRdivh(), obj->getPosition(), obj->getRotation());
	case ObjectType::PIRAMID:
		return buildPiramid(getVec2Data(obj), getVec3Data(obj), ((Piramid*)obj.get())->getHeight(), obj->getPosition(), obj->getRotation(), data_index, normals_index);
	case ObjectType::BOX:
		return buildBox(((Box*)obj.get())->getHsize(), obj->getPosition(), obj->getRotation());
	case ObjectType::POLYHEDRON:
		return buildPolyhedron(obj, data_index, normals_index, int_index);
	default://composed object тоже сюда не должен попадать
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

GLSL_vec4::GLSL_vec4(const Vector<3>& v) : x(v.x()), y(v.y()), z(v.z()), t(1)
{
}

GLSL_vec4::GLSL_vec4(const Vector<4>& v) : x(v.x()), y(v.y()), z(v.z()), t(v.t())
{
}

GLSL_vec4::GLSL_vec4()
{
}

GLSL_mat3::GLSL_mat3(const Matrix<3>& m)
{
	auto t = transpose(m);
	for (int i = 0; i < 3; ++i)
		this->mat[i] = t.mat[i];
}