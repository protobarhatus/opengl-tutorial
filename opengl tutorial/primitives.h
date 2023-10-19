#pragma once
#include "linear_algebra.h"
#include <vector>
#include <vector>
struct IntersectionResult
{
	double t;
	Vector<3> n;
	bool in;
	IntersectionResult(double t, double x, double y, double z, bool in);
	IntersectionResult(double t, const Vector<3>& n, bool in);
	IntersectionResult();
};
typedef IntersectionResult ISR;

class Object
{
protected:
	Vector<3> position;
	Quat rotation;
	Matrix<4> transformation_mat;
	Matrix<4> rotation_mat;
	Matrix<4> back_rotation_mat;
public:
	//если оставить в protected то ComposedObject не сможет вызывать это для своих членов. так что пока так
	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const = 0;
	Object(const Vector<3>& pos, const Quat& rot);

	std::pair<bool, ISR> intersectWithRay(const Vector<3>& start, const Vector<3>& direction) const;
	std::vector<ISR> intersectWithRayOnBothSides(const Vector<3>& start, const Vector<3>& direction) const;

	virtual bool isPointInside(const Vector<3>& p) const = 0;
	void moveOn(const Vector<3>& movement);
	void rotate(const Quat& rotation);
};

class Prizm : public Object
{
	bool is_convex = true;
	double half_height;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	std::vector<Vector<2>> base;
	std::vector<Vector<3>> normals;
public:
	Prizm(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot);

	virtual bool isPointInside(const Vector<3>& p) const override;
};

class Cone : public Object
{
	double height, rad, rdivh;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
public:
	virtual bool isPointInside(const Vector<3>& p) const override;
	Cone(double height, double rad, const Vector<3>& apex_position, const Quat& rot);
};

class Piramid : public Object
{
	bool is_convex = true;
	double height;
	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
	std::vector<Vector<2>> base;
	std::vector<Vector<3>> normals;
public:
	virtual bool isPointInside(const Vector<3>& p) const override;
	Piramid(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot);
};

class Cylinder : public Object
{
	double half_height, rad;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
public:
	virtual bool isPointInside(const Vector<3>& p) const override;
	Cylinder(const Vector<3>& pos, double height, double rad, const Quat& rotation);
};

class Sphere : public Object
{
	double rad;

	virtual std::vector<ISR> _intersectLine(const Vector<3>& start, const Vector<3>& dir) const override;
public:
	virtual bool isPointInside(const Vector<3>& p) const override;
	Sphere(const Vector<3>& pos, double rad);
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

bool isPointInsidePolygon(const Vector<2>& p, const std::vector<Vector<2>>& polygon);

double getParamOnShade(const Vector<3>& st, const Vector<3>& dir, const Vector<2>& p);






std::pair<bool, double> intersectLineWithTriangle(const Vector<3>& p, const Vector<3>& dir, const Vector<3>& A, const Vector<3>& B, const Vector<3>& C);


std::pair<int, std::pair<ISR, ISR>> intersectLineWithCircle(const Vector<2>& p, const Vector<2> dir, double rad);



