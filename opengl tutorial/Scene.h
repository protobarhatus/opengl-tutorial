#pragma once
#include "primitives.h"
#include <memory>



class Scene
{
	struct ObjUnit {
		std::vector<std::shared_ptr<Object>> summ;
		std::vector<std::shared_ptr<Object>> sub;
	};
	std::vector<ObjUnit> objs;
public:
	Scene() {}

	int addObject(const std::shared_ptr<Object>& obj);
	void subtractObject(const std::shared_ptr<Object>& obj, int ind);
	std::pair<bool, ISR> intersection(const Vector<3>& start, const Vector<3>& dir) const;
};

