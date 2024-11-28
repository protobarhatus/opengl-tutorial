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
		MINUS,
		HIERARCHY
	};
private:
	std::unique_ptr<Object> left, right;
	Operation operation;
	virtual std::vector<ISR> _intersectLine(const Vector<3>& pos, const Vector<3>& dir) const override;
	virtual void countBoundingBox() override;
public:
	virtual ObjectType getType() const override;
	virtual std::unique_ptr<Object> copy() const override;
	virtual bool isPointInside(const Vector<3>& p) const override;
	ComposedObject(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, Operation oper, const Vector<3>& pos, const Quat& rot);

	Operation getOperation() const;
	const std::unique_ptr<Object>& getLeft() const;
	const std::unique_ptr<Object>& getRight() const;
	//сецчас не учитывает вращение
	virtual void globalizeCoordinates();
	const Object* getObjectOfId(int id) const override;
	virtual void propagateId(int& id) override;
	void cutSubBoundingBoxes();
};

std::unique_ptr<Object> objectsCombination(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, ComposedObject::Operation oper, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsIntersection(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsSubtraction(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);
std::unique_ptr<Object> objectsHierarchy(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right, const Vector<3>& comm_pos, const Quat& comm_rot);

std::unique_ptr<Object> objectsUnion(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right);
std::unique_ptr<Object> objectsIntersection(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right);
std::unique_ptr<Object> objectsSubtraction(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right);
std::unique_ptr<Object> objectsHierarchy(std::unique_ptr<Object>&& left, std::unique_ptr<Object>&& right);


std::unique_ptr<Object> makeAnHierarchy( std::vector<std::unique_ptr<Object>>&& objs);
std::vector<std::unique_ptr<Object>> dissolveHierarchy(const std::unique_ptr<Object>& obj);
std::pair<Vector<3>, Vector<3>> boundingBox(const std::vector<std::unique_ptr<Object>>& objs);
std::pair<Vector<3>, Vector<3>> boundingBox(const std::unique_ptr<Object>& objs);
std::pair<Vector<3>, Vector<3>> boundingBox(const SceneStruct& objs);
//превращает все плюсы в иерархию начиная от корня, но прерывается при любой другой буленовской операции, т. е. плюсы, родителями которых не является иерархия, не станут иерархией
std::unique_ptr<Object> turnToHierarchy(const std::unique_ptr<Object>& obj);
