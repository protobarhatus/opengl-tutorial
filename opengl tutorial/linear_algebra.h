#pragma once
#include <math.h>
template<int dim>
struct Vector
{

};

template<>
struct Vector<4>
{
	double nums[4];
	Vector();
	Vector(double x, double y, double z, double t);
	Vector(const Vector<3>& v);

	inline double x() const { return nums[0]; }
	inline double y() const { return nums[1]; }
	inline double z() const { return nums[2]; }
	inline double t() const { return nums[3]; }
};

template<>
struct Vector<3>
{
	double nums[3];
	Vector();
	Vector(double x, double y, double z);
	Vector(const Vector<4>& v);
	Vector(const Vector<2>& v, double z);

	inline double x() const { return nums[0]; }
	inline double y() const { return nums[1]; }
	inline double z() const { return nums[2]; }
};


template<>
struct Vector<2>
{
	double nums[2];
	Vector();
	Vector(const Vector<3>& b);
	Vector(double x, double y);
	inline double x() const { return nums[0]; }
	inline double y() const { return nums[1]; }

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
double dot(const Vector<dim>& a, const Vector<dim>& b)
{
	double res = 0;
	for (int i = 0; i < dim; ++i)
		res += a.nums[i] * b.nums[i];
	return res;
}



Vector<3> cross(const Vector<3>& a, const Vector<3>& b);

template<int dim>
Vector<dim> operator*(const Vector<dim>& a, double b)
{
	Vector<dim> res;
	for (int i = 0; i < dim; ++i)
		res.nums[i] = a.nums[i] * b;
	return res;
}

template<int dim>
Vector<dim> operator*(double a, const Vector<dim>& b)
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

Vector<3> max(const Vector<3>& a, const Vector<3>& b);

class Quat;
Quat operator*(const Quat& a, const Quat& b);
class Quat
{
private:
	double _a0;
	Vector<3> a;
	friend Quat operator*(const Quat& a, const Quat& b);
	friend Quat inverseRot(const Quat& q);
public:

	Quat(double a0, double a1, double a2, double a3);
	Quat(double a0, const Vector<3>& a);
	void rotate(double angle, const Vector<3>& n);

	inline double a0() const
	{
		return _a0;
	}
	inline double a1() const { return a.x(); }
	inline double a2() const { return a.y(); }
	inline double a3() const { return a.z(); }


	Matrix<4> rotation() const;

};



Quat operator*(const Quat& a, const Quat& b);

Quat inverseRot(const Quat& q);