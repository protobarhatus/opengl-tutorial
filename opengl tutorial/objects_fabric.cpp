#include "objects_fabric.h"


std::unique_ptr<Object> makePrizm(const std::vector<Vector<2>>& base, double height, const Vector<3>& position, const Quat& rotation)
{
	return std::make_unique<Prizm>(base, position, height, rotation);
}

std::unique_ptr<Object> makeBox(const Vector<3>& half_size, const Vector<3>& pos, const Quat& rot)
{
	return std::make_unique<Box>(pos, half_size, rot);
}

std::unique_ptr<Object> makeCone(double height, double rad, const Vector<3>& pos, const Quat& rot)
{
	return std::make_unique<Cone>(height, rad, pos, rot);
}

std::unique_ptr<Object> makePiramid(const std::vector<Vector<2>>& base, double height, const Vector<3>& position, const Quat& rotation)
{
	return std::make_unique<Piramid>(base, position, height, rotation);
}

std::unique_ptr<Object> makeCylinder(double height, double rad, const Vector<3>& position, const Quat& rotation)
{
	return std::make_unique<Cylinder>(position, height, rad, rotation);
}

std::unique_ptr<Object> makeSphere(double rad, const Vector<3>& position)
{
	return std::make_unique<Sphere>(position, rad);
}

std::unique_ptr<Object> makePolyhedron(const std::vector<Vector<3>>& points, const std::vector<std::vector<int>>& edges, const Vector<3>& position, const Quat& rotation)
{
	return std::make_unique<Polyhedron>(position, rotation, points, edges);
}


std::unique_ptr<Object> makeEmptyBoxWithWindowsExample(const Vector<3>& position, const Quat& rotation)
{
	std::vector<Vector<2>> square = { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} };
	Quat null_rotation = Quat(1, 0, 0, 0);
	auto cilinders = objectsUnion(std::make_unique<Cylinder>(Cylinder({ 0, 0, 0 }, 2.1, 0.7, Quat(1, 0, 0, 0))),
		objectsUnion(
			std::make_unique<Cylinder>(Vector<3>{ 0, 0, 0 }, 2.1, 0.7, Quat(1. / 1.41, 0, 1. / 1.41, 0)),
			std::make_unique<Cylinder>(Vector<3>{0, 0, 0}, 2.1, 0.7, Quat(1. / 1.41, 1. / 1.41, 0, 0)), { 0,0,0 }, null_rotation),
		{ 0, 0, 0 }, null_rotation);

	auto cube = objectsSubtraction(std::make_unique<Box>(Vector<3>(0, 0, 0), Vector<3>{ 1, 1, 1 }, null_rotation),
		std::move(cilinders), { 0, 0, 0 }, null_rotation);
	cube = objectsSubtraction(std::move(cube), std::make_unique<Box>(Vector<3>(0, 0, 0), Vector<3>{ 0.9, 0.9, 0.9 }, null_rotation), position, rotation);
	return cube;
}

std::unique_ptr<Object> makePolyhedronExample(const Vector<3>& position, const Quat& rotation)
{
	auto poly = std::make_unique<Polyhedron>(position, rotation, std::vector<Vector<3>>{
		{0.469, 0.469, 0.469, },
		{ 0.290,     0.000 ,    0.759 },
		{ -0.759, -0.290,     0.000 },
		{ 0.759 ,    0.290 ,   0.000 },
		{ -0.469,    0.469, -0.469 },
		{ 0.000, -0.759, -0.290 },
		{ -0.759,     0.290 ,    0.000 },
		{ 0.469, -0.469,     0.469 },
		{ -0.469,     0.469,     0.469 },
		{ -0.469, -0.469,     0.469 },
		{ 0.469, -0.469, -0.469 },
		{ 0.290,     0.000, -0.759 },
		{ -0.469, -0.469, -0.469 },
		{ 0.000, -0.759,     0.290 },
		{ 0.000,     0.759, -0.290 },
		{ -0.290,     0.000,     0.759 },
		{ 0.759, -0.290,     0.000 },
		{ -0.290,     0.000, -0.759 },
		{ 0.469,     0.469, -0.469 },
		{ 0.000 ,    0.759,     0.290 }
	},
		std::vector<std::vector<int>>{
			{ 9, 13, 7, 1, 15  },
			{ 6, 4, 14, 19, 8 },
			{ 12, 5 ,13, 9, 2 },
			{ 6, 2 ,12, 17, 4 },
			{ 16, 10, 11, 18, 3 },
			{ 19, 8 ,15, 1, 0 },
			{ 16, 7 ,1, 0, 3 },
			{ 5, 12 ,17, 11 ,10 },
			{ 18, 14 ,4, 17 ,11 },
			{ 16, 10 ,5 ,13 ,7 },
			{ 2, 6 ,8 ,15 ,9 },
			{ 19, 0, 3, 18, 14 }
		});
	return poly;
}

std::unique_ptr<Object> makePolyhedronWithoutCilinderExample(const Vector<3>& position, const Quat& rotation)
{
	auto poly = makePolyhedronExample({ 0,0,0 }, { 1,0,0,0 });
	return objectsSubtraction(std::move(poly),
		std::make_unique<Cylinder>(Vector<3>{ 0, -0.7, 0 }, 1, 0.2, Quat{ 1. / 1.41, 1. / 1.41,0,0 }), { 0,5,0 }, rotation);
}

std::unique_ptr<Object> makeSimpleIntersectionExample(const Vector<3>& position, const Quat& rotation)
{
	auto box = makeBox({ 1,1,1 }, { 0,0,0 }, { 1,0,0,0 });
	auto sphere = makeSphere(1.3, { -1, 0,0 });
	return objectsIntersection(std::move(box), std::move(sphere), position, rotation);
}

std::unique_ptr<Object> makeBoxCutExample(const Vector<3>& position, const Quat& rotation)
{
	auto box = makeEmptyBoxWithWindowsExample({ 0,0,0 }, { 1,0,0,0 });
	auto sphere = makeSphere(1, { 1,-1,1 });
	return objectsIntersection(std::move(box), std::move(sphere), position, rotation);
}

std::unique_ptr<Object> makePolyhedronCutInHalfExample(const Vector<3>& position, const Quat& rotation)
{
	auto poly = makePolyhedronExample({ 0,0,0 }, { 1,0,0,0 });
	auto box = makeBox({ 0.2, 2, 2 }, { 0,0,0 }, { 1,0,0,0 });
	return objectsSubtraction(std::move(poly), std::move(box), position, rotation);
}

std::unique_ptr<Object> makePlatesWithConeCutExample(const Vector<3>& position, const Quat& rotation)
{
	auto box1 = makeBox({ 1, 0.2, 1 }, { 0, -0.5, 0 }, { 1,0,0,0 });
	auto box2 = makeBox({ 1, 0.2, 1 }, { 0, 0.5, 0 }, { 1,0,0,0 });
	auto box3 = makeBox({ 1, 0.2, 1 }, { 0,0,0 }, { 1,0,0,0 });

	auto uni = objectsUnion(std::move(box1), objectsUnion(std::move(box2), std::move(box3), { 0,0,0 }, { 1,0,0,0 }), { 0,0,0 }, { 1,0,0,0 });
	auto cone = makeCone(1.5, 0.5, { 0, 0.5, 0 }, { -1. / 1.41, 1. / 1.41, 0,0 });

	return objectsSubtraction(std::move(uni), std::move(cone), position, rotation);
	return std::move(cone);
}

