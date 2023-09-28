#include "linear_algebra.h"
#include <math.h>


Vector<4>::Vector(float x, float y, float z, float t) : nums{ x, y, z, t } {}
Vector<4>::Vector() : nums{ 0,0,0,0 } {}

Vector<3>::Vector(float x, float y, float z) : nums{ x, y, z } {}

Vector<3>::Vector(const Vector<4>& v) : nums{ v.nums[0], v.nums[1], v.nums[2] } {}

Vector<3>::Vector() : nums{ 0,0,0 } {}

Vector<4>::Vector(const Vector<3>& v) : nums{ v.nums[0], v.nums[1], v.nums[2], 1 } {}

Vector<2>::Vector() : nums{ 0,0 } {}

Vector<2>::Vector(const Vector<3>& b) : nums{ b.nums[0], b.nums[1] } {}

Vector<2>::Vector(float x, float y) : nums{ x, y } {}

Vector<3>::Vector(const Vector<2>& v, float z) : nums{ v.nums[0], v.nums[1], z } {}


Vector<3> cross(const Vector<3>& a, const Vector<3>& b)
{
	return Vector<3>(a.nums[1] * b.nums[2] - a.nums[2] * b.nums[1], b.nums[0] * a.nums[2] - a.nums[0] * b.nums[2], a.nums[0] * b.nums[1] - b.nums[0] * a.nums[1]);
}

Quat operator*(const Quat& a, const Quat& b)
{
	return Quat(a.a0() * b.a0() - dot(a.a, b.a), a.a0() * b.a + a.a * b.a0() + cross(a.a, b.a));
}

Quat inverseRot(const Quat& q)
{
	return Quat(q.a0(), -1 * q.a);
}
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

Quat::Quat(float a0, const Vector<3>& a) : _a0(a0), a(a) {}

Quat::Quat(float a0, float a1, float a2, float a3) : _a0(a0), a(a1, a2, a3) {}


