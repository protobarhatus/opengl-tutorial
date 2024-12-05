#pragma once
#include <memory>
#include <string>
#include "ComposedObject.h"
#include <map>
#include <set>

struct GLSL_Primitive;

struct GLSL_vec2
{
	float x, y;
	GLSL_vec2(const Vector<2>& v);
	GLSL_vec2();
	GLSL_vec2(const GLSL_vec2&) = default;
	GLSL_vec2& operator=(const GLSL_vec2&) = default;
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

struct GLSL_mat4
{
	GLSL_vec4 mat[4];
	GLSL_mat4() {}
	GLSL_mat4(const Matrix<4>& m);
};

struct GLSL_ComposedObject
{
	int operation;
	int bounding_box_index = -1;
};

enum class OperationTypeInShader
{
	OBJECTS_ADD = -1,
	OBJECTS_MULT = -2,
	OBJECTS_SUB = -3,
	OBJECTS_HIERARCHY = -4
};

struct GLSL_BoundingBoxData
{
	GLSL_vec3 bb_hsize;
	float dump1;
	GLSL_vec3 bb_position;
	float dump2;
};
class GlslSceneMemory
{
	friend class VulkanApp;
	typedef GLSL_vec4 Vec3Type;
	std::vector<GLSL_Primitive> primitives_buffer;
	std::vector<GLSL_vec2> vec2_buffer;
	std::vector<Vec3Type> vec3_buffer;
	std::vector<int> int_buffer;
	std::vector<GLSL_mat3> mat3_buffer;
	std::vector<GLSL_BoundingBoxData> bb_buffer;
	std::vector<int> blas_mapping_buffer;
	std::vector<GLSL_ComposedObject> hierarchy_buffer;
	
	void addObject(const std::unique_ptr<Object>& obj);
	typedef int ComposedObjectRepresentation;
	std::vector<GLSL_ComposedObject> composed_object_nodes_buffer;
	void setComposedObject(const std::unique_ptr<Object>& obj, int root_position, int csg_node_position, std::map<int, int>* map_of_ids, const std::set<int>& important_ids);
	void setHierarchyObject(const std::unique_ptr<Object>& obj, int node_position);


	friend std::string makeTraceFunction(const GlslSceneMemory& scene, int cur_hier_pos);
public:
	GlslSceneMemory();
	void setSceneAsComposedObject(const std::unique_ptr<Object>& obj);
	//void setSceneAsComposedObject(const std::unique_ptr<Object>& obj, const std::set<int>& important_ids, std::map<int, int>& map_of_ids);
	void setScene(const std::vector<std::unique_ptr<Object>>& objs);
	void bind(int programm, int current_program);
	void dropToFiles(const std::string& dir) const;
	//репликация пересечения с сервера для дебага
	std::pair<bool, ISR> __intersectWithRay(const Vector<3>& start, const Vector<3>& dir) const;

	int getPrimitivesCount();
	int getDataCount();
	int getNormalsCount();
	int getComposedObjectNodesCount();
};



struct GLSL_Quat
{
	float a0, a1, a2, a3;
	GLSL_Quat(const Quat& q);
	GLSL_Quat() {}
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

	GLSL_vec4 color;

	GLSL_mat4 transformation;
	GLSL_mat3 rotation_mat;
	GLSL_mat3 back_rotation;
	
};



GLSL_Primitive buildSphere(float radius, GLSL_vec3 position, GLSL_Quat rotation);
GLSL_Primitive buildCylinder(float height, float radius, GLSL_vec3 position, GLSL_Quat rotation);
GLSL_Primitive buildCone(float height, float radius, float rdivh, GLSL_vec3 position, GLSL_Quat rotation);

GLSL_Primitive buildPrizm(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float half_height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index,
	unsigned int normal_index);

GLSL_Primitive buildPiramid(const std::vector<Vector<2>>& base, const std::vector<Vector<3>>& normals, float height, GLSL_vec3 position, GLSL_Quat rotation, unsigned int data_index, unsigned int normal_index);
GLSL_Primitive buildBox(GLSL_vec3 hsize, GLSL_vec3 position, GLSL_Quat rotation);




void GLSL__castRays(int window_width, int window_height, std::vector<unsigned char>& canvas, const Vector<3>& camera_pos, GlslSceneMemory& memory);


std::string makeTraceFunction(const GlslSceneMemory& scene);