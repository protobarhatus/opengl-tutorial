#include "primitives.h"


Object::Object(const Vector<3>& pos, const Quat& rot) : position(pos), rotation(rot) {}


Prizm::Prizm(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot) : Object(pos, rot), height(height), base(polygon) {

}


std::pair<double, double> Sphere::_intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect)
{
	double A = dot(dir, dir);
	double B_half = dot(start, dir);
	double C = dot(start, start) - rad * rad;

	double D_4 = B_half * B_half - A * C;
	if (D_4 < 0)
	{
		has_intersect = false;
		return { 0,0 };
	}
	double D_sq = sqrt(D_4);

	return { (-B_half - D_sq) / A,  (-B_half + D_sq) / A };
}

bool Sphere::isPointInside(const Vector<3>& p)
{
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Vector<3> point = (rotation * transposition) * Vector<4>(p);
	return dot(point,point) <= rad*rad;
}

Sphere::Sphere(const Vector<3>& pos, double rad) : Object(pos, Quat(1,0,0,0)), rad(rad)
{
}


bool Prizm::isPointInside(const Vector<3>& p)
{

	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Vector<3> point = (rotation * transposition) * Vector<4>(p);
	return isPointInsidePolygon(point, base) && point.z() >= -height / 2 && point.z() <= height / 2;
}

#include <assert.h>
bool Cone::isPointInside(const Vector<3>& p)
{
	assert(false);
	return false;
}

Cone::Cone(double height, double rad, const Vector<3>& apex_position, const Quat& rot) : Object(apex_position, rot), height(height), rad(rad) {}

bool Piramid::isPointInside(const Vector<3>& p)
{
	assert(false);
	return false;
}

Piramid::Piramid(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot) : Object(pos, rot), height(height), base(polygon) {}

bool Cylinder::isPointInside(const Vector<3>& p)
{
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Vector<3> point = (rotation * transposition ) * Vector<4>(p);
	return dot(Vector<2>(point), Vector<2>(point)) <= rad * rad && point.z() >= -height / 2 && point.z() <= height / 2;
}

Cylinder::Cylinder(const Vector<3>& pos, double height, double rad, const Quat& rotation) : Object(pos, rotation), height(height), rad(rad) {}


std::pair<bool, std::pair<double, double>> Object::intersectWithRayOnBothSides(const Vector<3>& ray_start, const Vector<3>& direction)
{
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Matrix<4> transformation = rotation * transposition;

	Vector<3> start = transformation * Vector<4>(ray_start);
	Vector<3> dir = rotation * Vector<4>(direction);

	bool has_intersections;
	auto intersect = _intersectLine(start, dir, has_intersections);

	if (!has_intersections)
		return { false, {0,0 } };
	if (intersect.second < intersect.first)
		std::swap(intersect.first, intersect.second);
	if (intersect.second < 0)
		return { false, {0,0 } };
	//.first может быть < 0
	return { true, {intersect.first, intersect.second} };
}


std::pair<bool, Vector<3>> Object::intersectWithRay(const Vector<3>& ray_start, const Vector<3>& direction)
{
	
	auto inter = intersectWithRayOnBothSides(ray_start, direction);
	if (inter.first)
	{
		if (inter.second.first < 0)
			return { true, ray_start + inter.second.second * direction };
		return { true, ray_start + inter.second.first * direction };
	}
	return { false, ray_start };
	//Matrix<4> back_transposition(Vector<4>(1, 0, 0, position.x()), Vector<4>(0, 1, 0, position.y()), Vector<4>(0, 0, 1, position.z()), Vector<4>(0, 0, 0, 1));
	//Matrix<4> back_rotation = this->rotation.rotation();

	//if (intersect.first < 0)
	//	return { true, (back_transposition * back_rotation) * Vector<4>(start + dir * intersect.second) };
	//return { true, (back_transposition * back_rotation) * Vector<4>(start + dir * intersect.first) };
}
double getParamOnShade(const Vector<3>& st, const Vector<3>& dir, const Vector<2>& p)
{
	if (equal(dir.x(), 0))
	{
		return (p.y() - st.y()) / dir.y();
	}
	return (p.x() - st.x()) / dir.x();
}

double getParam(const Vector<2>& st, const Vector<2>& dir, const Vector<2>& p)
{
	if (equal(dir.x(), 0))
	{
		return (p.y() - st.y()) / dir.y();
	}
	return (p.x() - st.x()) / dir.x();
}


std::pair<bool,double> rayIntersectsSegment(const Vector<2>& p, const Vector<2>& dir, const Vector<2>& a, const Vector<2>& b)
{
	double div = dir.x() * (b.y() - a.y()) - dir.y() * (b.x() - a.x());
	double num = (b.x() - a.x()) * (p.y() - a.y()) - (b.y() - a.y()) * (p.x() - a.x());
	if (equal(div, 0))
	{
		if (equal(num, 0))
		{
			if (dot(b - p, a - p) < 0)
				return { true, 0 };
			else if (dot(dir, a - p) < 0)
				return { false, 0 };
			else if (dot(a - p, a - p) < dot(b - p, b - p))
				return { true, getParam(p, dir, a) };
			else
				return { true, getParam(p, dir, b) };

		}
		else
			return { false, 0 };
	}
	else
	{
		Vector<2> point = p + dir * (num / div);
		if (dot(b - point, a - point) <= 0)
			return { true, num/div };
		return { false, 0 };
	}

}




