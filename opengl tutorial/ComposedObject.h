#pragma once
#include "primitives.h"
#include <memory>
class ComposedObject : public Object
{
public:
	enum Operation
	{
		PLUS,
		MULT,
		MINUS
	};
private:
	std::unique_ptr<Object> left, right;
	Operation operation;
	virtual std::vector<ISR> _intersectLine(const Vector<3>& pos, const Vector<3>& dir) const override;
	virtual Vector<3> countBoundingBox() const override;
public:

	virtual bool isPointInside(const Vector<3>& p) const override;
	ComposedObject(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, Operation oper, const Vector<3>& pos, const Quat& rot);
};

std::unique_ptr<Object> objectsCombination(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, ComposedObject::Operation oper, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsIntersection(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsSubtraction(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);