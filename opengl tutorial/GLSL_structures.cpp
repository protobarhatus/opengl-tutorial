#include "GLSL_structures.h"
#include <stdlib.h>
#include <assert.h>
#include "gl_utils.h"
#include <fstream>
void __savePrimitive(const std::unique_ptr<Object>& prim);
GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj, int data_index, int normals_index, int int_index);

GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation, GLSL_vec4 color)
{
	return { (unsigned int)ObjectType::SPHERE, 0, 0, 0, position, 21231., rotation, 0, radius, 0, 0, color};
}

GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation, GLSL_vec4 color)
{
	return { (unsigned int)ObjectType::CYLINDER, 0, 0, 0, position, 1., rotation,0, height / 2, radius , 0, color };
}

GLSL_Primitive buildPrizm(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float half_height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index,
	unsigned int normal_index, GLSL_vec4 color)
{
	return { (unsigned int)ObjectType::PRIZM, data_index, (unsigned int)base.size(), normal_index, position, 1., rotation, (unsigned int)normals.size(), half_height, 0 , 0, color };
}

GLSL_Primitive buildCone(float height, float radius, float rdivh, GLSL_vec3 position, GLSL_Quat rotation, GLSL_vec4 color)
{
	return { (unsigned int)ObjectType::CONE, 0, 0, 0, position, rdivh, rotation, 0, height, radius , 0, color };
}

GLSL_Primitive buildPiramid(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index, unsigned int normal_index, GLSL_vec4 color)
{
	return { (unsigned int)ObjectType::PIRAMID, data_index, (unsigned int)base.size(), normal_index, position, 1., rotation, (unsigned int)normals.size(), height, 0 , 0, color };
}

GLSL_Primitive buildBox(GLSL_vec3 hsize, GLSL_vec3 position, GLSL_Quat rotation, GLSL_vec4 color)
{
	return { (unsigned int)ObjectType::BOX, 0, 0, 0, position, hsize.z, rotation,0, hsize.x, hsize.y, 0, color };
}
GLSL_Primitive buildPolyhedron(const std::unique_ptr<Object>& obj, unsigned int data_index, unsigned int normals_index, unsigned  int int_index, GLSL_vec4 color)
{
	const Polyhedron* poly = (const Polyhedron*)obj.get();
	unsigned int dat_size = 0;
	for (auto& it : poly->getPolygons())
		dat_size += it.size();
	return { (unsigned int)ObjectType::POLYHEDRON, data_index, (unsigned int)poly->getPoints().size(), normals_index, obj->getPosition(), 1., obj->getRotation(), (unsigned int)(poly->getPoints().size() + poly->getNormals().size()), 0, 0, int_index , color};
}


GlslSceneMemory::GlslSceneMemory()
{

}


std::vector<Vector<2>> getVec2Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getType())
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
std::vector<GLSL_BoundingBoxData> getBoundingBoxData(const std::unique_ptr<Object>& obj)
{
	if (obj->haveBoundingBox())
		return { { (GLSL_vec3)(obj->getBoundingBox()), 0.0, (GLSL_vec3)(obj->getBoundingBoxPosition()), 0.0 } };
	else
		return {};
}
void GlslSceneMemory::setComposedObject(const std::unique_ptr<Object>& obj, int buffer_position, std::map<int, int>* map_of_ids, const std::set<int>& important_ids)
{
	if (this->composed_object_nodes_buffer.size() <= buffer_position)
		this->composed_object_nodes_buffer.resize(buffer_position + 1);
	if (important_ids.find(obj->getId()) != important_ids.end() && map_of_ids != nullptr)
		map_of_ids->insert({ obj->getId(), buffer_position });
	if (obj->getType() == ObjectType::COMPOSED_OBJECT)
	{
		const ComposedObject* obj_p = (const ComposedObject*)obj.get();
		assert(equal(obj_p->getPosition(), { 0,0,0 }));
		assert(equal(obj_p->getRotation(), Quat(1, 0, 0, 0)));
		this->composed_object_nodes_buffer[buffer_position] = { (int)mapOperations(obj_p->getOperation()),
		 obj->haveBoundingBox() ? (int)this->bb_buffer.size() : -1 };
		auto bb_dat = getBoundingBoxData(obj);
		for (auto& it : bb_dat)
			this->bb_buffer.push_back(it);

		setComposedObject(obj_p->getLeft(), 2 * (buffer_position + 1) - 1, map_of_ids, important_ids);
		setComposedObject(obj_p->getRight(), 2 * (buffer_position + 1), map_of_ids, important_ids);
	}
	else
	{
		this->composed_object_nodes_buffer[buffer_position] = { (int)this->primitives_buffer.size(), obj->haveBoundingBox() ? (int)this->bb_buffer.size() : -1 };
		this->primitives_to_node_mapping.push_back(buffer_position);
		this->addObject(obj);
	}
}