std::pair<int, std::pair<double, double>> rayIntersectsPolygon(const Vector<2>& p, const Vector<2>& n, const std::vector<Vector<2>>& polygon)
{
	int c = 0;
	double res[2];
	for (int i = 0; i < polygon.size(); ++i)
	{
		auto int_res = rayIntersectsSegment(p, n, polygon[i], polygon[i < polygon.size() - 1 ? i + 1 : 0]);
		if (!int_res.first)
			continue;
		if (i > 0 && equal(p + n * int_res.second, polygon[i]))
			continue;
		res[c++] = int_res.second;
		if (c == 2)
			return { 2, {res[0], res[1]} };
	}
	return { c, {res[0], res[1]} };

}


bool isPointInsidePolygon(const Vector<2>& p, const std::vector<Vector<2>>& polygon)
{
	std::size_t vertCount = polygon.size();
	if (vertCount < 2)
		return false;
	bool inside = false;
	for (unsigned i = 1; i <= vertCount; ++i)
	{
		const Vector<2>& A = polygon[i - 1];
		const Vector<2>& B = polygon[i % vertCount];
		if ((B.y() <= p.y() && p.y() < A.y()) || (A.y() <= p.y() && p.y() < B.y()))
		{
			double t = (p.x() - B.x()) * (A.y() - B.y()) - (A.x() - B.x()) * (p.y() - B.y());
			if (A.y() < B.y())
				t = -t;
			if (t < 0)
				inside = !inside;
		}
	}

	return inside;
}




std::pair<double, double> Prizm::_intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect)
{
	double EPSILON = 1e-6;
	if (equal(dir.x(), 0) && equal(dir.y(), 0))
	{
		auto in_res = isPointInsidePolygon(start, this->base);
		if (in_res)
		{
			has_intersect = true;
			return { (height / 2 - start.z()) / dir.z(), (-height / 2 - start.z()) / dir.z() };
		}
		has_intersect = false;
		return { 0,0 };
	}
	auto shade_intersect = rayIntersectsPolygon(start, dir, base);

	if (shade_intersect.first == 0)
	{
		has_intersect = false;
		return { 0, 0 };
	}

	double t1 = shade_intersect.second.first;
	double point_z = start.z() + dir.z() * t1;
	if (shade_intersect.first == 1)
	{
		if (point_z >= -height / 2 - EPSILON && point_z <= height / 2 + EPSILON)
		{
			has_intersect = true;
			return { t1, t1 };
		}
		has_intersect = false;
		return { 0,0 };
	}
	double t2 = shade_intersect.second.second;
	double point_z2 = start.z() + dir.z() * t2;
	if (!(point_z >= -height / 2 - EPSILON && point_z <= height / 2 + EPSILON) && (point_z2 >= -height / 2 - EPSILON && point_z2 <= height / 2 + EPSILON))
	{
		std::swap(t1, t2);
		std::swap(point_z, point_z2);
		std::swap(shade_intersect.second.first, shade_intersect.second.second);
	}
	if (point_z >= -height / 2 - EPSILON && point_z <= height / 2 + EPSILON)
	{
		has_intersect = true;

		if (point_z2 >= -height / 2 - EPSILON && point_z2 <= height / 2 + EPSILON)
			return { t1, t2 };
		//z != 0 т к произошел подьем

		if (point_z2 > height / 2 + EPSILON)
		{
			double t_out = (height / 2 - start.z()) / dir.z();
			Vector<2> upper_point = start + dir * t_out;
			//наверно эту проверку можно убрать
			if (isPointInsidePolygon(upper_point, base))
				return { t1, t_out };
			//невозможная ситуация, т к если он не выходит в стене то должен выходить наверху но мало ли
			return { t1, t1 };
		}
		if (point_z2 < -height / 2 - EPSILON)
		{
			double t_out = (-height / 2 - start.z()) / dir.z();
			Vector<2> lower_point = start + dir * t_out;
			//эту проверку также думаю можно убрать
			if (isPointInsidePolygon(lower_point, base))
				return { t1, t_out };
			return { t1, t1 };
		}
	}
	//осталось возможна только ситуация когда линия проходит через оба основания, через верхнее/нижнее ребро, либо вообще не проходит
	if (equal(dir.z(), 0))
	{
		has_intersect = false;
		return { 0, 0 };
	}
	double t_out = (height / 2 - start.z()) / dir.z();
	Vector<2> up_point = start + dir * t_out;
	if (isPointInsidePolygon(up_point, base))
	{
		has_intersect = true;

		return { t_out, (-height / 2 - start.z()) / dir.z() };
	}
	has_intersect = false;
	return{ 0,0 };
}



std::pair<double, double> Cone::_intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect)
{
	return std::pair<double, double>();
}



