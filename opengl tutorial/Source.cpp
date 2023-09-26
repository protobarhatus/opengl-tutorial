#include <glad/glad.h> // GLAD: https://github.com/Dav1dde/glad ... GLAD 2 also works via the web-service: https://gen.glad.sh/ (leaving all checkbox options unchecked)

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


std::string readFile(const std::string& name)
{
	std::ifstream file(name, std::ios_base::in);
	std::string str{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	return str;
}

void loadShader(int prog, const std::string& name, GLuint type)
{
	unsigned int vert = glCreateShader(type);
	std::string src = readFile(name);
	const char* vert_src = src.c_str();
	glShaderSource(vert, 1, &vert_src, NULL);
	glCompileShader(vert);
	glAttachShader(prog, vert);
}

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <random>

template<int dim>
struct Vector
{

};

template<>
struct Vector<4>
{
	float nums[4];
	Vector() : nums{ 0,0,0,0 } {}
	Vector(float x, float y, float z, float t) : nums{ x, y, z, t } {}
	Vector(const Vector<3>& v);

	inline float x() const { return nums[0]; }
	inline float y() const { return nums[1]; }
	inline float z() const { return nums[2]; }
	inline float t() const { return nums[3]; }
};

template<>
struct Vector<3>
{
	float nums[3];
	Vector() :nums{ 0,0,0 } {}
	Vector(float x, float y, float z) : nums{ x, y, z } {}
	Vector(const Vector<4>& v) : nums{ v.nums[0], v.nums[1], v.nums[2] } {}
	Vector(const Vector<2>& v, float z);

	inline float x() const { return nums[0]; }
	inline float y() const { return nums[1]; }
	inline float z() const { return nums[2]; }
};

Vector<4>::Vector(const Vector<3>& v) : nums{ v.nums[0], v.nums[1], v.nums[2], 1 } {}

template<>
struct Vector<2>
{
	float nums[2];
	Vector() :nums{ 0,0 } {}
	Vector(const Vector<3>& b) : nums{ b.nums[0], b.nums[1] } {}
	Vector(float x, float y) : nums{ x, y } {}
	inline float x() const { return nums[0]; }
	inline float y() const { return nums[1]; }

};
Vector<3>::Vector(const Vector<2>& v, float z) : nums{ v.nums[0], v.nums[1], z } {}

template<int dim>
Vector<dim> operator+(const Vector<dim>& a, const Vector<dim>& b)
{
	Vector<dim> res;
	for (int i = 0; i < dim; ++i)
		res.nums[i] = a.nums[i] + b.nums[i];
	return res;
}
template<int dim>
Vector<dim> operator-(const Vector<dim>& a, const Vector<dim>& b)
{
	Vector<dim> res;
	for (int i = 0; i < dim; ++i)
		res.nums[i] = a.nums[i] - b.nums[i];
	return res;
}

template<int dim>
float dot(const Vector<dim>& a, const Vector<dim>& b)
{
	float res = 0;
	for (int i = 0; i < dim; ++i)
		res += a.nums[i] * b.nums[i];
	return res;
}

Vector<3> cross(const Vector<3>& a, const Vector<3>& b)
{
	return Vector<3>(a.nums[1] * b.nums[2] - a.nums[2] * b.nums[1], b.nums[0] * a.nums[2] - a.nums[0] * b.nums[2], a.nums[0] * b.nums[1] - b.nums[0] * a.nums[1]);
}

template<int dim>
Vector<dim> operator*(const Vector<dim>& a, float b)
{
	Vector<dim> res;
	for (int i = 0; i < dim; ++i)
		res.nums[i] = a.nums[i] * b;
	return res;
}

template<int dim>
Vector<dim> operator*(float a, const Vector<dim>& b)
{
	return b * a;
}

template<int dim>
Vector<dim> normalize(const Vector<dim>& b)
{
	return b * (1.0 / sqrt(dot(b, b)));
}

template<int dim>
struct Matrix
{
	Matrix() {}
};

template<>
struct Matrix<4>
{
	Vector<4> mat[4];
	Matrix(const Vector<4>& a, const Vector<4>& b, const Vector<4>& c, const Vector<4>& d) : mat{ a, b, c, d } {}
	Matrix() {}
};

template<>
struct Matrix<3>
{
	Vector<3> mat[3];
	Matrix(const Vector<3>& a, const Vector<3>& b, const Vector<3>& c) : mat{ a, b, c } {}
	Matrix() {}

};

template<int dim>
Matrix<dim> operator*(const Matrix<dim>& a, const Matrix<dim>& b) {
	Matrix<dim> res;
	for (int i = 0; i < dim; ++i)
	{
		for (int j = 0; j < dim; ++j)
		{
			res.mat[i].nums[j] = 0;
			for (int k = 0; k < dim; ++k)
				res.mat[i].nums[j] += a.mat[i].nums[k] * b.mat[k].nums[j];
		}
	}
	return res;
}

template<int dim>
Vector<dim> operator*(const Matrix<dim>& a, const Vector<dim>& b)
{
	Vector<dim> res;
	for (int i = 0; i < dim; ++i)
		res.nums[i] = dot(a.mat[i], b);
	return res;
}


class Quat;
Quat operator*(const Quat& a, const Quat& b);
class Quat
{
private:
	float _a0;
	Vector<3> a;
	friend Quat operator*(const Quat& a, const Quat& b);
	friend Quat inverseRot(const Quat& q);
public:

	Quat(float a0, float a1, float a2, float a3) : _a0(a0), a(a1, a2, a3) {}
	Quat(float a0, const Vector<3>& a) : _a0(a0), a(a) {}
	void rotate(double angle, const Vector<3>& n) {
		*this = *this * Quat(cos(angle / 2), sin(angle / 2) * normalize(n));
	}

	inline float a0() const
	{
		return _a0;
	}
	inline float a1() const { return a.x(); }
	inline float a2() const { return a.y(); }
	inline float a3() const { return a.z(); }


	Matrix<4> rotation() const
	{
		return Matrix<4>(Vector<4>(a1() * a1() + a0() * a0() - a2() * a2() - a3() * a3(), 2 * a2() * a1() - 2 * a3() * a0(), 2 * a3() * a1() + 2 * a2() * a0(), 0),
			Vector<4>(2 * a2() * a1() + 2 * a0() * a3(), a2() * a2() + a0() * a0() - a3() * a3() - a1() * a1(), 2 * a3() * a2() - 2 * a1() * a0(), 0),
			Vector<4>(2 * a3() * a1() - 2 * a0() * a2(), 2 * a3() * a2() + 2 * a1() * a0(), a3() * a3() + a0() * a0() - a2() * a2() - a1() * a1(), 0),
			Vector<4>(0, 0, 0, 1)
			);
	}


};



Quat operator*(const Quat& a, const Quat& b)
{
	return Quat(a.a0() * b.a0() - dot(a.a, b.a), a.a0() * b.a + a.a * b.a0() + cross(a.a, b.a));
}

Quat inverseRot(const Quat& q)
{
	return Quat(q.a0(), -1 * q.a);
}



class Object
{
protected:
	Vector<3> position;
	Quat rotation;


	virtual std::pair<double, double> _intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect) = 0;
public:
	Object(const Vector<3>& pos, const Quat& rot) : position(pos), rotation(rot) {}

	std::pair<bool, Vector<3>> intersectWithRay(const Vector<3>& start, const Vector<3>& direction);
};