void GlslSceneMemory::setSceneAsComposedObject(const std::unique_ptr<Object>& obj)
{
	setComposedObject(obj, 0, nullptr, std::set<int>());
}

void GlslSceneMemory::setSceneAsComposedObject(const std::unique_ptr<Object>& obj, const std::set<int>& important_ids, std::map<int, int>& map_of_ids)
{
	setComposedObject(obj, 0, &map_of_ids, important_ids);
}

std::vector<Vector<3>> getVec3Data(const std::unique_ptr<Object>& obj)
{
	switch (obj->getType())
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
	switch (obj->getType())
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
	switch (obj->getType())
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
	if (obj->getType() == ObjectType::COMPOSED_OBJECT)
		assert(false);
	__savePrimitive(obj);

	GLSL_Primitive primitive = buildObject(obj, this->vec2_buffer.size(), this->vec3_buffer.size(), this->int_buffer.size());
	auto vec2_data = getVec2Data(obj);
	auto vec3_data = getVec3Data(obj);
	auto int_data = getIntData(obj, this->mat3_buffer.size());
	auto mat3_data = getMat3Data(obj);
	auto bb_data = getBoundingBoxData(obj);

	this->primitives_buffer.push_back(primitive);
	for (auto& it : vec2_data)
		this->vec2_buffer.push_back(it);
	for (auto& it : vec3_data)
		this->vec3_buffer.push_back(it);
	for (auto& it : int_data)
		this->int_buffer.push_back(it);
	for (auto& it : mat3_data)
		this->mat3_buffer.push_back(it);
	for (auto& it : bb_data)
		this->bb_buffer.push_back(it);
}

static void writeBinaryFile(const char* data, int size, const std::string& name)
{
	std::ofstream out;
	out.open(name, std::ios::out | std::ios::binary);
	out.write(data, size);
	out.close();
}

void GlslSceneMemory::dropToFiles(const std::string& dir) const
{
	if (this->primitives_buffer.size() > 0)
		writeBinaryFile((char*)&this->primitives_buffer[0], sizeof(GLSL_Primitive) * this->primitives_buffer.size(), dir + "primitives.buf");
	if (this->vec2_buffer.size() > 0)
		writeBinaryFile((char*)&this->vec2_buffer[0], sizeof(GLSL_vec2) * this->vec2_buffer.size(), dir + "vec2.buf");
	if (this->vec3_buffer.size() > 0)
		writeBinaryFile((char*)&this->vec3_buffer[0], sizeof(Vec3Type) * this->vec3_buffer.size(), dir + "vec3.buf");
	if (this->int_buffer.size() > 0)
		writeBinaryFile((char*)&this->int_buffer[0], sizeof(int) * this->int_buffer.size(), dir + "int.buf");	
	if (this->mat3_buffer.size() > 0)
		writeBinaryFile((char*)&this->mat3_buffer[0], sizeof(GLSL_mat3) * this->mat3_buffer.size(), dir + "mat3.buf");
	if (this->composed_object_nodes_buffer.size() > 0)
		writeBinaryFile((char*)&this->composed_object_nodes_buffer[0], sizeof(GLSL_ComposedObject) * this->composed_object_nodes_buffer.size(), dir + "composed.buf");
	if (this->bb_buffer.size() > 0)
		writeBinaryFile((char*)&this->bb_buffer[0], sizeof(GLSL_BoundingBoxData) * this->bb_buffer.size(), dir + "bounding_boxes.buf");
}

