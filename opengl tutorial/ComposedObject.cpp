#include "ComposedObject.h"
#include <assert.h>
std::vector<ISR> uniteObjectsSets(const std::vector<ISR>& a, const std::vector<ISR>& b)
{
	bool in_a = false;
	bool in_b = false;
	std::vector<ISR> result;
	int a_it = 0;
	int b_it = 0;
	while (a_it != a.size() && b_it != b.size())
	{
		if (a[a_it].t < b[b_it].t)
		{
			if (!in_b)
				result.push_back(a[a_it]);
			in_a = a[a_it].in;
			++a_it;
		}
		else
		{
			if (!in_a)
				result.push_back(b[b_it]);
			in_b = b[b_it].in;
			++b_it;
		}
	}
	//сейчас одно из них дошло точно до конца поэтому никаких ифов можно не делать
	while (b_it != b.size())
	{
		result.push_back(b[b_it]);
		++b_it;
	}
	while (a_it != a.size())
	{
		result.push_back(a[a_it]);
		++a_it;
	}
	return result;
}


std::vector<ISR> intersectObjectsSets(const std::vector<ISR>& a, const std::vector<ISR>& b)
{
	bool in_a = false;
	bool in_b = false;
	std::vector<ISR> result;
	int a_it = 0;
	int b_it = 0;
	while (a_it != a.size() && b_it != b.size())
	{
		if (a[a_it].t < b[b_it].t)
		{
			if (in_b)
				result.push_back(a[a_it]);
			in_a = a[a_it].in;
			++a_it;
		}
		else
		{
			if (in_a)
				result.push_back(b[b_it]);
			in_b = b[b_it].in;
			++b_it;
		}
	}
	return result;
}

std::vector<ISR> subtractObjectsSets(const std::vector<ISR>& a, const std::vector<ISR>& b)
{
	bool in_a = false;
	bool in_b = false;
	std::vector<ISR> result;
	int a_it = 0;
	int b_it = 0;
	while (a_it != a.size() && b_it != b.size())
	{
		if (a[a_it].t < b[b_it].t - 1e-3 * (1 - 2*in_a))
		{
			if (!in_b)
				result.push_back(a[a_it]);
			in_a = a[a_it].in;
			++a_it;
		}
		else
		{
			if (in_a)
			{
				result.push_back(b[b_it]);
				result.back().in = !b[b_it].in;
				result.back().n = -1 * b[b_it].n;
			}
			in_b = b[b_it].in;
			++b_it;
		}
	}
	while (a_it != a.size())
	{
		result.push_back(a[a_it]);
		++a_it;
	}
	return result;
}

//поддерживаем в качестве инварианта то что два раза подряд in и out идти не может, внутренняя точка убирается

std::vector<ISR> ComposedObject::_intersectLine(const Vector<3>& pos, const Vector<3>& dir) const
{
	auto left_points = left->intersectWithRayOnBothSides(pos, dir);
	auto right_points = right->intersectWithRayOnBothSides(pos, dir);
	switch (this->operation)
	{
	case PLUS:
		return uniteObjectsSets(left_points, right_points);
	case MULT:
		return intersectObjectsSets(left_points, right_points);
	case MINUS:
		return subtractObjectsSets(left_points, right_points);
	default:
		assert(false);
		return {};
	}
}



void ComposedObject::countBoundingBox()
{
#ifdef BB_SPHERE
	return max(Vector<3>{ length(left->getPosition()),0,0 } + left->getBoundingBox(), Vector<3>{ length(right->getPosition()),0,0 } + right->getBoundingBox());
#endif
	switch (this->operation)
	{
	case HIERARCHY:
	case PLUS:
	{
		Vector<3> bb_max = max(left->getBoundingBoxPosition() + left->getBoundingBox(), right->getBoundingBoxPosition() + right->getBoundingBox());
		Vector<3> bb_min = min(left->getBoundingBoxPosition() - left->getBoundingBox(), right->getBoundingBoxPosition() - right->getBoundingBox());

		this->bounding_box_position = (bb_max + bb_min) * 0.5;
		this->bounding_box = (bb_max - bb_min) * 0.5;
		break;
	}
	case MULT:
		//пересечение боксов
	{
		Vector<3> a = max(left->getBoundingBoxPosition() - left->getBoundingBox(), right->getBoundingBoxPosition() - right->getBoundingBox());
		Vector<3> b = min(left->getBoundingBoxPosition() + left->getBoundingBox(), right->getBoundingBoxPosition() + right->getBoundingBox());
		this->bounding_box_position = (a + b) * 0.5;
		this->bounding_box = (b - a) * 0.5;
		if (bounding_box.x() <= 0 || bounding_box.y() <= 0 || bounding_box.z() <= 0)
			bounding_box = { 0,0,0 };
		break;
	}
	case MINUS:
		this->bounding_box_position = left->getBoundingBoxPosition();
		this->bounding_box = left->getBoundingBox();
		break;
	default:
		assert(false);
	}
	
}

