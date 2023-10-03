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
    ISR nearest = { 1e20, {0,0,0} };
    for (auto& it : objs)
    {
        //������ summ ������ �������� ��� ����������� �� ������ ��� �� ��� ��� ��� ���
        for (auto& pr : it.summ)
        {
            auto inter = pr->intersectWithRayOnBothSides(start, dir);
            if (!inter.first)
                continue;
            //���� �� ������ �� ��������� ����� ����� ��������, �� ����� ����� �������� ��������� �������� �� ������������ �� ��������� ������� ��� ���
            if (inter.second.first.t < 0)
                assert(false);

            double d_point = inter.second.first.t;
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
                nearest = inter.second.first;
        }
        for (int i = 0; i < it.sub.size(); ++i)
        {
            const std::shared_ptr<Object>& sb = it.sub[i];
            auto inter = sb->intersectWithRayOnBothSides(start, dir);
            if (!inter.first)
                continue;

            double d_point = inter.second.second.t;
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
                nearest = inter.second.second;
        }
    }
    if (nearest.t == 1e20)
        return { false, {0,0,0,0} };
    return { true, nearest };
}