class Prizm : public Object
{

	float height;
	virtual std::pair<double, double> _intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect) override;
	std::vector<Vector<2>> base;
public:
	Prizm(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, float height, const Quat& rot) : Object(pos, rot), height(height), base(polygon) {

	}
};

class Cone : public Object
{
	float height, rad;
	virtual std::pair<double, double> _intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect) override;
public:
	Cone(float height, float rad, const Vector<3>& apex_position, const Quat& rot) : Object(apex_position, rot), height(height), rad(rad) {}
};

class Piramid : public Object
{
	float height;
	virtual std::pair<double, double> _intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect) override;
	std::vector<Vector<2>> base;
public:
	Piramid(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, float height, const Quat& rot) : Object(pos, rot), height(height), base(polygon) {}
};

class Cylinder : public Object
{
	float height, rad;
	virtual std::pair<double, double> _intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect) override;
public:
	Cylinder(const Vector<3>& pos, float height, float rad, const Quat& rotation) : Object(pos, rotation), height(height), rad(rad) {}
};



std::pair<bool, Vector<3>> Object::intersectWithRay(const Vector<3>& ray_start, const Vector<3>& direction)
{
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Matrix<4> transformation = rotation * transposition;

	Vector<3> start = transformation * Vector<4>(ray_start);
	Vector<3> dir = rotation * Vector<4>(direction);

	bool has_intersections;
	auto intersect = _intersectLine(start, dir, has_intersections);

	if (!has_intersections)
		return { false, {0,0,0} };
	if (intersect.second < intersect.first)
		std::swap(intersect.first, intersect.second);
	if (intersect.second < 0)
		return { false, {0,0,0} };

	Matrix<4> back_transposition(Vector<4>(1, 0, 0, position.x()), Vector<4>(0, 1, 0, position.y()), Vector<4>(0, 0, 1, position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> back_rotation = this->rotation.rotation();

	if (intersect.first < 0)
		return { true, (back_transposition * back_rotation) * Vector<4>(start + dir * intersect.second) };
	return { true, (back_transposition * back_rotation) * Vector<4>(start + dir * intersect.first) };
}

bool equal(float a, float b)
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

std::pair<bool, Vector<2>> rayIntersectsSegment(const Vector<2>& p, const Vector<2>& dir, const Vector<2>& a, const Vector<2>& b)
{
	float div = dir.x() * (b.y() - a.y()) - dir.y() * (b.x() - a.x());
	float num = (b.x() - a.x()) * (p.y() - a.y()) - (b.y() - a.y()) * (p.x() - a.x());
	if (equal(div, 0))
	{
		if (equal(num, 0))
		{
			if (dot(b - p, a - p) < 0)
				return { true, p };
			else if (dot(dir, a - p) < 0)
				return { false, {0,0} };
			else if (dot(a - p, a - p) < dot(b - p, b - p))
				return { true, a };
			else
				return { true, b };

		}
		else
			return { false, {0,0} };
	}
	else
	{
		Vector<2> point = p + dir * (num / div);
		if (dot(b - point, a - point) <= 0)
			return { true, point };
		return { false, {0,0} };
	}

}

std::pair<int, std::pair<Vector<2>, Vector<2>>> rayIntersectsPolygon(const Vector<2>& p, const Vector<2>& n, const std::vector<Vector<2>>& polygon)
{
	int c = 0;
	Vector<2> res[2];
	for (int i = 0; i < polygon.size(); ++i)
	{
		auto int_res = rayIntersectsSegment(p, n, polygon[i], polygon[i < polygon.size()-1 ? i+1 : 0]);
		if (!int_res.first)
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
			float t = (p.x() - B.x()) * (A.y() - B.y()) - (A.x() - B.x()) * (p.y() - B.y());
			if (A.y() < B.y())
				t = -t;
			if (t < 0)
				inside = !inside;
		}
	}

	return inside;
}

float getParamOnShade(const Vector<3>& st, const Vector<3>& dir, const Vector<2>& p)
{
	if (equal(dir.x(), 0))
	{
		return (p.y() - st.y()) / dir.y();
	}
	return (p.x() - st.x()) / dir.x();
}


//либо 2 либо 0 либо по грани.
std::pair<double, double> Prizm::_intersectLine(const Vector<3>& start, const Vector<3>& dir, bool& has_intersect)
{
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

	float t1 = getParamOnShade(start, dir, shade_intersect.second.first);
	float point_z = start.z() + dir.z() * t1;
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
	float t2 = getParamOnShade(start, dir, shade_intersect.second.second);
	float point_z2 = start.z() + dir.z() * t2;
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
			float t_out = (height / 2 - start.z()) / dir.z();
			Vector<2> upper_point = start + dir * t_out;
			//наверно эту проверку можно убрать
			if (isPointInsidePolygon(upper_point, base))
				return { t1, t_out };
			//невозможная ситуация, т к если он не выходит в стене то должен выходить наверху но мало ли
			return { t1, t1 };
		}
		if (point_z2 < -height / 2)
		{
			float t_out = (-height / 2 - start.z()) / dir.z();
			Vector<2> lower_point = start + dir * t_out;
			//эту проверку также думаю можно убрать
			if (isPointInsidePolygon(lower_point, base))
				return { t1, t_out };
			return { t1, t1 };
		}
	}
	//осталось возможна только ситуация когда линия проходит через оба основания либо вообще не проходит
	if (equal(dir.z(), 0))
	{
		has_intersect = false;
		return { 0, 0 };
	}
	float t_out = (height / 2 - start.z()) / dir.z();
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
	float t = (dot(A, n) - dot(p, n)) / dot(dir, n);
	Vector<3> projection = p + dir * t;


	b = B - C;
	float L;
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
	float t[2];
	if (!equal(dir.z(), 0))
	{
		float t_lev = (-height - start.z()) / dir.z();
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

std::pair<int, std::pair<double, double>> intersectLineWithCircle(const Vector<2>& p, const Vector<2> dir, float rad)
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

	float t1 = shade_intersect.second.first;
	float point_z = start.z() + dir.z() * t1;
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
	float t2 = shade_intersect.second.second;
	float point_z2 = start.z() + dir.z() * t2;
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
			float t_out = (height / 2 - start.z()) / dir.z();
			return { t1, t_out };

		}
		if (point_z2 < -height / 2)
		{
			float t_out = (-height / 2 - start.z()) / dir.z();
			return { t1, t_out };

		}
	}
	//осталось возможна только ситуация когда линия проходит через оба основания либо вообще не проходит
	if (equal(dir.z(), 0))
	{
		has_intersect = false;
		return { 0, 0 };
	}
	float t_out = (height / 2 - start.z()) / dir.z();
	Vector<2> up_point = start + dir * t_out;
	if (dot(up_point, up_point) < rad*rad)
	{
		has_intersect = true;
		return { t_out, (-height / 2 - start.z()) / dir.z() };
	}
	has_intersect = false;
	return{ 0,0 };
}