ObjectType ComposedObject::getType() const
{
	return ObjectType::COMPOSED_OBJECT;
}

std::unique_ptr<Object> ComposedObject::copy() const
{
	auto res = std::make_unique<ComposedObject>(left->copy(), right->copy(), operation, position, rotation);

	res->bounding_box = this->bounding_box;
	res->bounding_box_position = this->bounding_box_position;
	res->left->setParent(res.get());
	res->right->setParent(res.get());
	res->id = this->id;
	return res;
}

bool ComposedObject::isPointInside(const Vector<3>& p) const
{
	//потом
	assert(false);
	return false;
}

ComposedObject::ComposedObject(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, Operation oper, const Vector<3>& pos, const Quat& rot) : left(std::move(left)), right(std::move(right)), operation(oper), Object(pos, rot)
{
	this->left->setParent(this);
	this->right->setParent(this);
	reCountBoundingBox();
}

ComposedObject::Operation ComposedObject::getOperation() const
{
	return this->operation;
}

const std::unique_ptr<Object>& ComposedObject::getLeft() const
{
	return this->left;
}

const std::unique_ptr<Object>& ComposedObject::getRight() const
{
	return this->right;
}

void ComposedObject::globalizeCoordinates()
{
	this->left->moveOn(this->position);
	this->left->globalizeCoordinates();
	this->right->moveOn(this->position);
	this->right->globalizeCoordinates();
	this->position = { 0,0,0 };
	Matrix<4> transposition(Vector<4>(1, 0, 0, 0), Vector<4>(0, 1, 0, 0), Vector<4>(0, 0, 1,0), Vector<4>(0, 0, 0, 1));
	transformation_mat = rotation_mat * transposition;
}

const Object* ComposedObject::getObjectOfId(int id) const
{
	if (this->id == id)
		return this;
	if (this->left->getObjectOfId(id) != nullptr)
		return this->left->getObjectOfId(id);
	return this->right->getObjectOfId(id);
}

void ComposedObject::propagateId(int& id)
{
	this->left->propagateId(id);
	this->right->propagateId(id);
	Object::propagateId(id);
}

void ComposedObject::cutSubBoundingBoxes()
{
	Vector<3> left_max = min(left->getBoundingBoxPosition() + left->getBoundingBox(), this->getBoundingBoxPosition() + this->getBoundingBox());
	Vector<3> left_min = max(left->getBoundingBoxPosition() - left->getBoundingBox(), this->getBoundingBoxPosition() - this->getBoundingBox());
	left->setBoundingBoxPosition((left_max + left_min) / 2);
	left->setBoundingBoxHSize((left_max - left_min) / 2);

	Vector<3> right_max = min(right->getBoundingBoxPosition() + right->getBoundingBox(), this->getBoundingBoxPosition() + this->getBoundingBox());
	Vector<3> right_min = max(right->getBoundingBoxPosition() - right->getBoundingBox(), this->getBoundingBoxPosition() - this->getBoundingBox());
	right->setBoundingBoxPosition((right_max + right_min) / 2);
	right->setBoundingBoxHSize((right_max - right_min) / 2);
}



std::unique_ptr<Object> objectsCombination(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, ComposedObject::Operation oper, const Vector<3>& comm_pos, const Quat& comm_rot)
{
	//left->moveOn(comm_pos);
	//right->moveOn(comm_pos);

	//left->rotate(comm_rot);
	//right->rotate(comm_rot);

	return std::unique_ptr<Object>(new ComposedObject(std::move(left), std::move(right), oper, comm_pos, comm_rot));
}

std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot)
{
	return objectsCombination(std::move(left), std::move(right), ComposedObject::PLUS, comm_pos, comm_rot);
}

std::unique_ptr<Object> objectsIntersection(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot)
{
	return objectsCombination(std::move(left), std::move(right), ComposedObject::MULT, comm_pos, comm_rot);
}

