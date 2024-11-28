#include "linear_algebra.h"
#include <math.h>
#include <algorithm>

Vector<4>::Vector(double x, double y, double z, double t) : nums{ x, y, z, t } {}
Vector<4>::Vector() : nums{ 0,0,0,0 } {}

Vector<3>::Vector(double x, double y, double z) : nums{ x, y, z } {}

Vector<3>::Vector(const Vector<4>& v) : nums{ v.nums[0], v.nums[1], v.nums[2] } {}

Vector<3>::Vector() : nums{ 0,0,0 } {}

Vector<4>::Vector(const Vector<3>& v) : nums{ v.nums[0], v.nums[1], v.nums[2], 1 } {}

Vector<2>::Vector() : nums{ 0,0 } {}

Vector<2>::Vector(const Vector<3>& b) : nums{ b.nums[0], b.nums[1] } {}

Vector<2>::Vector(double x, double y) : nums{ x, y } {}

Vector<3>::Vector(const Vector<2>& v, double z) : nums{ v.nums[0], v.nums[1], z } {}

Vector<2> max(const Vector<2>& a, const Vector<2>& b)
{
	return Vector<2>(std::max(a.x(), b.x()), std::max(a.y(), b.y()));
}
Vector<3> min(const Vector<3>& a, const Vector<3>& b)
{
	return { std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()) };
}
Vector<2> min(const Vector<2>& a, const Vector<2>& b)
{
	return Vector<2>(std::min(a.x(), b.x()), std::min(a.y(), b.y()));
}
Vector<3> max(const Vector<3>& a, const Vector<3>& b)
{
	return { std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()) };
}

Vector<3> cross(const Vector<3>& a, const Vector<3>& b)
{
	return Vector<3>(a.nums[1] * b.nums[2] - a.nums[2] * b.nums[1], b.nums[0] * a.nums[2] - a.nums[0] * b.nums[2], a.nums[0] * b.nums[1] - b.nums[0] * a.nums[1]);
}

Quat rotationQuat(double angle, const Vector<3>& n)
{
	return Quat(cos(angle / 2), sin(angle / 2) * normalize(n));
}

bool equal(const Quat& a, const Quat& b)
{
	return std::abs(a.a0()- b.a0()) < 1e-10 && equal(a.a, b.a);
}

Quat operator*(const Quat& a, const Quat& b)
{
	return Quat(a.a0() * b.a0() - dot(a.a, b.a), a.a0() * b.a + a.a * b.a0() + cross(a.a, b.a));
}

Quat inverseRot(const Quat& q)
{
	return Quat(q.a0(), -1 * q.a);
}

Quat::Quat() : _a0(1), a({ 0,0,0 }) {}

Matrix<4> Quat::rotation() const
{
	return Matrix<4>(Vector<4>(a1() * a1() + a0() * a0() - a2() * a2() - a3() * a3(), 2 * a2() * a1() - 2 * a3() * a0(), 2 * a3() * a1() + 2 * a2() * a0(), 0),
		Vector<4>(2 * a2() * a1() + 2 * a0() * a3(), a2() * a2() + a0() * a0() - a3() * a3() - a1() * a1(), 2 * a3() * a2() - 2 * a1() * a0(), 0),
		Vector<4>(2 * a3() * a1() - 2 * a0() * a2(), 2 * a3() * a2() + 2 * a1() * a0(), a3() * a3() + a0() * a0() - a2() * a2() - a1() * a1(), 0),
		Vector<4>(0, 0, 0, 1)
		);
}

void Quat::rotate(double angle, const Vector<3>& n) {
	*this = *this * Quat(cos(angle / 2), sin(angle / 2) * normalize(n));
}

Quat::Quat(double a0, const Vector<3>& a) : _a0(a0), a(a) {}

Quat::Quat(double a0, double a1, double a2, double a3) : _a0(a0), a(a1, a2, a3) {}







#include <vector>
Matrix<3> inverse(Matrix<3> matrix)
{
	
	Matrix<3> result = { {1,0,0}, {0,1,0}, {0,0,1} };
	int height = 3;
	int width = 3;
	auto get_index_of_first_non_zero = [width](Vector<3>& vec)->int {
		for (int i = 0; i < width; ++i)
			if (vec.nums[i] != 0)
				return i;
		return width;
	};
	std::vector<std::pair<int, int> > indexes_of_first_non_zero(height);
	auto sort_rows = [height, &get_index_of_first_non_zero, &matrix, &indexes_of_first_non_zero, &result]()->void {
		for (int i = 0; i < height; ++i)
		{
			indexes_of_first_non_zero[i] = { i, get_index_of_first_non_zero(matrix.mat[i]) };
		}
		std::sort(indexes_of_first_non_zero.begin(), indexes_of_first_non_zero.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
			return a.second < b.second; });
		Matrix<3> sorted_matrix;
		Matrix<3> sorted_result;
		for (int i = 0; i < height; ++i)
		{
			//if (i < indexes_of_first_non_zero[i].first)
			//{
			//	std::swap(matrix.mat[i], matrix.mat[indexes_of_first_non_zero[i].first]);
			//	std::swap(result.mat[i], result.mat[indexes_of_first_non_zero[i].first]);
			//}
			sorted_matrix.mat[i] = matrix.mat[indexes_of_first_non_zero[i].first];
			sorted_result.mat[i] = result.mat[indexes_of_first_non_zero[i].first];
		}
		matrix = sorted_matrix;
		result = sorted_result;
	};
	auto divide_on_lead_element = [&matrix, &result, height](int x)->void {
		long double lead_el = matrix.mat[x].nums[x];
		for (int i = x; i < height; ++i)
		{
			matrix.mat[x].nums[i] /= lead_el;
		}
		for (int i = 0; i < height; ++i)
		{
			result.mat[x].nums[i] /= lead_el;
		}
	};
	sort_rows();

	for (int n = 0; n < height; ++n) {
		divide_on_lead_element(n);
		for (int i = n + 1; i < height; ++i)
		{
			long double multiplier = -matrix.mat[i].nums[n] / matrix.mat[n].nums[n];
			matrix.mat[i] = matrix.mat[i] + matrix.mat[n] * (multiplier);
			result.mat[i] = result.mat[i] + result.mat[n] * (multiplier);


		}

		//sort_rows();
	}
	for (int i = 0; i <  height; ++i)
		for (int j = 0; j < width; ++j)
			if (abs(matrix.mat[i].nums[j]) < 1e-20)
				matrix.mat[i].nums[j] = 0;
	for (int n = height - 1; n >= 0; --n)
	{
		for (int i = n - 1; i >= 0; --i)
		{
			long double multiplier = -matrix.mat[i].nums[n] / matrix.mat[n].nums[n];
			matrix.mat[i] = matrix.mat[i] + matrix.mat[n] * (multiplier);
			result.mat[i] = result.mat[i] + result.mat[n] * (multiplier);
		}
	}
	for (int i = 0; i < width; ++i)
		for (int j = 0; j < height; ++j)
			if (abs(matrix.mat[i].nums[j]) < 1e-20)
				matrix.mat[i].nums[j] = 0;
	return result;
}