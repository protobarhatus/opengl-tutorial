#pragma once
#include "linear_algebra.h"
#include <vector>
#include <vector>
#include <memory>
//#define BB_SPHERE

struct IntersectionResult
{
	double t;
	Vector<3> n;
	bool in;
	int obj_id;
	IntersectionResult(double t, double x, double y, double z, bool in, int index);
	IntersectionResult(double t, const Vector<3>& n, bool in, int index);
	IntersectionResult();
};
typedef IntersectionResult ISR;

enum class ObjectType
{
	BOX,
	PRIZM,
	CONE,
	PIRAMID,
	CYLINDER,
	SPHERE,
	POLYHEDRON,
	COMPOSED_OBJECT
};
class Object;


struct SceneStruct
{
	enum class Type
	{
		NONE,
		OBJECT,
		VECTOR
	};
	Type type;
	std::unique_ptr<Object> obj_scene;
	std::vector<std::unique_ptr<Object>> vec_scene;
};


class Object
{
protected:
	//parent is composed object
	const Object* parent = nullptr;
	Vector<3> position;
	Quat rotation;
	Matrix<4> transformation_mat;
	Matrix<4> rotation_mat;
	Matrix<4> back_rotation_mat;
	Vector<3> bounding_box;
	//пока оно только для шейдера, на цпу не используется. Позиция центра коробки
	Vector<3> bounding_box_position;
	//оно переопределяется в пирамиде и конусе т к там центр смещен по вертикали
	virtual bool lineIntersectsBoundingBox(const Vector<3>& start, const Vector<3>& dir) const;
	Vector<4> color = { 1,1,1,1 };
	//!! При добавлении новых вещей прочекать ::copy() в ComposedObject иначе парсить будет без учета новых аргументов
	int id = -1;
public:
	virtual ObjectType getType() const = 0;
	virtual std::unique_ptr<Object> copy() const = 0;
	void reCountBoundingBox();
	//в этой функции он не вращается, но в конструкторе сразу после этого поворачивается
	virtual void countBoundingBox() = 0;
	//для composed object требуется перевести bounding box в мировую ск т к иначе граница комбинированного бб будет рассчитана неправильно
	void rotateBoundingBox();
	//заданный напрямую извне
	void setBoundingBoxHSize(const Vector<3>& bb);
	void setBoundingBoxPosition(const Vector<3>& bb);
	Vector<3> getBoundingBoxPosition() const;
	bool haveBoundingBox() const;
	//если оставить в protected то ComposedObject не сможет вызывать это для своих членов. так что пока так
	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const = 0;
	Object(const Vector<3>& pos, const Quat& rot);

	std::pair<bool, ISR> intersectWithRay(const Vector<3>& start, const Vector<3>& direction) const;
	//
	virtual std::vector<ISR> intersectWithRayOnBothSides(const Vector<3>& start, const Vector<3>& direction) const;

	virtual bool isPointInside(const Vector<3>& p) const = 0;
	void moveOn(const Vector<3>& movement);
	void rotate(const Quat& rotation);

	Vector<3> getBoundingBox() const;

	Vector<3> getPosition() const;

	Quat getRotation() const;
	//служит только для ComposedObject
	virtual void globalizeCoordinates();

	void setColor(const Vector<3>& col);
	void setAlpha(double a);
	Vector<4> getColor() const;

	int getId() const;
	void setId(int id);

	virtual const Object* getObjectOfId(int id) const;
	void setParent(const Object* parent);
	bool isItIdOfObjectOrItsParent(int id_to_check) const;

	virtual void propagateId(int& idcounter);

	Matrix<4> getTransformationMatrix() const;
};



class Box : public Object
{
	Vector<3> size;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	Box(const Vector<3>& position, const Vector<3>& half_size, const Quat& rotation);
	virtual void countBoundingBox() override;
	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	std::vector<ISR> intersectWithRayOnBothSides(const Vector<3>& start, const Vector<3>& direction) const;
	bool isPointInside(const Vector<3>& p) const override;
	Vector<3> getHsize() const;


};

class Prizm : public Object
{
	bool is_convex = true;
	double half_height;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	std::vector<Vector<2>> base;
	std::vector<Vector<3>> normals;
	virtual void countBoundingBox() override;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	Prizm(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot);

