#pragma once
#include <math.h>
template<int dim>
struct Vector
{

};

template<>
struct Vector<4>
{
	float nums[4];
	Vector();
	Vector(float x, float y, float z, float t);
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
	Vector();
	Vector(float x, float y, float z);
	Vector(const Vector<4>& v);
	Vector(const Vector<2>& v, float z);

	inline float x() const { return nums[0]; }
	inline float y() const { return nums[1]; }
	inline float z() const { return nums[2]; }
};


template<>
struct Vector<2>
{
	float nums[2];
	Vector();
	Vector(const Vector<3>& b);
	Vector(float x, float y);
	inline float x() const { return nums[0]; }
	inline float y() const { return nums[1]; }

};


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



Vector<3> cross(const Vector<3>& a, const Vector<3>& b);

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
bool equal(const Vector<dim>& a, const Vector<dim>& b)
{
	return dot(a - b, a - b) <= 1e-10;
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

	Quat(float a0, float a1, float a2, float a3);
	Quat(float a0, const Vector<3>& a);
	void rotate(double angle, const Vector<3>& n);

	inline float a0() const
	{
		return _a0;
	}
	inline float a1() const { return a.x(); }
	inline float a2() const { return a.y(); }
	inline float a3() const { return a.z(); }


	Matrix<4> rotation() const;

};



Quat operator*(const Quat& a, const Quat& b);

Quat inverseRot(const Quat& q);