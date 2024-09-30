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