	virtual bool isPointInside(const Vector<3>& p) const override;

	const std::vector<Vector<2>>& getBase() const;
	const std::vector<Vector<3>>& getNormals() const;
	double getHalfHeight() const;
};

class Cone : public Object
{
	double height, rad, rdivh;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	virtual void countBoundingBox() override;
	virtual bool lineIntersectsBoundingBox(const Vector<3>& start, const Vector<3>& dir) const;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	virtual bool isPointInside(const Vector<3>& p) const override;
	Cone(double height, double rad, const Vector<3>& apex_position, const Quat& rot);

	double getHeight() const;
	double getRadius() const;
	double getRdivh() const;
};

class Piramid : public Object
{
	bool is_convex = true;
	double height;
	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	virtual void countBoundingBox() override;
	std::vector<Vector<2>> base;
	std::vector<Vector<3>> normals;
	virtual bool lineIntersectsBoundingBox(const Vector<3>& start, const Vector<3>& dir) const;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	virtual bool isPointInside(const Vector<3>& p) const override;
	Piramid(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot);

	const std::vector<Vector<2>>& getBase() const;
	const std::vector<Vector<3>>& getNormals() const;
	double getHeight() const;
};

class Cylinder : public Object
{
	double half_height, rad;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	virtual void countBoundingBox() override;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	virtual bool isPointInside(const Vector<3>& p) const override;
	Cylinder(const Vector<3>& pos, double height, double rad, const Quat& rotation);

	double getHalfHeight() const;
	double getRadius() const;
};

class Sphere : public Object
{
	double rad;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	virtual void countBoundingBox() override;
public:
	double getRadius() const;
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	virtual bool isPointInside(const Vector<3>& p) const override;
	Sphere(const Vector<3>& pos, double rad);
};


class Polyhedron : public Object
{
	std::vector<Vector<3>> points;
	std::vector<std::vector<int>> edges;
	std::vector<Vector<3>> normals;
	bool convex = true;
	//в их собственных координатах
	std::vector<std::vector<Vector<2>>> polygons;
	std::vector<Matrix<3>> polygs_coords;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	Polyhedron(const Vector<3>& position, const Quat& rotation, const std::vector<Vector<3>>& points, const std::vector<std::vector<int>>& edges);
	virtual void countBoundingBox() override;
	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	bool isPointInside(const Vector<3>& p) const override;

	const std::vector<Vector<3>>& getPoints() const;
	const std::vector<Vector<3>>& getNormals() const;
	const std::vector<std::vector<int>>& getEdges() const;
	const std::vector<std::vector<Vector<2>>> getPolygons() const;
	const std::vector<Matrix<3>>& getCoords() const;
};





inline bool equal(double a, double b)
{
	return abs(a - b) < 1e-6;
}

template<int dim>
bool isNull(const Vector<dim>& v)
{
	for (int i = 0; i < dim; ++i)
		if (!equal(v.nums[i], 0))
			return false;
	return true;
}

std::pair<bool, double> rayIntersectsSegment(const Vector<2>& p, const Vector<2>& dir, const Vector<2>& a, const Vector<2>& b);

std::pair<int, std::pair<ISR, ISR>> rayIntersectsPolygon(const Vector<2>& p, const Vector<2>& n, const std::vector<Vector<2>>& polygon, const std::vector<Vector<3>>& normals);

bool __depr__isPointInsidePolygon(const Vector<2>& p, const std::vector<Vector<2>>& polygon);

double getParamOnShade(const Vector<3>& st, const Vector<3>& dir, const Vector<2>& p);






std::pair<bool, double> intersectLineWithTriangle(const Vector<3>& p, const Vector<3>& dir, const Vector<3>& A, const Vector<3>& B, const Vector<3>& C);


std::pair<int, std::pair<ISR, ISR>> intersectLineWithCircle(const Vector<2>& p, const Vector<2> dir, double rad);

std::unique_ptr<Object> getBoundingBoxAsObject(const std::unique_ptr<Object>& obj);
std::vector < std::vector<Vector<2>>> splitPolygonIntoConvexParts(const std::vector<Vector<2>>& polygon);