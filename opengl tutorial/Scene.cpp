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

std::pair<bool, Vector<3>> Scene::intersection(const Vector<3>& start, const Vector<3>& dir) const
{
    double nearest = 1e20;
    for (auto& it : objs)
    {
        //вообще summ должно работать как пересечение но сейчас это не так так что мда
        for (auto& pr : it.summ)
        {
            auto inter = pr->intersectWithRayOnBothSides(start, dir);
            if (!inter.first)
                continue;
            //если мы внутри то выход€щую точку нужно рисовать, но тогда нужно отдельно раздел€ть проверку на внутренность на включение границы или нет
            if (inter.second.first < 0)
                assert(false);

            double d_point = inter.second.first;
            if (d_point >= nearest)
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
                nearest = d_point;
        }
        for (auto& sb : it.sub)
        {
            auto inter = sb->intersectWithRayOnBothSides(start, dir);
            if (!inter.first)
                continue;

            double d_point = inter.second.second;
            if (d_point >= nearest)
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
            if (is_in)
                nearest = d_point;
        }
    }
    if (nearest == 1e20)
        return { false, {0,0,0} };
    return { true, start + nearest * dir };
}