GLuint loadTGA_glfw(const char* imagepath) {

	// Создаем одну OpenGL текстуру
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Привязываем" только что созданную текстуру и таким образом все последующие операции будут производиться с ней
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Читаем файл и вызываем glTexImage2D с необходимыми параметрами
	//stbi_set_flip_vertically_on_load(1);
	int width, height, bpp;
	unsigned char* texture = stbi_load(imagepath, &width, &height, &bpp, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	stbi_image_free(texture);


	// Трилинейная фильтрация.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	// Возвращаем идентификатор текстуры который мы создали

	
	return textureID;
}

struct Vertex
{
	Vector<3> position;
	Vector<2> texture_coords;
	Vertex(const Vector<3>& pos, const Vector<2>& text) : position(pos), texture_coords(text) {}
	Vertex(float x, float y, float z, float s, float p) : position(x, y, z), texture_coords(s, p) {}
	Vertex() : position(0,0,0), texture_coords(0,0){}
};



void setCube(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& cube_pos)
{
	Vertex data[] = {
		{-1.f, -1.f, 1.f, 48.0f / 512, 48.0f / 256},
		{1, -1, 1,  48.0 / 512, 64.0 / 256},
		{1, -1, -1, 64.0 / 512, 64.0 / 256},
		{-1, -1, -1, 64.0 / 512, 48.0 / 256},

		{-1, -1, 1, 48.0 / 512, 48.0 / 256},
		{1, -1, 1, 48.0 / 512, 64.0 / 256},
		{1, 1, 1,  64.0 / 512, 64.0 / 256},
		{-1, 1, 1, 64.0 / 512, 48.0 / 256},

		{1, -1, 1, 48.0 / 512, 48.0 / 256},
		{1, -1, -1, 48.0 / 512, 64.0 / 256},
		{1, 1, -1, 64.0 / 512, 64.0 / 256},
		{1, 1, 1, 64.0 / 512, 48.0 / 256},

		{-1, -1, -1, 48.0 / 512, 48.0 / 256},
		{1, -1, -1, 48.0 / 512, 64.0 / 256},
		{1, 1, -1, 64.0 / 512, 64.0 / 256},
		{-1, 1, -1, 64.0 / 512, 48.0 / 256},

		{-1, -1, 1, 48.0 / 512, 48.0 / 256},
		{-1, -1, -1, 48.0 / 512, 64.0 / 256},
		{-1, 1, -1, 64.0 / 512, 64.0 / 256},
		{-1, 1, 1, 64.0 / 512, 48.0 / 256},

		{-1, 1, 1, 48.0 / 512, 48.0 / 256},
		{1, 1, 1, 48.0 / 512, 64.0 / 256},
		{1, 1, -1, 64.0 / 512, 64.0 / 256},
		{-1, 1, -1, 64.0 / 512, 48.0 / 256},
	};
	for (auto& it : data)
		it.position = it.position + cube_pos;
	memcpy(buff + size, data, sizeof(data));
	

	unsigned int elements[] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 15, 14,
		12, 14, 13,

		16, 19, 18,
		16, 18, 17,

		20, 21, 22,
		20, 22, 23
	};
	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, elements, sizeof(elements));

	size += sizeof(data) / sizeof(Vertex);
	ind_size += sizeof(elements) / sizeof(unsigned int);
}