std::pair<bool, double> intersectLineWithTriangle(const Vector<3>& p, const Vector<3>& dir, const Vector<3>& A, const Vector<3>& B, const Vector<3>& C)
{
	auto a = C - A, b = B - A;
	auto n = cross(a, b);
	if (equal(dot(n, dir), 0))
	{
		//можно разобрать случай когда линия в плоскости но это довольно муторно
		return { false, 0 };
	}
	double t = (dot(A, n) - dot(p, n)) / dot(dir, n);
	Vector<3> projection = p + dir * t;


	b = B - C;
	double L;
	if (!equal(a.z() * b.y(), -a.y() * b.z()))
		L = (a.z() * projection.y() - a.y() * projection.z()) / (a.z() * b.y() + a.y() * b.z());
	else if (!equal(a.x() * b.z(), -a.z() * b.x()))
		L = (a.x() * projection.z() - a.z() * projection.x()) / (a.x() * b.z() + a.z() * b.x());
	else
		L = (a.y() * projection.x() - a.x() * projection.y()) / (a.y() * b.x() + a.x() * b.y());

	Vector<3> AC_proj = projection - L * b;
	if (dot(AC_proj - A, AC_proj - C) < 0)
		return { true, t };
	else
		return { false, 0 };

}



std::pair<double, double> Piramid::_intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect)
{
	int count = 0;
	double t[2];
	if (!equal(dir.z(), 0))
	{
		double t_lev = (-height - start.z()) / dir.z();
		Vector<2> base_inter = start + dir * t_lev;
		if (isPointInsidePolygon(base_inter, base))
			t[count++] = t_lev;
	}
	for (int i = 0; i < base.size(); ++i)
	{
		auto inter = intersectLineWithTriangle(start, dir, Vector<3>(base[i], -height), Vector<3>(base[i < base.size() - 1 ? i + 1 : 0], -height), { 0,0,0 });
		if (inter.first)
			t[count++] = inter.second;
		if (count == 2)
			break;
	}
	has_intersect = count > 0;
	return { t[0], t[1] };
}



std::pair<int, std::pair<double, double>> intersectLineWithCircle(const Vector<2>& p, const Vector<2> dir, double rad)
{
	double A = (dir.x() * dir.x() + dir.y() * dir.y());
	double B_half = p.x() * dir.x() + p.y() * dir.y();
	double C = p.x() * p.x() + p.y() * p.y() - rad * rad;
	double D = B_half * B_half - A * C;
	if (D < 0)
		return { false, {0,0} };
	double D_sq = sqrt(D);
	if (equal(D_sq, 0))
		return { 1, {-B_half / A, -B_half / A} };
	return { 2, {(-B_half + D_sq) / A, (-B_half - D_sq) / A} };
}



std::pair<double, double> Cylinder::_intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect)
{
	if (equal(dir.x(), 0) && equal(dir.y(), 0))
	{
		auto in_res = dot(start, start) < rad * rad;
		if (in_res)
		{
			has_intersect = true;
			return { (height / 2 - start.z()) / dir.z(), (-height / 2 - start.z()) / dir.z() };
		}
		has_intersect = false;
		return { 0,0 };
	}
	auto shade_intersect = intersectLineWithCircle(start, dir, rad);

	if (shade_intersect.first == 0)
	{
		has_intersect = false;
		return { 0, 0 };
	}

	double t1 = shade_intersect.second.first;
	double point_z = start.z() + dir.z() * t1;
	if (shade_intersect.first == 1)
	{
		if (point_z >= -height / 2 && point_z <= height / 2)
		{
			has_intersect = true;
			return { t1, t1 };
		}
		has_intersect = false;
		return { 0,0 };
	}
	double t2 = shade_intersect.second.second;
	double point_z2 = start.z() + dir.z() * t2;
	if (!(point_z >= -height / 2 && point_z <= height / 2) && (point_z2 >= -height / 2 && point_z2 <= height / 2))
	{
		std::swap(t1, t2);
		std::swap(point_z, point_z2);
		std::swap(shade_intersect.second.first, shade_intersect.second.second);
	}
	if (point_z >= -height / 2 && point_z <= height / 2)
	{
		has_intersect = true;

		if (point_z2 >= -height / 2 && point_z2 <= height / 2)
			return { t1, t2 };
		//z != 0 т к произошел подьем

		if (point_z2 > height / 2)
		{
			double t_out = (height / 2 - start.z()) / dir.z();
			return { t1, t_out };

		}
		if (point_z2 < -height / 2)
		{
			double t_out = (-height / 2 - start.z()) / dir.z();
			return { t1, t_out };

		}
	}
	//осталось возможна только ситуация когда линия проходит через оба основания либо вообще не проходит
	if (equal(dir.z(), 0))
	{
		has_intersect = false;
		return { 0, 0 };
	}
	double t_out = (height / 2 - start.z()) / dir.z();
	Vector<2> up_point = start + dir * t_out;
	if (dot(up_point, up_point) < rad * rad)
	{
		has_intersect = true;
		return { t_out, (-height / 2 - start.z()) / dir.z() };
	}
	has_intersect = false;
	return{ 0,0 };
}

