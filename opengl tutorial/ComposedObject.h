#pragma once
#include "primitives.h"
#include <memory>
class ComposedObject : public Object
{
	enum Operations
	{
		PLUS,
		MULT,
		MINUS
	};
	std::unique_ptr<Object> left, right;
	Operations operation;
	virtual std::list<ISR> _intersectLine(const Vector<3>& pos, const Vector<3>& dir) const override;
public:
	virtual bool isPointInside(const Vector<3>& p) const override;
	ComposedObject(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, Operations oper, const Vector<3>& pos, const Quat& rot);
};

std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);