//ужасный костыль чисто нужен на время сейчас т к лень менять сеткуб
void setGreenCube(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& cube_pos)
{
	Vertex data[] = {
		{-1.f, -1.f, 1.f, 40.0f / 512, 4.0 / 256},
		{1, -1, 1,  40.0 / 512, 4.0 / 256},
		{1, -1, -1, 40.0 / 512, 4.0 / 256},
		{-1, -1, -1, 40.0 / 512, 4.0 / 256},

		{-1, -1, 1, 40.0 / 512, 4.0 / 256},
		{1, -1, 1, 40.0 / 512, 4.0 / 256},
		{1, 1, 1,  40.0 / 512, 4.0 / 256},
		{-1, 1, 1, 40.0 / 512, 4.0 / 256},

		{1, -1, 1, 40.0 / 512, 4.0 / 256},
		{1, -1, -1, 40.0 / 512, 4.0 / 256},
		{1, 1, -1, 40.0 / 512, 4.0 / 256},
		{1, 1, 1, 40.0 / 512, 4.0 / 256},

		{-1, -1, -1, 40.0 / 512, 4.0 / 256},
		{1, -1, -1, 40.0 / 512, 4.0 / 256},
		{1, 1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, -1, 40.0 / 512, 4.0 / 256},

		{-1, -1, 1, 40.0 / 512, 4.0 / 256},
		{-1, -1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, 1, 40.0 / 512, 4.0 / 256},

		{-1, 1, 1, 40.0 / 512, 4.0 / 256},
		{1, 1, 1, 40.0 / 512, 4.0 / 256},
		{1, 1, -1, 40.0 / 512, 4.0 / 256},
		{-1, 1, -1, 40.0 / 512, 4.0 / 256},
	};
	for (auto& it : data)
		it.position = it.position * 0.05 + cube_pos;
	memcpy(buff + size, data, sizeof(data));


	unsigned int elements[] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 15, 14,
		12, 14, 13,

		16, 19, 18,
		16, 18, 17,

		20, 21, 22,
		20, 22, 23
	};
	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, elements, sizeof(elements));

	size += sizeof(data) / sizeof(Vertex);
	ind_size += sizeof(elements) / sizeof(unsigned int);
}


