#include "Scene.h"
#include <assert.h>
int Scene::addObject(const std::shared_ptr<Object>& obj)
{
    objs.push_back({ {obj},{} });
    return objs.size() - 1;
}

void Scene::subtractObject(const std::shared_ptr<Object>& obj, int ind)
{
    objs[ind].sub.push_back(obj);
}

std::pair<bool, ISR> Scene::intersection(const Vector<3>& start, const Vector<3>& dir) const
{
    ISR nearest = { 1e20, {0,0,0}, false, -1 };
    for (auto& it : objs)
    {
        //вообще summ должно работать как пересечение но сейчас это не так так что мда
        for (auto& pr : it.summ)
        {
            auto inter = pr->intersectWithRayOnBothSides(start, dir);
            if (inter.size() == 0)
                continue;
            

            double d_point = inter.begin()->t;
            if (d_point >= nearest.t)
                continue;
            Vector<3> point = start + dir * d_point;
            bool is_subbed = false;
            for (auto& sb : it.sub)
            {
                if (sb->isPointInside(point))
                {
                    is_subbed = true;
                    break;
                }
            }
            if (!is_subbed)
                nearest = inter.front();
        }
        for (int i = 0; i < it.sub.size(); ++i)
        {
            const std::shared_ptr<Object>& sb = it.sub[i];
            auto inter = sb->intersectWithRayOnBothSides(start, dir);
            if (inter.size()==0)
                continue;

            double d_point = std::next(inter.begin())->t;
            if (d_point >= nearest.t)
                continue;
            Vector<3> point = start + dir * d_point;
            bool is_in = false;
            for (auto& pr : it.summ)
            {
                if (pr->isPointInside(point))
                {
                    is_in = true;
                    break;
                }
            }

            bool is_subbed = false;
            for (int j = 0; j < it.sub.size(); ++j)
            {
                if (i == j)
                    continue;

                if (it.sub[j]->isPointInside(point))
                {
                    is_subbed = true;
                    break;
                }
            }

            if (is_in && !is_subbed)
                nearest = *std::next(inter.begin());
        }
    }
    if (nearest.t == 1e20)
        return { false, {0,0,0,0,0, -1} };
    return { true, nearest };
}