std::unique_ptr<Object> objectsSubtraction(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot)
{
	return objectsCombination(std::move(left), std::move(right), ComposedObject::MINUS, comm_pos, comm_rot);
}

std::unique_ptr<Object> objectsHierarchy(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot)
{
	return objectsCombination(std::move(left), std::move(right), ComposedObject::HIERARCHY, comm_pos, comm_rot);
}

std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right)
{
	return objectsUnion(std::move(left), std::move(right), { 0,0,0 }, { 1,0,0,0 });
}
std::unique_ptr<Object> objectsIntersection(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right)
{
	return objectsIntersection(std::move(left), std::move(right), { 0,0,0 }, { 1,0,0,0 });
}
std::unique_ptr<Object> objectsSubtraction(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right)
{
	return objectsSubtraction(std::move(left), std::move(right), { 0,0,0 }, { 1,0,0,0 });
}
std::unique_ptr<Object> objectsHierarchy(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right)
{
	return objectsHierarchy(std::move(left), std::move(right), { 0,0,0 }, { 1,0,0,0 });
}

std::pair<Vector<3>, Vector<3>> countBoundingBox(const std::vector<std::unique_ptr<Object>>& objs, int start, int count)
{
	Vector<3> vmin = objs[start]->getBoundingBoxPosition() - objs[start]->getBoundingBox();
	Vector<3> vmax = objs[start]->getBoundingBoxPosition() + objs[start]->getBoundingBox();
	for (int i = 1; i < count; ++i)
	{
		vmin = min(vmin, objs[i]->getBoundingBoxPosition() - objs[i]->getBoundingBox());
		vmax = max(vmax, objs[i]->getBoundingBoxPosition() + objs[i]->getBoundingBox());
	}
	return { vmin, vmax };
}
std::pair<Vector<3>, Vector<3>> boundingBox(const std::vector<std::unique_ptr<Object>>& objs)
{
	return countBoundingBox(objs, 0, objs.size());
}
std::pair<Vector<3>, Vector<3>> boundingBox(const std::unique_ptr<Object>& objs)
{
	Vector<3> vmin = objs->getBoundingBoxPosition() - objs->getBoundingBox();
	Vector<3> vmax = objs->getBoundingBoxPosition() + objs->getBoundingBox();
	return { vmin, vmax };
}

std::pair<Vector<3>, Vector<3>> boundingBox(const SceneStruct& objs)
{
	switch (objs.type)
	{
	case SceneStruct::Type::OBJECT:
		return boundingBox(objs.obj_scene);
		break;
	case SceneStruct::Type::VECTOR:
		return boundingBox(objs.vec_scene);
		break;
	default:
		throw "wrong scene type";
	}
}

std::unique_ptr<Object> turnToHierarchy(const std::unique_ptr<Object>& obj)
{
	if (obj->getType() == ObjectType::COMPOSED_OBJECT && static_cast<ComposedObject*>(obj.get())->getOperation() == ComposedObject::Operation::PLUS)
	{
		return objectsHierarchy(turnToHierarchy(static_cast<ComposedObject*>(obj.get())->getLeft()),
			turnToHierarchy(static_cast<ComposedObject*>(obj.get())->getRight()), { 0,0,0 }, { 1,0,0,0 });
	}
	return obj->copy();
}

double surfaceAreaMetric(const Vector<3>& min, const Vector<3>& max)
{
	double a = max.x() - min.x();
	double b = max.y() - min.y();
	double c = max.z() - min.z();
	return a * b + a * c + b * c;
}
double volumeMetric(const Vector<3>& min, const Vector<3>& max)
{
	double a = max.x() - min.x();
	double b = max.y() - min.y();
	double c = max.z() - min.z();
	return a * b * c;
}

int maxPowerOf2LessThan(int n) {
	// Start with the largest power of 2 less than n
	int power = 1;
	while (power * 2 < n) {
		power *= 2;
	}
	return power;
}