void GlslSceneMemory::bind(int programm, int current_program)
{
	if (this->primitives_buffer.size() > 0)
		auto prim_buf = createSharedBufferObject(&this->primitives_buffer[0], sizeof(GLSL_Primitive)*this->primitives_buffer.size(), 1);
	if (this->vec2_buffer.size() > 0)
		auto data_buf = createSharedBufferObject(&this->vec2_buffer[0], sizeof(GLSL_vec2)*this->vec2_buffer.size(), 2);
	if (this->vec3_buffer.size() > 0)
		auto normals_buf = createSharedBufferObject(&this->vec3_buffer[0], sizeof(Vec3Type)*this->vec3_buffer.size(), 3);
	if (this->int_buffer.size() > 0)
		auto int_buf = createSharedBufferObject(&this->int_buffer[0], sizeof(int) * this->int_buffer.size(), 4);
	if (this->mat3_buffer.size() > 0)
		auto mat3_buf = createSharedBufferObject(&this->mat3_buffer[0], sizeof(GLSL_mat3) * this->mat3_buffer.size(), 5);
	if (this->composed_object_nodes_buffer.size() > 0)
		auto composed_buf = createSharedBufferObject(&this->composed_object_nodes_buffer[0], sizeof(GLSL_ComposedObject) * this->composed_object_nodes_buffer.size(), 6);
	if (this->bb_buffer.size() > 0)
		auto bb_buf = createSharedBufferObject(&this->bb_buffer[0], sizeof(GLSL_BoundingBoxData) * this->bb_buffer.size(), 7);


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


int GlslSceneMemory::getPrimitivesCount()
{
	return this->primitives_buffer.size();
}

int GlslSceneMemory::getDataCount()
{
	return vec2_buffer.size();
}

int GlslSceneMemory::getNormalsCount()
{
	return this->vec3_buffer.size();
}

int GlslSceneMemory::getComposedObjectNodesCount()
{
	return this->composed_object_nodes_buffer.size();
}



GLSL_Primitive buildObject(const std::unique_ptr<Object>& obj, int data_index, int normals_index, int int_index)
{
	switch (obj->getType())
	{
	case ObjectType::SPHERE:
		return buildSphere(((Sphere*)obj.get())->getRadius(), obj->getPosition(), obj->getRotation(), obj->getColor());
	case ObjectType::CYLINDER:
		return buildCylinder(((Cylinder*)obj.get())->getHalfHeight() * 2, ((Cylinder*)obj.get())->getRadius(), obj->getPosition(), obj->getRotation(), obj->getColor());
	case ObjectType::PRIZM:
		return buildPrizm(((Prizm*)obj.get())->getBase(), ((Prizm*)obj.get())->getNormals(), ((Prizm*)obj.get())->getHalfHeight(),
			obj->getPosition(), obj->getRotation(), data_index, normals_index, obj->getColor());
	case ObjectType::CONE:
		return buildCone(((Cone*)obj.get())->getHeight(), ((Cone*)obj.get())->getRadius(), ((Cone*)obj.get())->getRdivh(), obj->getPosition(), obj->getRotation(), obj->getColor());
	case ObjectType::PIRAMID:
		return buildPiramid(getVec2Data(obj), getVec3Data(obj), ((Piramid*)obj.get())->getHeight(), obj->getPosition(), obj->getRotation(), data_index, normals_index, obj->getColor());
	case ObjectType::BOX:
		return buildBox(((Box*)obj.get())->getHsize(), obj->getPosition(), obj->getRotation(), obj->getColor());
	case ObjectType::POLYHEDRON:
		return buildPolyhedron(obj, data_index, normals_index, int_index, obj->getColor());
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







/*репликация кода с шейдера для дебага*/

//точки которые вылазят за стек будут отброшены.
#define STACK_SIZE 200
struct Intersection
{
	float t;
	Vector<3> n;
	Intersection() {}
	Intersection(float t, Vector<3> n) : t(t), n(n) {}
	Intersection(const ISR& s) : t(s.t), n(s.n) {}
};
struct IntersectionListUnit
{
	Intersection data;
	int next_index;
	bool is_in;
	IntersectionListUnit() {}
	IntersectionListUnit(Intersection data, int ni, bool in) : data(data), next_index(ni), is_in(in) {}
};
IntersectionListUnit intersections_stack[STACK_SIZE];
//поскольку удаление не предусмотренно, мы просто идем вправо. То есть есть ограничение по в целом количеству пересечений на обьекте.
int stack_index = 0;
//листы по инлексу должны совпадать с нодами, которых они представляют. Это по сути просто отображение из нодов в начало листа с их данными
int lists_stack[STACK_SIZE];
std::vector<std::unique_ptr<Object>> primitives;

void __savePrimitive(const std::unique_ptr<Object>& prim)
{
	primitives.resize(primitives.size() + 1);
	primitives.back() = prim->copy();
}
std::vector<GLSL_ComposedObject> composed_objects;
typedef std::vector<ISR> IRO;

#define ComposedObjectNode_isPrimitive(X) (X.operation >= 0)
#define ComposedObjectNode_getPrimitiveIndex(X) (X.operation)

#define ComposedObjectNode_left(X, N) (2*((N)+1) - 1)
#define ComposedObjectNode_right(X, N) (2*((N)+1))
#define ComposedObjectNode_parent(X, N) (((N)+1)/2 - 1)

const int OBJECTS_ADD = -1;
const int OBJECTS_MULT = -2;
const int OBJECTS_SUB = -3;

void intersectWithPrimitiveAsNode(int node, Vector<3> camera_pos, Vector<3> dir)
{
	IRO inter_res = primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[node])]->intersectWithRayOnBothSides(camera_pos, dir);
	switch (inter_res.size())
	{
	case 0:
		lists_stack[node] = -1;
		return;
	case 1:
		//это просто касание, и не стоит это рассматривать. Потому что тогда is_in всеравно кривоватое.
		lists_stack[node] = -1;
		return;
		/*if (inter_res.first.t >= 0)
		{
			lists_stack[node] = stack_index;
			intersections_stack[stack_index++] = IntersectionListUnit(inter_res.first, -1);
			return;
		}
		lists_stack[node] = -1;
		return;*/
	case 2:
		if (inter_res[0].t >= 0) //если .first >= 0 то и .second >= 0 точно
		{
			lists_stack[node] = stack_index;
			intersections_stack[stack_index] = IntersectionListUnit(inter_res[0], stack_index + 1, true);
			stack_index++;
			intersections_stack[stack_index++] = IntersectionListUnit(inter_res[1], -1, false);
			return;
		}
		//.second точно >= 0, иначе бы вернулось ZERO_IRO
		lists_stack[node] = stack_index;
		intersections_stack[stack_index++] = IntersectionListUnit(inter_res[1], -1, false); //вообще конечно вопрос как это повлияет на алгоритмы т-м операций но вроде бы должно быть нормально.
		return;
	}
}

void pushBackToList(int object, int& last_pushed, int to_push)
{
	if (last_pushed == -1)
		lists_stack[object] = to_push;
	else
		intersections_stack[last_pushed].next_index = to_push;
	last_pushed = to_push;
}

//при этом листы, относящиеся к дочерним нодам разрушаются.
void uniteObjects(int current, int left, int right)
{
	int left_it = lists_stack[left];
	int right_it = lists_stack[right];

	bool in_a = false;
	bool in_b = false;

	int last_pushed = -1;

	while (left_it != -1 && right_it != -1)
	{
		if (intersections_stack[left_it].data.t < intersections_stack[right_it].data.t)
		{
			//if (!in_b)
				pushBackToList(current, last_pushed, left_it);
			in_a = intersections_stack[left_it].is_in;
			left_it = intersections_stack[left_it].next_index;
		}
		else
		{
			//if (!in_a)
				pushBackToList(current, last_pushed, right_it);
			in_b = intersections_stack[right_it].is_in;
			right_it = intersections_stack[right_it].next_index;
		}
	}
	//сейчас одно из них дошло точно до конца поэтому никаких ифов можно не делать
	while (right_it != -1)
	{
		pushBackToList(current, last_pushed, right_it);
		right_it = intersections_stack[right_it].next_index;
	}
	while (left_it != -1)
	{
		pushBackToList(current, last_pushed, left_it);
		left_it = intersections_stack[left_it].next_index;
	}
	//это по сути закрытие листа, иначе финальный нод будет ссылаться на рандомный другой
	pushBackToList(current, last_pushed, -1);
}

void intersectObjects(int current, int left, int right)
{
	bool in_a = false;
	bool in_b = false;

	int last_pushed = -1;
	int left_it = lists_stack[left];
	int right_it = lists_stack[right];

	while (left_it != -1 && right_it != -1)
	{
		if (intersections_stack[left_it].data.t < intersections_stack[right_it].data.t)
		{
			if (in_b)
				pushBackToList(current, last_pushed, left_it);
			in_a = intersections_stack[left_it].is_in;
			left_it = intersections_stack[left_it].next_index;
		}
		else
		{
			if (in_a)
				pushBackToList(current, last_pushed, right_it);
			in_b = intersections_stack[right_it].is_in;
			right_it = intersections_stack[right_it].next_index;
		}
	}
	pushBackToList(current, last_pushed, -1);
}

void subtractObjects(int current, int left, int right)
{
	bool in_a = false;
	bool in_b = false;

	int left_it = lists_stack[left];
	int right_it = lists_stack[right];

	int last_pushed = -1;
	while (left_it != -1 && right_it != -1)
	{
		if (intersections_stack[left_it].data.t < intersections_stack[right_it].data.t - 1e-3 * (1 - 2 * int(in_a)))
		{
			if (!in_b)
				pushBackToList(current, last_pushed, left_it);
			in_a = intersections_stack[left_it].is_in;
			left_it = intersections_stack[left_it].next_index;
		}
		else
		{
			if (in_a)
			{
				pushBackToList(current, last_pushed, right_it);
				//при этом last_pushed == right_it
				intersections_stack[last_pushed].is_in = !intersections_stack[right_it].is_in;
				intersections_stack[last_pushed].data.n = -1 * intersections_stack[right_it].data.n;
				in_b = !intersections_stack[last_pushed].is_in;
			}
			else
				in_b = intersections_stack[right_it].is_in;
			right_it = intersections_stack[right_it].next_index;
		}
	}
	while (left_it != -1)
	{
		pushBackToList(current, last_pushed, left_it);
		left_it = intersections_stack[left_it].next_index;
	}
	pushBackToList(current, last_pushed, -1);
}

void combineObjects(int current, int left, int right)
{
	switch (composed_objects[current].operation)
	{
	case OBJECTS_ADD:
		uniteObjects(current, left, right);
		return;
	case OBJECTS_MULT:
		intersectObjects(current, left, right);
		return;
	case OBJECTS_SUB:
		subtractObjects(current, left, right);
		return;
	}
}
std::pair<bool, ISR> GlslSceneMemory::__intersectWithRay(const Vector<3>& start, const Vector<3>& dir) const
{
	composed_objects = this->composed_object_nodes_buffer;
	stack_index = 0;

	int current = 0;
	int prev = -1;
	if (ComposedObjectNode_isPrimitive(composed_objects[0]))
	{
		IRO inter_res = primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[0])]->intersectWithRayOnBothSides(start, dir);
		bool has_intersection = (inter_res.size() > 0);
		return { has_intersection, inter_res[0] };
	}
	//has_intersection = true;
	while (current != -1)
	{
		if (prev == ComposedObjectNode_parent(composed_objects[current], current))
		{
			int left = ComposedObjectNode_left(composed_objects[current], current);
			if (ComposedObjectNode_isPrimitive(composed_objects[left]))
			{

				intersectWithPrimitiveAsNode(left, start, dir);
				//return MAGENTA_INTER;
				prev = left;
			}
			else
			{
				prev = current;
				current = left;
				continue;
			}
		}
		else if (prev == ComposedObjectNode_left(composed_objects[current], current))
		{
			int right = ComposedObjectNode_right(composed_objects[current], current);
			if (ComposedObjectNode_isPrimitive(composed_objects[right]))
			{
				intersectWithPrimitiveAsNode(right, start, dir);
				prev = right;
			}
			else
			{
				prev = current;
				current = right;
				continue;
			}
		}
		else
		{
			int left = ComposedObjectNode_left(composed_objects[current], current);
			int right = prev;
			combineObjects(current, left, right);
			prev = current;
			current = ComposedObjectNode_parent(composed_objects[current], current);
		}
	}

	if (lists_stack[0] == -1)
	{
		bool has_intersection = false;
		Intersection res = { 0,{0,0,0} };
		return { has_intersection, ISR(0,0,0,0,0, -1) };
	}
	
	return { true, ISR(intersections_stack[lists_stack[0]].data.t,  intersections_stack[lists_stack[0]].data.n, true, -1)};
}
#include <iostream>
static inline int at(int i, int j, int width)
{
	return (j * width + i) * 4;
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
void GLSL__castRays(int window_width, int window_height, std::vector<unsigned char>& canvas, const Vector<3>& camera_pos, GlslSceneMemory& memory)
{
	double horizontal_step = 2.0 / window_width;
	double vertical_step = 2.0 / window_height;
	for (int i = 0; i < window_width; ++i)
	{
		for (int j = 0; j < window_height; ++j)
		{

			//setColor(i, j, window_width, { 255, (unsigned char)(255 * double(i) / window_width), (unsigned char)(255 * double(j) / window_height), 255 }, canvas);
			//continue;

			if (i == 430 - 1 && j == window_height - (530 - 30))
				std::cout << "A";
			if (i == 167 && j == 100)
				std::cout << "A";
			Vector<3> ray_dir(-1 + i * horizontal_step, 1, -1 + j * vertical_step);
			auto cast = memory.__intersectWithRay(camera_pos, ray_dir);
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