void setConvexPrizm(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& center_pos, const std::vector<Vector<2>>& base, float height)
{
	std::vector<Vertex> data(base.size() * 4);
	for (int i = 0; i < base.size(); ++i)
	{
		data[i] = Vertex(Vector<3>(base[i].x(), -height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(48.0 / 512, 64.0 / 256) : Vector<2>(64.0 / 512, 64.0 / 256));
		data[i + base.size()] = Vertex(Vector<3>(base[i].x(), height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(48.0 / 512, 48.0 / 256) : Vector<2>(64.0 / 512, 48.0 / 256));
	}
	for (int i = 0; i < base.size(); ++i)
	{
		data[base.size() * 2 + i] = Vertex(Vector<3>(base[i].x(), -height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(52.0 / 512, 59.0 / 256) : Vector<2>(52.0 / 512, 59.0 / 256));
		data[base.size() * 2 + i + base.size()] = Vertex(Vector<3>(base[i].x(), height / 2, base[i].y()), i % 2 == 0 ? Vector<2>(52.0 / 512, 59.0 / 256) : Vector<2>(52.0 / 512, 59.0 / 256));
	}
	for (auto& it : data)
		it.position = it.position + center_pos;
	memcpy(buff + size, &data[0], data.size() * sizeof(Vertex));

	std::vector<unsigned int> elements(base.size() * 2 * 3 + 2*(base.size() - 2) * 3);
	for (int i = 0; i < base.size(); ++i)
	{
		elements[i*6] = i;
		elements[i * 6 + 1] = (i + 1) % base.size();
		elements[i * 6 + 2] = i + base.size();
		elements[i * 6 + 3] = i + base.size();
		elements[i * 6 + 4] = base.size() + ((i + 1) % base.size());
		elements[i * 6 + 5] = (i + 1) % base.size();
	}
	for (int i = 0; i < base.size() - 2; ++i)
	{
		elements[base.size() * 6 + i * 6] = base.size() * 2;
		elements[base.size() * 6 + i * 6 + 1] = base.size() * 2 + i + 1;
		elements[base.size() * 6 + i * 6 + 2] = base.size() * 2 + i + 2;
		elements[base.size() * 6 + i * 6 + 3] = base.size() * 2 + base.size();
		elements[base.size() * 6 + i * 6 + 4] = base.size() * 2 + base.size() + i + 1;
		elements[base.size() * 6 + i * 6 + 5] = base.size() * 2 + base.size() + i + 2;
	}
	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, &elements[0], elements.size() * sizeof(unsigned int));

	size += data.size();
	ind_size += elements.size();
}

void setConvexPiramid(Vertex* buff, int& size, unsigned int* indicies, int& ind_size, const Vector<3>& center_pos, const std::vector<Vector<2>>& base, float height)
{
	std::vector<Vertex> data(base.size() * 2 + 1);
	for (int i = 0; i < base.size(); ++i)
	{
		data[i] = Vertex(Vector<3>(base[i].x(), -height, base[i].y()), i % 2 == 0 ? Vector<2>(48.0 / 512, 64.0 / 256) : Vector<2>(64.0 / 512, 64.0 / 256));
	}
	for (int i = 0; i < base.size(); ++i)
	{
		data[base.size() + i] = Vertex(Vector<3>(base[i].x(), -height, base[i].y()), i % 2 == 0 ? Vector<2>(52.0 / 512, 59.0 / 256) : Vector<2>(52.0 / 512, 59.0 / 256));
	}
	data[base.size() * 2] = Vertex({ 0,0,0 }, { 48.0 / 512, 48.0 / 256 });
	for (auto& it : data)
		it.position = it.position + center_pos;
	memcpy(buff + size, &data[0], data.size() * sizeof(Vertex));

	std::vector<unsigned int> elements(base.size() * 3 + (base.size() - 2) * 3);
	for (int i = 0; i < base.size(); ++i)
	{
		elements[i * 3] = i;
		elements[i * 3 + 1] = (i + 1) % base.size();
		elements[i * 3 + 2] = base.size() * 2;
	}

	for (int i = 0; i < base.size() - 2; ++i)
	{
		elements[base.size() * 3 + i * 3] = base.size();
		elements[base.size() * 3 + i * 3 + 1] = base.size() + i + 1;
		elements[base.size() * 3 + i * 3 + 2] = base.size() + i + 2;
	}

	for (auto& it : elements)
		it += size;
	memcpy(indicies + ind_size, &elements[0], elements.size() * sizeof(unsigned int));

	size += data.size();
	ind_size += elements.size();
}

int main()
{
	// (1) GLFW: Initialise & Configure
	// -----------------------------------------
	if (!glfwInit())
		exit(EXIT_FAILURE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_DOUBLEBUFFER, 1);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int monitor_width = mode->width; // Monitor's width.
	int monitor_height = mode->height;

	int window_width = 600;
	int window_height = 600;

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "GLFW Test Window – Changing Colours", NULL, NULL);
	// GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Drawing Basic Shapes - Buffer Objects & Shaders", glfwGetPrimaryMonitor(), NULL); // Full Screen Mode ("Alt" + "F4" to Exit!)

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); // Set the window to be used and then centre that window on the monitor. 
	glfwSetWindowPos(window, (monitor_width - window_width) / 2, (monitor_height - window_height) / 2);

	glfwSwapInterval(1); // Set VSync rate 1:1 with monitor's refresh rate.

	 //(2) GLAD: Load OpenGL Function Pointers
	// -------------------------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // For GLAD 2 use the following instead: gladLoadGL(glfwGetProcAddress)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	
	
	Vertex data[1000];
	unsigned int elements[1000];
	int data_size = 0;
	int elements_size = 0;

	//setCube(data, data_size, elements, elements_size, Vector<3>(0, 0, -5));
	//setCube(data, data_size, elements, elements_size, Vector(0, 2, -5));
	//std::vector<Vector<2>> base = { {-0.64, 0.49}, {-0.25, 0.9}, {0.6, 0.75}, {0.83, 0.09}, {0.41, -0.52}, {-0.33, -0.96}, {-0.79, -0.28}, {-0.59, -0.58} };
	//Vector<3> position = { 0,0,-5 };
	float height = 2;
	//setConvexPrizm(data, data_size, elements, elements_size, position, base, height);
	//Prizm cube(base, position, height, Quat(1, 0, 0, 0));

	std::vector<Vector<2>> base = { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} };
	Vector<3> position = { 0, 1, -5 };
	setConvexPiramid(data, data_size, elements, elements_size, position, base, height);

	Piramid piramid(base, Vector<3>(position.x(), position.z(), position.y()), height, Quat{ 1,0,0,0 });

	Vector<3> line_base = { -1, -1, -1 };
	Vector<3> line_dir = { 2, 1, -8 };

	auto intersect = piramid.intersectWithRay(Vector<3>(line_base.x(), line_base.z(), line_base.y()), Vector<3>(line_dir.x(), line_dir.z(), line_dir.y()));
	//auto intersect = cube.intersectWithRay(line_base, line_dir);
	std::swap(intersect.second.nums[1], intersect.second.nums[2]);
	setGreenCube(data, data_size, elements, elements_size, intersect.second);


	//ЭТО ЧИСТО ДЛЯ ЛИНИИ КОСТЫЛЬ
	data[data_size++] = Vertex(line_base, Vector<2>(48.0 / 512, 48.0 / 256));
	data[data_size++] = Vertex(line_base + 10 * line_dir, Vector<2>(48.0 / 512, 48.0 / 256));

	elements[elements_size++] = data_size - 2;
	elements[elements_size++] = data_size - 1;



	glBufferData(GL_ARRAY_BUFFER, data_size*sizeof(Vertex), data, GL_STATIC_DRAW);


	int vertex_dim = sizeof(Vector<3>) / sizeof(float);
	int color_dim = sizeof(Vector<2>) / sizeof(float);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, vertex_dim, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, color_dim, GL_FLOAT, GL_FALSE, (vertex_dim + color_dim) * sizeof(float),(const void*)offsetof(Vertex, texture_coords));

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_DYNAMIC_DRAW);


	unsigned int prog = glCreateProgram();

	
	loadShader(prog, "shader.vert", GL_VERTEX_SHADER);
	loadShader(prog, "shader.frag", GL_FRAGMENT_SHADER);
	
	glEnable(GL_DEPTH_TEST);
	glLinkProgram(prog);
	glValidateProgram(prog);
	glUseProgram(prog);

	int rotation_location = glGetUniformLocation(prog, "u_cr");
	int position_location = glGetUniformLocation(prog, "u_pos");
	int texture_sampler_location = glGetUniformLocation(prog, "u_texture");
	

	auto texture_id = loadTGA_glfw("terrain-atlas copy.tga");
	glUniform1i(texture_sampler_location, 0);

	Quat rot(1, 0, 0, 0);
	Vector<3> camera_pos(0, 0, 1);
	
	int pos_direction = -1;

	glUniform1i(texture_sampler_location, 0);

	auto cstart = glfwGetTime();
	int counter = 0;
	while (!glfwWindowShouldClose(window)) // Main-Loop
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen with... red, green, blue.
		
		glUniform4f(rotation_location, rot.a0(), rot.a1(), rot.a2(), rot.a3());
		glUniform3f(position_location, camera_pos.x(), camera_pos.y(), camera_pos.z());
		glDrawElements(GL_TRIANGLES, elements_size - 2, GL_UNSIGNED_INT, 0);

		glDrawElements(GL_LINES, 3, GL_UNSIGNED_INT, (const void*)((elements_size - 2)*sizeof(unsigned int)));
		

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera_pos.nums[0] += 0.06;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera_pos.nums[0] -= 0.06;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera_pos.nums[1] += 0.06;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera_pos.nums[1] -= 0.06;
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			camera_pos.nums[2] -= 0.06;
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
			camera_pos.nums[2] += 0.06;

		++counter;
		while (glfwGetTime() - cstart < counter * (1.0 / 60.0))
		{
		}
		if (counter % 60 == 0)
			std::cout << counter << std::endl;
	}

	glDeleteTextures(1, &texture_id);
	/* glfwDestroyWindow(window) // Call this function to destroy a specific window */
	glfwTerminate(); // Destroys all remaining windows and cursors, restores modified gamma ramps, and frees resources.

	exit(EXIT_SUCCESS); // Function call: exit() is a C/C++ function that performs various tasks to help clean up resources.
}