class AxisTree
{
	Vector<2> vmin;
	Vector<2> vmax;
	double xmin, xmax;
	AxisTree* left, * right;
	double single_point_place;
	int single_point_direction;
	int sign(double d) { return d > 0 ? 1 : (d < 0 ? -1 : 0); }
public:
	AxisTree(const std::vector<std::pair<Vector<3>, int>>& vec, int axis, double xmin, double xmax) : xmin(xmin), xmax(xmax), vmin(1e50, 1e50), vmax(-1e50, -1e50), left(nullptr), right(nullptr)
	{
		for (auto& it : vec)
		{
			vmin = min(vmin, Vector<2>{ it.first[(axis + 1) % 3], it.first[(axis + 2) % 3] });
			vmax = max(vmax, Vector<2>{ it.first[(axis + 1) % 3], it.first[(axis + 2) % 3] });
		}
		double vecmin = 1e50, vecmax = -1e50;
		std::vector<std::pair<Vector<3>, int>> leftvec, rightvec;
		double div = (xmin + xmax) / 2;
		for (auto& it : vec)
		{
			vecmin = std::min(vecmin, it.first[axis]);
			vecmax = std::max(vecmax, it.first[axis]);
			if (it.first[axis] < div || (it.first[axis] == div && it.second == -1) || ((xmin == div || xmax == div) && it.second == -1))
				leftvec.push_back(it);
			else
				rightvec.push_back(it);
		}
		if ((vecmin == vecmax || vecmin == div || vecmax == div) && !(leftvec.size() > 0 && rightvec.size() > 0))
		{
			single_point_place = vecmin;
			single_point_direction = vec[0].second;
			return;
		}
		if (leftvec.size() > 0)
			left = new AxisTree(leftvec, axis, xmin, div);
		if (rightvec.size() > 0)
			right = new AxisTree(rightvec, axis, div, xmax);
	}
	~AxisTree()
	{
		if (left)
			delete left;
		if (right)
			delete right;
		left = right = nullptr;
	}
	std::pair<Vector<2>, Vector<2>> findBbs(double divpoint, int dir)
	{
		if (left == nullptr && right == nullptr)
		{
			if (single_point_place == divpoint && single_point_direction == dir)
				return { vmin, vmax };
			else if (single_point_place == divpoint)
				return { {1e50, 1e50}, {-1e50, -1e50} };
			if (sign(single_point_place - divpoint) == dir)
				return { vmin, vmax };
			return { {1e50, 1e50}, {-1e50, -1e50} };
		}
		if (divpoint < (xmax + xmin) / 2)
		{
			std::pair<Vector<2>, Vector<2>> res = { {1e50, 1e50}, {-1e50, -1e50} };
			if (left != nullptr)
				res = left->findBbs(divpoint, dir);
			if (right != nullptr && dir == 1)
			{
				res.first = min(res.first, right->vmin);
				res.second = max(res.second, right->vmax);
			}
			return res;
		}
		if (divpoint > (xmax + xmin) / 2)
		{
			std::pair<Vector<2>, Vector<2>> res = { {1e50, 1e50}, {-1e50, -1e50} };
			if (right != nullptr)
				res = right->findBbs(divpoint, dir);
			if (left != nullptr && dir == -1)
			{
				res.first = min(res.first, left->vmin);
				res.second = max(res.second, left->vmax);
			}
			return res;
		}
		if (dir == 1 && right != nullptr)
			return { right->vmin, right->vmax };
		if (dir == -1 && left != nullptr)
			return { left->vmin, left->vmax };
		return { {1e50, 1e50}, {-1e50, -1e50} };
	}
};
#include <algorithm>
#include <set>
std::unique_ptr<Object> makeHierarchySimple(std::vector<std::unique_ptr<Object>>&& bbs, int last_axis=-1)
{
	if (bbs.size() == 1)
		return std::move(bbs[0]);
	if (bbs.size() == 2)
	{
		auto hier = objectsHierarchy(std::move(bbs[0]), std::move(bbs[1]), { 0,0,0 }, { 1,0,0,0 });
		((ComposedObject*)hier.get())->cutSubBoundingBoxes();
		return std::move(hier);
	}
	int axis = (last_axis + 1) % 3;
	int ind_of_min_left = -1;
	double min_left = 1e50;
	for (int i = 0; i < bbs.size(); ++i)
		if (bbs[i]->getBoundingBoxPosition()[axis] - bbs[i]->getBoundingBox()[axis] < min_left)
		{
			ind_of_min_left = i;
			min_left = bbs[i]->getBoundingBoxPosition()[axis] - bbs[i]->getBoundingBox()[axis];
		}
	if (ind_of_min_left != 0)
		std::swap(bbs[0], bbs[ind_of_min_left]);
	std::sort(std::next(bbs.begin()), bbs.end(), [axis](const std::unique_ptr<Object>& a, const std::unique_ptr<Object>& b)->bool {
		return a->getBoundingBoxPosition()[axis] + a->getBoundingBox()[axis] < b->getBoundingBoxPosition()[axis] + b->getBoundingBox()[axis];
		});
	std::vector<std::unique_ptr<Object>> left, right;
	for (int i = 0; i < bbs.size() / 2; ++i)
		left.push_back(std::move(bbs[i]));
	for (int i = bbs.size() / 2; i < bbs.size(); ++i)
		right.push_back(std::move(bbs[i]));
	return objectsHierarchy(makeHierarchySimple(std::move(left), axis), makeHierarchySimple(std::move(right), axis));
}
std::unique_ptr<Object> makeAnHierarchyMSAH(std::vector<std::unique_ptr<Object>>&& bbs, int last_axis)
{
	if (bbs.size() == 1)
		return std::move(bbs[0]);
	if (bbs.size() == 2)
	{
		auto hier = objectsHierarchy(std::move(bbs[0]), std::move(bbs[1]), { 0,0,0 }, { 1,0,0,0 });
		((ComposedObject*)hier.get())->cutSubBoundingBoxes();
		return std::move(hier);
	}
	if (bbs.size() <= 64)
		return makeHierarchySimple(std::move(bbs), last_axis);

	auto metric = surfaceAreaMetric;

	std::vector<std::pair<int, double>> points_of_min;
	int axes_of_min = -1;
	double min_msah = 1e50;
	int div_of_min = -1;
	double axmin_of_min, axmax_of_min;
	for (int i = 0; i < 3; ++i)
	{
		if (i == last_axis)
			continue;
		std::vector<std::pair<int, double>> points_of_axe;
		for (int j = 0; j < bbs.size(); ++j)
		{
			points_of_axe.push_back({ j, (bbs[j]->getBoundingBoxPosition() - bbs[j]->getBoundingBox()).nums[i] });
			points_of_axe.push_back({ j, (bbs[j]->getBoundingBoxPosition() + bbs[j]->getBoundingBox()).nums[i] });
		}
		std::sort(points_of_axe.begin(), points_of_axe.end(), [](std::pair<int, double> a, std::pair<int, double> b)->bool {
			return a.second < b.second;
			});
		std::vector<std::pair<Vector<3>, int>> points;
		double axmin = 1e50, axmax = -1e50;
		for (auto& it : bbs)
		{
			
			Vector<3> p = it->getBoundingBoxPosition() - it->getBoundingBox();
			points.push_back({ p, 1 });

			axmin = std::min(axmin, p[i]);

			p[i] = (it->getBoundingBoxPosition() + it->getBoundingBox())[i];
			points.push_back({ p, -1 });

			p = it->getBoundingBoxPosition() + it->getBoundingBox();
			points.push_back({ p, -1 });

			axmax = std::max(axmax, p[i]);

			p[i] = (it->getBoundingBoxPosition() - it->getBoundingBox())[i];
			points.push_back({ p, 1 });
		}
		AxisTree tree(points, i, axmin, axmax);
		std::set<int> active_objs;
		std::set<int> dead_left;
		std::set<int> dead_right;
		for (int j = 0; j < bbs.size(); ++j)
			dead_right.insert(j);
		double min_msah_of_axe = 1e50;
		int div_of_min_of_axe;
		for (int j = 0; j < points_of_axe.size(); ++j)
		{
			int obj = points_of_axe[j].first;
			if (active_objs.find(obj) != active_objs.end())
			{
				active_objs.erase(obj);
				dead_left.insert(obj);
			}
			double Xdiv = points_of_axe[j].second;
			auto left_bb = tree.findBbs(Xdiv, -1);
			auto right_bb = tree.findBbs(Xdiv, 1);
			double msah = metric(Vector<3>(left_bb.first, axmin), Vector<3>(left_bb.second, Xdiv)) * (1 + dead_left.size() + active_objs.size()) +
				metric(Vector<3>(right_bb.first, Xdiv), Vector<3>(right_bb.second, axmax)) * (1 + active_objs.size() + dead_right.size());
			if (msah < min_msah_of_axe)
			{
				min_msah_of_axe = msah;
				div_of_min_of_axe = j;
			}
			if (dead_right.find(obj) != dead_right.end())
			{
				dead_right.erase(obj);
				active_objs.insert(obj);
			}
		}
		if (min_msah_of_axe < min_msah)
		{
			min_msah = min_msah_of_axe;
			axes_of_min = i;
			div_of_min = div_of_min_of_axe;
			points_of_min = points_of_axe;
			axmin_of_min = axmin;
			axmax_of_min = axmax;
		}
	}
	std::unique_ptr<Object> left, right;
	std::set<int> active_objs;
	std::set<int> dead_left;
	std::set<int> dead_right;
	for (int j = 0; j < bbs.size(); ++j)
		dead_right.insert(j);
	double XDiv;


	for (int j = 0; j <= div_of_min; ++j)
	{
		int obj = points_of_min[j].first;
		if (active_objs.find(obj) != active_objs.end())
		{
			active_objs.erase(obj);
			dead_left.insert(obj);
		}
		if (j == div_of_min)
		{
			XDiv = points_of_min[j].second;
			break;
		}
		if (dead_right.find(obj) != dead_right.end())
		{
			dead_right.erase(obj);
			active_objs.insert(obj);
		}
	}
	auto cur_bb = boundingBox(bbs);
	if (div_of_min == 0 || div_of_min == points_of_min.size() - 1 || (XDiv - axmin_of_min)/(axmax_of_min - axmin_of_min) < 1e-2 || (axmax_of_min - XDiv)/(axmax_of_min - axmin_of_min) < 1e-2 ||
		min_msah >= metric(cur_bb.first, cur_bb.second)*(1 + bbs.size()))
	{
		//если не получается поделить по двум осям, то может получится по третьей (к примеру, кубики в ряд). Но иначе - тупо кое как иерархию составляем, т к обьекты сильно перекрывают друг друга
		if (last_axis != -1)
			return makeAnHierarchyMSAH(std::move(bbs), -1);
		return makeHierarchySimple(std::move(bbs));
	}
	std::vector<std::unique_ptr<Object>> left_objs;
	for (auto& it : dead_left)
		left_objs.push_back(std::move(bbs[it]));
	for (auto& it : active_objs)
	{
		left_objs.push_back(bbs[it]->copy());
		auto bb_min = left_objs.back()->getBoundingBoxPosition() - left_objs.back()->getBoundingBox();
		auto bb_max = left_objs.back()->getBoundingBoxPosition() + left_objs.back()->getBoundingBox();
		bb_max[axes_of_min] = XDiv;
		left_objs.back()->setBoundingBoxPosition((bb_min + bb_max) / 2);
		left_objs.back()->setBoundingBoxHSize((bb_max - bb_min) / 2);
	}
	left = makeAnHierarchyMSAH(std::move(left_objs), axes_of_min);


	std::vector<std::unique_ptr<Object>> right_objs;
	for (auto& it : active_objs)
	{
		right_objs.push_back(std::move(bbs[it]));
		auto bb_min = right_objs.back()->getBoundingBoxPosition() - right_objs.back()->getBoundingBox();
		auto bb_max = right_objs.back()->getBoundingBoxPosition() + right_objs.back()->getBoundingBox();
		bb_min[axes_of_min] = XDiv;
		right_objs.back()->setBoundingBoxPosition((bb_min + bb_max) / 2);
		right_objs.back()->setBoundingBoxHSize((bb_max - bb_min) / 2);
	}
	for (auto& it : dead_right)
		right_objs.push_back(std::move(bbs[it]));
	right = makeAnHierarchyMSAH(std::move(right_objs), axes_of_min);

	return objectsHierarchy(std::move(left), std::move(right));
	
}
std::unique_ptr<Object> makeAnHierarchy(std::vector<std::unique_ptr<Object>>&& objs)
{
	return makeAnHierarchyMSAH(std::move(objs), -1);
	return makeHierarchySimple(std::move(objs));
	return makeAnHierarchyMSAH(std::move(objs), -1);
}

std::vector<std::unique_ptr<Object>> dissolveHierarchy(const std::unique_ptr<Object>& obj)
{
	std::vector<std::unique_ptr<Object>> res;
	if (obj->getType() == ObjectType::COMPOSED_OBJECT && static_cast<ComposedObject*>(obj.get())->getOperation() == ComposedObject::Operation::HIERARCHY)
	{
		auto left = dissolveHierarchy(static_cast<ComposedObject*>(obj.get())->getLeft());
		for (auto& it : left)
			res.push_back(std::move(it));
		auto right = dissolveHierarchy(static_cast<ComposedObject*>(obj.get())->getRight());
		for (auto& it : right)
			res.push_back(std::move(it));
	}
	else
		res.push_back(obj->copy());
	return res;
}