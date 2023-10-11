#include "ComposedObject.h"
#include <assert.h>
std::list<ISR> uniteObjectsSets(const std::list<ISR>& a, const std::list<ISR>& b)
{
	bool in_a = false;
	bool in_b = false;
	std::list<ISR> result;
	auto a_it = a.begin();
	auto b_it = b.begin();
	while (a_it != a.end() && b_it != b.end())
	{
		if (a_it->t < b_it->t)
		{
			if (!in_b)
				result.push_back(*a_it);
			in_a = a_it->in;
			++a_it;
		}
		else
		{
			if (!in_a)
				result.push_back(*b_it);
			in_b = b_it->in;
			++b_it;
		}
	}
	//сейчас одно из них дошло точно до конца поэтому никаких ифов можно не делать
	while (b_it != b.end())
	{
		result.push_back(*b_it);
		++b_it;
	}
	while (a_it != a.end())
	{
		result.push_back(*a_it);
		++a_it;
	}
	return result;
}


std::list<ISR> intersectObjectsSets(const std::list<ISR>& a, const std::list<ISR>& b)
{
	bool in_a = false;
	bool in_b = false;
	std::list<ISR> result;
	auto a_it = a.begin();
	auto b_it = b.begin();
	while (a_it != a.end() && b_it != b.end())
	{
		if (a_it->t < b_it->t)
		{
			if (in_b)
				result.push_back(*a_it);
			in_a = a_it->in;
		}
		else
		{
			if (in_a)
				result.push_back(*b_it);
			in_b = b_it->in;
		}
	}
	return result;
}

std::list<ISR> subtractObjectsSets(const std::list<ISR>& a, const std::list<ISR>& b)
{
	bool in_a = false;
	bool in_b = false;
	std::list<ISR> result;
	auto a_it = a.begin();
	auto b_it = b.begin();
	while (a_it != a.end() && b_it != b.end())
	{
		if (a_it->t < b_it->t)
		{
			if (!in_b)
				result.push_back(*a_it);
			in_a = a_it->in;
		}
		else
		{
			if (in_a)
			{
				result.push_back(*b_it);
				result.back().in = !b_it->in;
				result.back().n = -1 * b_it->n;
			}
			in_b = b_it->in;
		}
	}
	while (a_it != a.end())
	{
		result.push_back(*a_it);
		++a_it;
	}
	return result;
}

//поддерживаем в качестве инварианта то что два раза подр€д in и out идти не может, внутренн€€ точка убираетс€

std::list<ISR> ComposedObject::_intersectLine(const Vector<3>& pos, const Vector<3>& dir) const
{
	auto left_points = left->_intersectLine(pos, dir);
	auto right_points = right->_intersectLine(pos, dir);
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

bool ComposedObject::isPointInside(const Vector<3>& p) const
{
	//потом
	assert(false);
	return false;
}

ComposedObject::ComposedObject(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, Operations oper, const Vector<3>& pos, const Quat& rot) : left(std::move(left)), right(std::move(right)), operation(oper), Object(pos, rot)
{
}

std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot)
{
	
	return std::unique_ptr<Object>();
}
