#include "primitives.h"

enum class PointPolygonRelation
{
	INSIDE,
	BOUNDARY,
	OUTSIDE
};

static int compareDouble(double a, double b, double eps)
{
	if (std::abs(a - b) < eps)
		return 0;
	else if (a < b)
		return -1;
	else
		return 1;
}

static int sideOfPointRelativeToLine(Vector<2> point, Vector<2> low, Vector<2> high)
{
	//considering low.y < high.y
	Vector<2> v1 = high - low;
	Vector<2> v2 = point - low;
	double z_orient = v1.x() * v2.y() - v1.y() * v2.x();
	//z_orient > 0 => ����� �����
	return compareDouble(0, z_orient/length(v1)/length(v2), 1e-4);
}

//�������� �� �� ��������� �� ����� ������ ��������������
//�������� ���� � https://github.com/CGAL/cgal/blob/b83479ee14e2e0b17a31d20f8836787b3cf58623/Polygon/include/CGAL/Polygon_2/Polygon_2_algorithms_impl.h#L396
PointPolygonRelation isPointInsidePolygon(const Vector<2>& p, const std::vector<Vector<2>>& polygon)
{

	int current = 0;
	if (polygon.size() < 2)
		return PointPolygonRelation::OUTSIDE;

	bool IsInside = false;
	int cur_y_comp_res = compareDouble(polygon[current].y(), p.y(), 1e-4);
	int next = 1;
	do // check if the segment (current,next) intersects
	   // the ray { (t,point.y()) | t >= point.x() }
	{
		
		
		int next_y_comp_res = compareDouble(polygon[next].y(), p.y(), 1e-4);

		switch (cur_y_comp_res) {
		case -1:
			switch (next_y_comp_res) {
			case -1:
				break;
			case 0:
				switch (compareDouble(p.x(), polygon[next].x(), 1e-4)) {
				case -1: IsInside = !IsInside; break;
				case 0:   return PointPolygonRelation::BOUNDARY;
				case 1:  break;
				}
				break;
			case 1:
				switch (sideOfPointRelativeToLine(p, polygon[current], polygon[next])) {
				case -1: IsInside = !IsInside; break;
				case  0: return PointPolygonRelation::BOUNDARY;
				}
				break;
			}
			break;
		case 0:
			switch (next_y_comp_res) {
			case -1:
				switch (compareDouble(p.x(), polygon[current].x(), 1e-4)) {
				case -1: IsInside = !IsInside; break;
				case 0:   return PointPolygonRelation::BOUNDARY;
				case 1:  break;
				}
				break;
			case 0:
				switch (compareDouble(p.x(), polygon[current].x(), 1e-4)) {
				case -1:
					if (compareDouble(p.x(), polygon[next].x(), 1e-4) != -1)
						return PointPolygonRelation::BOUNDARY;
					break;
				case 0: return PointPolygonRelation::BOUNDARY;
				case 1:
					if (compareDouble(p.x(), polygon[next].x(), 1e-4) != 1)
						return PointPolygonRelation::BOUNDARY;
					break;
				}
				break;
			case 1:
				if (compareDouble(p.x(), polygon[current].x(), 1e-4) == 0) {
					return PointPolygonRelation::BOUNDARY;
				}
				break;
			}
			break;
		case 1:
			switch (next_y_comp_res) {
			case -1:
				switch (sideOfPointRelativeToLine(p, polygon[next], polygon[current])) {
				case -1: IsInside = !IsInside; break;
				case  0: return PointPolygonRelation::BOUNDARY;
				}
				break;
			case 0:
				if (compareDouble(p.x(), polygon[next].x(), 1e-4) == 0) {
					return PointPolygonRelation::BOUNDARY;
				}
				break;
			case 1:
				break;
			}
			break;
		}

		current = next;
		cur_y_comp_res = next_y_comp_res;
		++next;
		if (next == polygon.size()) next = 0;
	} while (current != 0);

	return IsInside ? PointPolygonRelation::INSIDE : PointPolygonRelation::OUTSIDE;
}


bool checkBBInterseption(int dir_c, int c1, int c2, const Vector<3>& bounding_box, const Vector<3>& start, const Vector<3>& dir)
{
	double xpT = (bounding_box.nums[dir_c] - start.nums[dir_c]) / dir.nums[dir_c];
	double yatx = start.nums[c1] + dir.nums[c1] * xpT;
	double zatx = start.nums[c2] + dir.nums[c2] * xpT;
	if (abs(yatx) < bounding_box.nums[c1] && abs(zatx) < bounding_box.nums[c2])
		return true;

	xpT = (-bounding_box.nums[dir_c] - start.nums[dir_c]) / dir.nums[dir_c];
	yatx = start.nums[c1] + dir.nums[c1] * xpT;
	zatx = start.nums[c2] + dir.nums[c2] * xpT;
	if (abs(yatx) < bounding_box.nums[c1] && abs(zatx) < bounding_box.nums[c2])
		return true;

	return false;
}
bool Object::lineIntersectsBoundingBox(const Vector<3>& start, const Vector<3>& dir) const
{
#ifdef BB_SPHERE
	double A = dot(dir, dir);
	double B_half = dot(start, dir);
	double C = dot(start, start) - bounding_box.nums[0] * bounding_box.nums[0];

	double D_4 = B_half * B_half - A * C;
	return D_4 >= 0;
#endif
	if (!equal(dir.x(), 0))
	{
		if (checkBBInterseption(0, 1, 2, bounding_box, start, dir))
			return true;
	}
	if (!equal(dir.y(), 0))
	{
		if (checkBBInterseption(1, 0, 2, bounding_box, start, dir))
			return true;
	}
	if (!equal(dir.z(), 0))
	{
		if (checkBBInterseption(2, 0, 1, bounding_box, start, dir))
			return true;
	}
	return false;
}

void Object::rotateBoundingBox()
{
	
	Vector<3> a = bounding_box + bounding_box_position - position;
	Vector<3> b = Vector<3>{ -bounding_box.x(), bounding_box.y(), bounding_box.z() } + bounding_box_position - position;
	Vector<3> c = Vector<3>{ bounding_box.x(), -bounding_box.y(), bounding_box.z() } + bounding_box_position - position;
	Vector<3> d = Vector<3>{ -bounding_box.x(), -bounding_box.y(), bounding_box.z() } + bounding_box_position - position;
	Vector<3> e = Vector<3>{ bounding_box.x(), bounding_box.y(), -bounding_box.z() } + bounding_box_position - position;
	Vector<3> f = Vector<3>{ -bounding_box.x(), bounding_box.y(), -bounding_box.z() } + bounding_box_position - position;
	Vector<3> g = Vector<3>{ bounding_box.x(), -bounding_box.y(), -bounding_box.z() } + bounding_box_position - position;
	Vector<3> h = Vector<3>{ -bounding_box.x(), -bounding_box.y(), -bounding_box.z() } + bounding_box_position - position;

	a = back_rotation_mat * Vector<4>(a);
	b = back_rotation_mat * Vector<4>(b);
	c = back_rotation_mat * Vector<4>(c);
	d = back_rotation_mat * Vector<4>(d);
	e = back_rotation_mat * Vector<4>(e);
	f = back_rotation_mat * Vector<4>(f);
	g = back_rotation_mat * Vector<4>(g);
	h = back_rotation_mat * Vector<4>(h);

	Vector<3> bb_max = max(a, max(b, max(c, max(d, max(e, max(f, max(g, h))))))) + position;
	Vector<3> bb_min = min(a, min(b, min(c, min(d, min(e, min(f, min(g, h))))))) + position;

	this->bounding_box = (bb_max - bb_min) * 0.5;
	//this->bounding_box_position = back_rotation_mat * Vector<4>(this->bounding_box_position);
	this->bounding_box_position = (bb_max + bb_min) * 0.5;
}


Object::Object(const Vector<3>& pos, const Quat& rot) : position(pos), rotation(rot) {
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	rotation_mat = inverseRot(this->rotation).rotation();
	transformation_mat = rotation_mat * transposition;
	back_rotation_mat = this->rotation.rotation();
	


}
#include <assert.h>
const Object* Object::getObjectOfId(int id) const
{
	assert(this->getType() != ObjectType::COMPOSED_OBJECT);
	if (this->id == id)
		return this;
	return nullptr;
}

int Object::getId() const {
	return id;
}

void Object::propagateId(int& idcounter)
{
	if (this->id == -1)
		this->setId(idcounter++);
}

void Object::setId(int i)
{
	id = i;
}

static int sign(double d)
{
	return d > 0 ? 1 : -1;
}
void Prizm::countBoundingBox()
{
#ifdef BB_SPHERE
	double max = 0;
	for (auto& it : base)
		max = std::max(max, sqrt(dot(it, it) + half_height * half_height));
	return { max,0,0 };
#endif
	Vector<2> base_bb_max = { 0,0 };
	Vector<2> base_bb_min = { 0,0 };
	for (auto& it : base)
	{
		base_bb_max = max(base_bb_max, it);
		base_bb_min = min(base_bb_min, it);
	}
	this->bounding_box = Vector<3>( (base_bb_max - base_bb_min) * 0.5, half_height );
	this->bounding_box_position = position + Vector<3>((base_bb_max + base_bb_min) * 0.5, 0);
}
Prizm::Prizm(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot) : Object(pos, rot), half_height(height / 2), base(polygon) {
	normals.resize(base.size());
	for (int i = 0; i < base.size(); ++i)
	{
		Vector<2> normal = { polygon[i < polygon.size() - 1 ? i + 1 : 0].y() - polygon[i].y(), polygon[i].x() - polygon[i < polygon.size() - 1 ? i + 1 : 0].x() };
		normals[i] = { normalize(normal) * sign(dot(polygon[i], normal)), 0 };
	}
	reCountBoundingBox();
}




std::vector<ISR> Sphere::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{
	double A = dot(dir, dir);
	double B_half = dot(start, dir);
	double C = dot(start, start) - rad * rad;

	double D_4 = B_half * B_half - A * C;
	if (D_4 < 0)
	{
		return {};
	}
	double D_sq = sqrt(D_4);
	double t1 = (-B_half - D_sq) / A;
	double t2 = (-B_half + D_sq) / A;

	return { ISR{t1, (start + dir * t1) * (1.0 / rad), true, this->id},  ISR{t2, (start + dir * t2) * (1.0 / rad), false , this->id} };
}

void Sphere::countBoundingBox()
{
	this->bounding_box = { rad, rad, rad };
	this->bounding_box_position = this->position;
}

bool Sphere::isPointInside(const Vector<3>& p) const
{
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Vector<3> point = (rotation * transposition) * Vector<4>(p);
	return dot(point,point) <= rad*rad;
}

Sphere::Sphere(const Vector<3>& pos, double rad) : Object(pos, Quat(1,0,0,0)), rad(rad)
{
	reCountBoundingBox();
}


bool Prizm::isPointInside(const Vector<3>& p) const
{

	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Vector<3> point = (rotation * transposition) * Vector<4>(p);
	return isPointInsidePolygon(point, base) != PointPolygonRelation::OUTSIDE && point.z() >= -half_height && point.z() <= half_height;
}

#include <assert.h>
bool Cone::isPointInside(const Vector<3>& p) const
{
	assert(false);
	return false;
}

Cone::Cone(double height, double rad, const Vector<3>& apex_position, const Quat& rot) : Object(apex_position, rot), height(height), rad(rad), rdivh(rad/height) {
	reCountBoundingBox();
}

bool Piramid::lineIntersectsBoundingBox(const Vector<3>& start, const Vector<3>& dir) const
{
	return Object::lineIntersectsBoundingBox({ start.x(), start.y(), start.z() + height / 2 }, dir);
}

bool Piramid::isPointInside(const Vector<3>& p) const
{
	assert(false);
	return false;
}

Piramid::Piramid(const std::vector<Vector<2>>& polygon, const Vector<3>& pos, double height, const Quat& rot) : Object(pos, rot), height(height), base(polygon)
{
	normals.resize(base.size());
	for (int i = 0; i < base.size(); ++i)
	{
		normals[i] = normalize(cross({ base[i], -height }, { base[(i + 1) % base.size()], -height }));
	}
	reCountBoundingBox();
}

bool Cylinder::isPointInside(const Vector<3>& p) const
{
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	Matrix<4> rotation = inverseRot(this->rotation).rotation();
	Vector<3> point = (rotation * transposition ) * Vector<4>(p);
	return dot(Vector<2>(point), Vector<2>(point)) <= rad * rad && point.z() >= -half_height && point.z() <= half_height;
}

Cylinder::Cylinder(const Vector<3>& pos, double height, double rad, const Quat& rotation) : Object(pos, rotation), half_height(height/2), rad(rad) {
	reCountBoundingBox();
}


void Object::setColor(const Vector<3>& col)
{
	color = Vector<4>(col.x(), col.y(), col.z(), color.t());
}
void Object::setAlpha(double a)
{
	color.nums[3] = a;
}
Vector<4> Object::getColor() const
{
	return color;
}


std::vector<ISR> Object::intersectWithRayOnBothSides(const Vector<3>& ray_start, const Vector<3>& direction) const
{

	Vector<3> start = transformation_mat * Vector<4>(ray_start);
	Vector<3> dir = rotation_mat * Vector<4>(direction);
	//if (!lineIntersectsBoundingBox(start, dir))
	//	return { false, {0,0,0,0,0} };

	auto intersect = _intersectLine(start, dir);


	
	for (auto it = intersect.begin(); it != intersect.end(); )
	{
		if (it->t < 0)
		{
			it = intersect.erase(it);
			continue;
		}
		
		it->n = back_rotation_mat * Vector<4>(it->n);
		++it;
	}
	return intersect;
}

void Object::moveOn(const Vector<3>& movement)
{
	position = position + movement;
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	transformation_mat = rotation_mat * transposition;
	this->bounding_box_position = bounding_box_position + movement;
}

void Object::globalizeCoordinates()
{

}

void Object::rotate(const Quat& rotation)
{
	this->rotation = this->rotation * rotation;
	this->rotation_mat = inverseRot(this->rotation).rotation();
	this->back_rotation_mat = this->rotation.rotation();
	Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	transformation_mat = rotation_mat * transposition;
	this->reCountBoundingBox();
}

void Object::reCountBoundingBox()
{
	this->countBoundingBox();
	this->rotateBoundingBox();
}

Vector<3> Object::getBoundingBox() const
{
	return this->bounding_box;
}

Matrix<4> Object::getTransformationMatrix() const
{
	return this->transformation_mat;
}
std::pair<bool, ISR> Object::intersectWithRay(const Vector<3>& ray_start, const Vector<3>& direction) const
{
	
	auto inter = intersectWithRayOnBothSides(ray_start, direction);
	if (inter.size() > 0)
		return { true, *inter.begin() };
	return { false, {0,0,0,0,0, this->id} };
	//Matrix<4> back_transposition(Vector<4>(1, 0, 0, position.x()), Vector<4>(0, 1, 0, position.y()), Vector<4>(0, 0, 1, position.z()), Vector<4>(0, 0, 0, 1));
	//Matrix<4> back_rotation = this->rotation.rotation();

	//if (intersect.first < 0)
	//	return { true, (back_transposition * back_rotation) * Vector<4>(start + dir * intersect.second) };
	//return { true, (back_transposition * back_rotation) * Vector<4>(start + dir * intersect.first) };
}

bool Object::isItIdOfObjectOrItsParent(int id_to_check) const
{
	if (this->id == id_to_check)
		return true;
	return this->parent != nullptr && this->parent->isItIdOfObjectOrItsParent(id_to_check);
}
void Object::setParent(const Object* parent)
{
	this->parent = parent;
}
double getParamOnShade(const Vector<3>& st, const Vector<3>& dir, const Vector<2>& p)
{
	if (equal(dir.x(), 0))
	{
		return (p.y() - st.y()) / dir.y();
	}
	return (p.x() - st.x()) / dir.x();
}

double getParam(const Vector<2>& st, const Vector<2>& dir, const Vector<2>& p)
{
	if (equal(dir.x(), 0))
	{
		return (p.y() - st.y()) / dir.y();
	}
	return (p.x() - st.x()) / dir.x();
}


std::pair<bool,double> rayIntersectsSegment(const Vector<2>& p, const Vector<2>& dir, const Vector<2>& a, const Vector<2>& b)
{
	double div = dir.x() * (b.y() - a.y()) - dir.y() * (b.x() - a.x());
	double num = (b.x() - a.x()) * (p.y() - a.y()) - (b.y() - a.y()) * (p.x() - a.x());
	if (equal(div, 0))
	{
		if (equal(num, 0))
		{
			if (dot(b - p, a - p) < 0)
				return { true, 0 };
			else if (dot(dir, a - p) < 0)
				return { false, 0 };
			else if (dot(a - p, a - p) < dot(b - p, b - p))
				return { true, getParam(p, dir, a) };
			else
				return { true, getParam(p, dir, b) };

		}
		else
			return { false, 0 };
	}
	else
	{
		Vector<2> point = p + dir * (num / div);
		
		if (abs(dot(b - point, a - point)/length(b-point)/length(a-point) - (-1)) <= 1e-2)
			return { true, num/div };
		return { false, 0 };
	}

}




std::pair<int, std::pair<ISR, ISR>> rayIntersectsPolygon(const Vector<2>& p, const Vector<2>& n, const std::vector<Vector<2>>& polygon, const std::vector<Vector<3>>& normals, int id)
{
	int c = 0;
	ISR res[2];
	//auto n_ = normalize(n);
	for (int i = 0; i < polygon.size(); ++i)
	{
		auto int_res = rayIntersectsSegment(p, n, polygon[i], polygon[i < polygon.size() - 1 ? i + 1 : 0]);
		if (!int_res.first)
			continue;
		Vector<2> point = p + n* int_res.second;
		//if (i > 0 && equal(point, polygon[i]))
		//	continue;
		if (c == 1 && equal(int_res.second, res[0].t))
			continue;
		res[c++] = { int_res.second, normals[i], false, id };
		if (c == 2)
			break;
	}
	if (c == 1)
		return { 0, {res[0], res[1]} };
	if (res[0].t < res[1].t)
		res[0].in = true;
	else
		res[1].in = true;
	return { c, {res[0], res[1]} };

}


bool __depr__isPointInsidePolygon(const Vector<2>& p, const std::vector<Vector<2>>& polygon)
{
	std::size_t vertCount = polygon.size();
	if (vertCount < 2)
		return false;
	bool inside = false;
	for (unsigned i = 1; i <= vertCount; ++i)
	{
		const Vector<2>& A = polygon[i - 1];
		const Vector<2>& B = polygon[i % vertCount];
		if ((B.y() <= p.y() && p.y() < A.y()) || (A.y() <= p.y() && p.y() < B.y()))
		{
			double t = (p.x() - B.x()) * (A.y() - B.y()) - (A.x() - B.x()) * (p.y() - B.y());
			if (A.y() < B.y())
				t = -t;
			if (t < 0)
				inside = !inside;
		}
	}

	return inside;
}


std::pair<bool, double> intersectLineWithTriangle(const Vector<3>& p, const Vector<3>& dir, const Vector<3>& A, const Vector<3>& B, const Vector<3>& C);

std::vector<ISR> Prizm::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{

	if (!is_convex)
		assert(false);

	double EPSILON = 1e-6;
	if (equal(dir.x(), 0) && equal(dir.y(), 0))
	{
		auto in_res = isPointInsidePolygon(start, this->base);
		if (in_res != PointPolygonRelation::OUTSIDE)
		{
			double t_up = (half_height - start.z()) / dir.z();
			double t_down = (-half_height - start.z()) / dir.z();
			if (t_up < t_down)
				return std::vector<ISR>{ {t_up, {0,0,1}, true, this->id}, {t_down, {0,0,-1}, false, this->id} };
			else
				return { {t_down, {0,0,-1}, true, this->id} , {t_up, {0,0,1}, false, this->id} };
		}
		return {};
	}
	auto shade_intersect = rayIntersectsPolygon(start, (dir), base, normals, this->id);

	if (shade_intersect.first == 0)
	{
		return {};
	}

	ISR t1 = shade_intersect.second.first;
	double point_z = start.z() + dir.z() * t1.t;
	if (shade_intersect.first == 1)
	{
		//���� ���� ���� ����������� �� �� ������ � ��� �� ������ ��� ���������
		assert(false);
		if (point_z >= -half_height - EPSILON && point_z <= half_height + EPSILON)
		{
			//���� ��� �� ������� ���� �� ���� �� ������ ���������� ��������� in � out
			return { t1, t1 };
		}
		return {};
	}
	ISR t2 = shade_intersect.second.second;
	double point_z2 = start.z() + dir.z() * t2.t;
	if (!(point_z >= -half_height - EPSILON && point_z <= half_height + EPSILON) && (point_z2 >= -half_height - EPSILON && point_z2 <= half_height + EPSILON))
	{
		std::swap(t1, t2);
		std::swap(point_z, point_z2);
		std::swap(shade_intersect.second.first, shade_intersect.second.second);
	}
	if (point_z >= -half_height - EPSILON && point_z <= half_height + EPSILON)
	{

		if (point_z2 >= -half_height - EPSILON && point_z2 <= half_height + EPSILON)
		{
			if (t1.in)
				return { t1, t2 };
			else
				return { t2, t1 };
		}
		//z != 0 � � ��������� ������

		if (point_z2 > half_height + EPSILON)
		{
			double t_out = (half_height - start.z()) / dir.z();
			Vector<2> upper_point = start + dir * t_out;
			//������� ��� �������� ����� ������
			//if (isPointInsidePolygon(upper_point, base))
			if (t1.t < t_out)
			{
				t1.in = true;
				return { t1, {t_out, {0, 0, 1}, false, this->id } };
			}
			else
			{
				t1.in = false;
				return {  {t_out, {0, 0, 1}, true, this->id }, t1 };
			}
			//����������� ��������, � � ���� �� �� ������� � ����� �� ������ �������� ������� �� ���� ��
			return { t1, t1 };
		}
		if (point_z2 < -half_height - EPSILON)
		{
			double t_out = (-half_height - start.z()) / dir.z();
			Vector<2> lower_point = start + dir * t_out;
			//��� �������� ����� ����� ����� ������
			//if (isPointInsidePolygon(lower_point, base))
			if (t1.t < t_out)
			{
				t1.in = true;
				return { t1, {t_out, {0, 0, -1}, false, this->id } };
			}
			else
			{
				t1.in = false;
				return { {t_out, {0, 0, -1}, true, this->id }, t1 };
			}
			return { t1, t1 };
		}
	}
	//�������� �������� ������ �������� ����� ����� �������� ����� ��� ���������, ����� �������/������ �����, ���� ������ �� ��������
	if (equal(dir.z(), 0))
	{
		return {};
	}
	double t_out = (half_height - start.z()) / dir.z();
	Vector<2> up_point = start + dir * t_out;
	if (isPointInsidePolygon(up_point, base) != PointPolygonRelation::OUTSIDE)
	{
		double t_bot = (-half_height - start.z()) / dir.z();
		if (t_out < t_bot)
			return { {t_out, {0,0,1}, true, this->id},  {t_bot, {0,0,-1}, false, this->id} };
		else
			return { {t_bot, {0,0,-1}, true, this->id},  {t_out, {0,0,1}, false, this->id} };
	}

	return {};
}





std::vector<ISR> Cone::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{
	double M_s = rdivh * rdivh;
	double A = dir.x() * dir.x() + dir.y() * dir.y() - M_s * dir.z() * dir.z();
	double B_half = dir.x() * start.x() + dir.y() * start.y() - M_s * dir.z() * start.z();
	double C = start.x() * start.x() + start.y() * start.y() - M_s * start.z() * start.z();
	double D_4 = B_half * B_half - A * C;
	if (D_4 < 0)
		return {};
	double D_sq_2 = sqrt(D_4);
	double t1 = (-B_half - D_sq_2) / A;
	double t2 = (-B_half + D_sq_2) / A;

	double z1 = start.z() + dir.z() * t1;
	double z2 = start.z() + dir.z() * t2;

	bool z1_good = z1 >= -height && z1 <= 0;
	bool z2_good = z2 >= -height && z2 <= 0;
	if (z1_good && z2_good)
	{
		Vector<2> p1 = start + dir * t1;
		Vector<2> p2 = start + dir * t2;

		double zn1 = -dot(p1, p1) / z1;
		double zn2 = -dot(p2, p2) / z2;
		return { {t1, normalize(Vector<3>{p1, zn1}), true, this->id}, {t2, normalize(Vector<3>{p2, zn2}), false, this->id} };
	}
	if (!z1_good && !z2_good)
		return {};
	if (z2_good)
	{
		std::swap(t1, t2);
		std::swap(z1, z2);
	}
	//���� ����� ���������� ��������� ����� � ����� ����� � �� ������� � ������, �� ��� �������������� ������� ����� ���������
	Vector<2> p1 = start + dir * t1;
	double zn1 = -dot(p1, p1) / z1;

	double t_base = (-height - start.z()) / dir.z();
	if (t_base < t1)
		return { {t_base, {0,0,-1}, true, this->id}, {t1, normalize(Vector<3>{p1, zn1}), false, this->id} };
	return { {t1, normalize(Vector<3>{p1, zn1}), true, this->id}, {t_base, {0,0,-1}, false, this->id} };

}

void Cone::countBoundingBox()
{
#ifdef BB_SPHERE
	return { sqrt(height * height / 4 + rad * rad),0,0 };
#endif
	this->bounding_box = { rad, rad, height / 2 };
	this->bounding_box_position = this->position - Vector<3>{0, 0, height / 2};
}

bool Cone::lineIntersectsBoundingBox(const Vector<3>& start, const Vector<3>& dir) const
{
	return Object::lineIntersectsBoundingBox({ start.x(), start.y(), start.z() + height / 2 }, dir);
}

Vector<3> Object::getPosition() const
{
	return this->position;
}

std::pair<bool, double> intersectLineWithTriangle(const Vector<3>& p, const Vector<3>& dir, const Vector<3>& A, const Vector<3>& B, const Vector<3>& C)
{
	auto a = C - A, b = B - A;
	auto n = cross(a, b);
	if (equal(dot(n, dir), 0))
	{
		//����� ��������� ������ ����� ����� � ��������� �� ��� �������� �������
		return { false, 0 };
	}
	double t = (dot(A, n) - dot(p, n)) / dot(dir, n);
	Vector<3> projection = p + dir * t;

	int sa = sign(dot(cross(projection - A, B - A), n));
	return { sa == sign(dot(cross(projection - B, C - B), n)) && sa == sign(dot(cross(projection - C, A - C), n)) , t };
}

std::unique_ptr<Object> Box::copy() const
{
	return std::unique_ptr<Object>(new Box(*this));
}

std::unique_ptr<Object> Prizm::copy() const
{
	return std::unique_ptr<Object>(new Prizm(*this));
}
std::unique_ptr<Object> Cone::copy() const
{
	return std::unique_ptr<Object>(new Cone(*this));
}
std::unique_ptr<Object> Piramid::copy() const
{
	return std::unique_ptr<Object>(new Piramid(*this));
}
std::unique_ptr<Object> Cylinder::copy() const
{
	return std::unique_ptr<Object>(new Cylinder(*this));
}
std::unique_ptr<Object> Sphere::copy() const
{
	return std::unique_ptr<Object>(new Sphere(*this));
}
std::unique_ptr<Object> Polyhedron::copy() const
{
	return std::unique_ptr<Object>(new Polyhedron(*this));
}

std::vector<ISR> Piramid::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{
	if (!is_convex)
		assert(false);
	int count = 0;
	ISR t[2];
	if (!equal(dir.z(), 0))
	{
		double t_lev = (-height - start.z()) / dir.z();
		Vector<2> base_inter = start + dir * t_lev;
		if (isPointInsidePolygon(base_inter, base) != PointPolygonRelation::OUTSIDE)
			t[count++] = { t_lev, {0,0,-1}, false, this->id };
	}
	for (int i = 0; i < base.size(); ++i)
	{
		auto inter = intersectLineWithTriangle(start, dir, Vector<3>(base[i], -height), Vector<3>(base[i < base.size() - 1 ? i + 1 : 0], -height), { 0,0,0 });
		if (inter.first)
			t[count++] = { inter.second, normals[i], false, this->id };
		if (count == 2)
			break;
	}
	if (t[0].t < t[1].t)
	{
		t[0].in = true;
		return { t[0], t[1] };
	}
	t[1].in = true;
	return { t[1], t[0] };
}

void Piramid::countBoundingBox()
{
#ifdef BB_SPHERE
	double max = 0;
	for (auto& it : base)
		max = std::max(max, sqrt(dot(it, it) + height * height / 4));
	return { max,0,0 };
#endif
	Vector<2> base_bb_max = { 0,0 };
	Vector<2> base_bb_min = { 0,0 };
	for (auto& it : base)
	{
		base_bb_max = max(base_bb_max, it);
		base_bb_min = min(base_bb_min, it);
	}
	this->bounding_box = Vector<3>( (base_bb_max - base_bb_min) * 0.5, height/2 );
	this->bounding_box_position = this->position - Vector<3>(0, 0, height / 2);
}




std::pair<int, std::pair<ISR, ISR>> intersectLineWithCircle(const Vector<2>& p, const Vector<2> dir, double rad, int id)
{
	double A = (dir.x() * dir.x() + dir.y() * dir.y());
	double B_half = p.x() * dir.x() + p.y() * dir.y();
	double C = p.x() * p.x() + p.y() * p.y() - rad * rad;
	double D = B_half * B_half - A * C;
	if (D < 0)
		return { false, {{0,0,0,0, 0, id}, {0,0,0,0, 0, id} } };
	double D_sq = sqrt(D);
	
	//if (equal(D_sq, 0))
	//{
	//	double t = -B_half / A;
	//	Vector<2> point = p + t * dir;
	//	return { 1, {{t, {p,0}}, {t, {p,0}}} };
	//}
	double t1 = (-B_half - D_sq) / A;
	double t2 = (-B_half + D_sq) / A;
	Vector<2> p1 = (p + dir * t1) * (1.0/rad);
	Vector<2> p2 = (p + dir * t2) * (1.0/rad);
	return { 2, {{t1, {p1,0}, true, id}, {t2, {p2, 0}, false, id}} };
}



std::vector<ISR> Cylinder::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{
	if (equal(dir.x(), 0) && equal(dir.y(), 0))
	{
		auto in_res = dot<2>(start, start) < rad * rad;
		if (in_res)
		{
			double t_up = (half_height - start.z()) / dir.z();
			double t_down = (-half_height - start.z()) / dir.z();
			if (t_up < t_down)
				return { {t_up, {0,0,1}, true, this->id}, {t_down,{0,0,-1}, false, this->id} };
			return { {t_down,{0,0,-1}, true, this->id}, {t_up, {0,0,1}, false, this->id} };
		}
		return {};
	}
	auto shade_intersect = intersectLineWithCircle(start, dir, rad, id);

	if (shade_intersect.first == 0)
	{
		return {};
	}

	ISR t1 = shade_intersect.second.first;
	double point_z = start.z() + dir.z() * t1.t;
	/*if (shade_intersect.first == 1)
	{
		if (point_z >= -height / 2 && point_z <= height / 2)
		{
			has_intersect = true;
			return { t1, t1 };
		}
		has_intersect = false;
		return { {0,0,0,0},{0,0,0,0} };
	}*/
	ISR t2 = shade_intersect.second.second;
	double point_z2 = start.z() + dir.z() * t2.t;
	if (!(point_z >= -half_height && point_z <= half_height) && (point_z2 >= -half_height && point_z2 <= half_height))
	{
		std::swap(t1, t2);
		std::swap(point_z, point_z2);
		std::swap(shade_intersect.second.first, shade_intersect.second.second);
	}
	if (point_z >= -half_height && point_z <= half_height)
	{
		//����� ������������� ��� t1.t < t2.t � �������������� in � out ����������
		if (point_z2 >= -half_height && point_z2 <= half_height)
			return { t1, t2 };
		//z != 0 � � ��������� ������

		if (point_z2 > half_height)
		{
			double t_out = (half_height - start.z()) / dir.z();
			if (t1.t < t_out)
				return { t1, {t_out, {0,0,1}, false, this->id} };
			else
				return { {t_out, {0,0,1}, true, this->id}, t1 };

		}
		if (point_z2 < -half_height)
		{
			double t_out = (-half_height - start.z()) / dir.z();
			if (t1.t < t_out)
				return { t1, {t_out, {0,0,-1}, false, this->id} };
			else
				return { {t_out, {0,0,-1}, true, this->id}, t1 };

		}
	}
	//�������� �������� ������ �������� ����� ����� �������� ����� ��� ��������� ���� ������ �� ��������
	if (equal(dir.z(), 0))
	{
		return {};
	}
	double t_out = (half_height - start.z()) / dir.z();
	Vector<2> up_point = start + dir * t_out;
	if (dot(up_point, up_point) < rad * rad)
	{
		double t_bot = (-half_height - start.z()) / dir.z();
		if (t_out < t_bot)
			return { {t_out, {0, 0, 1}, true, this->id}, {t_bot, {0, 0, -1}, false, this->id} };
		else
			return {{t_bot, {0, 0, -1}, true, this->id},  {t_out, {0, 0, 1}, false, this->id} };
	}
	return {};
}

void Cylinder::countBoundingBox()
{
#ifdef BB_SPHERE
	return { sqrt(rad * rad + half_height * half_height),0,0 };
#endif
	this->bounding_box = { rad, rad, half_height };
	this->bounding_box_position = this->position;
}



IntersectionResult::IntersectionResult(double t, double x, double y, double z, bool in, int id) : t(t), n(x, y, z), in(in), obj_id(id)
{
	
}

IntersectionResult::IntersectionResult(double t, const Vector<3>& n, bool in, int id) : t(t),n(n), in(in), obj_id(id)
{
}

IntersectionResult::IntersectionResult() : t(0), n(0,0,0)
{
}

Box::Box(const Vector<3>& position, const Vector<3>& size, const Quat& rotation) : Object(position, rotation), size(size)
{
	reCountBoundingBox();
}

void Box::countBoundingBox()
{
#ifdef BB_SPHERE
	return { sqrt(dot(size, size)),0,0 };
#endif
	this->bounding_box = getHsize();
	this->bounding_box_position = position;
}

std::pair<bool, ISR> intersectLineWithBoxOnSide(int sig, int dir_c, int c1, int c2, const Vector<3>& bounding_box, const Vector<3>& start, const Vector<3>& dir, int id)
{
	double xpT = (sig*bounding_box.nums[dir_c] - start.nums[dir_c]) / dir.nums[dir_c];
	double yatx = start.nums[c1] + dir.nums[c1] * xpT;
	double zatx = start.nums[c2] + dir.nums[c2] * xpT;
	if (abs(yatx) <= bounding_box.nums[c1] && abs(zatx) <= bounding_box.nums[c2])
	{
		Vector<3> n;
		n.nums[dir_c] = sig;
		n.nums[c1] = 0;
		n.nums[c2] = 0;
		return { true, ISR(xpT, n, sig * sign(dir.nums[dir_c]) < 0, id) };
	}
	return { false, {0, 0,0,0,0, id} };

}
std::vector<ISR> Box::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{
	int c = 0;
	static std::vector<ISR> res(2);
	auto intres = intersectLineWithBoxOnSide(1, 0, 1, 2, size, start, dir, this->id);
	int previn = -1;
	if (intres.first)
	{
		res[1 - intres.second.in] = intres.second;
		previn = intres.second.in;
		++c;
	}
	intres = intersectLineWithBoxOnSide(-1, 0, 1, 2, size, start, dir, this->id);
	if (intres.first && intres.second.in != previn)
	{
		res[1 - intres.second.in] = intres.second;
		previn = intres.second.in;
		++c;
	}
	if (c == 2)
		return res;
	intres = intersectLineWithBoxOnSide(1, 1, 0, 2, size, start, dir, this->id);
	if (intres.first && intres.second.in != previn)
	{
		res[1 - intres.second.in] = intres.second;
		previn = intres.second.in;
		++c;
	}
	if (c == 2)
		return res;
	intres = intersectLineWithBoxOnSide(-1, 1, 0, 2, size, start, dir, this->id);
	if (intres.first && intres.second.in != previn)
	{
		res[1 - intres.second.in] = intres.second;
		previn = intres.second.in;
		++c;
	}
	if (c == 2)
		return res;
	intres = intersectLineWithBoxOnSide(1, 2, 1, 0, size, start, dir, this->id);
	if (intres.first && intres.second.in != previn)
	{
		res[1 - intres.second.in] = intres.second;
		previn = intres.second.in;
		++c;
	}
	if (c == 2)
		return res;
	intres = intersectLineWithBoxOnSide(-1, 2, 1, 0, size, start, dir, this->id);
	if (intres.first && intres.second.in != previn)
	{
		res[1 - intres.second.in] = intres.second;
		previn = intres.second.in;
		++c;
	}
	if (c == 2)
		return res;
	return {};

}

std::vector<ISR> Box::intersectWithRayOnBothSides(const Vector<3>& ray_start, const Vector<3>& direction) const
{
	Vector<3> start = transformation_mat * Vector<4>(ray_start);
	Vector<3> dir = rotation_mat * Vector<4>(direction);
	//� �� ������� ������ �� � ���� ��� ���������� ������� ��� ��� �����
	if (!lineIntersectsBoundingBox(start, dir))
		return { false, {0,0,0,0,0, this->id} };
	auto intersect = _intersectLine(start, dir);
	if (intersect.size() == 0)
		return { false, {0,0,0,0,0, this->id} };

	for (auto it = intersect.begin(); it != intersect.end(); )
	{
		if (it->t < 0)
		{
			it = intersect.erase(it);
			continue;
		}

		it->n = back_rotation_mat * Vector<4>(it->n);
		++it;
	}
	return intersect;
}

bool Box::isPointInside(const Vector<3>& p) const
{
	return abs(p.x()) < bounding_box.x() && abs(p.y()) < bounding_box.y() && abs(p.z()) < bounding_box.z();
}

Polyhedron::Polyhedron(const Vector<3>& position, const Quat& rotation, const std::vector<Vector<3>>& points, const std::vector<std::vector<int>>& edges) : Object(position, rotation), points(points), edges(edges)
{
	normals.resize(edges.size());
	polygons.resize(edges.size());
	polygs_coords.resize(edges.size());
	for (int i = 0; i < edges.size(); ++i)
	{
		const std::vector<int>& it = edges[i];
		Vector<3> n = cross(points[it[1]] - points[it[0]], points[it[2]] - points[it[0]]);
		//double mD = dot(points[it[0]], n);
		//double t = mD / dot(n, n);
		//if (mD < 0)
		//	n = n * -1;
		normals[i] = normalize(n);

		Vector<3> base_a = normalize(points[it[1]] - points[it[0]]);
		Vector<3> base_c = normals[i];
		Vector<3> base_b = cross(base_c, base_a);

		polygs_coords[i] = inverse(transpose(Matrix<3>{ base_a, base_b, base_c }));
		for (int j = 0; j < it.size(); ++j)
		{
			polygons[i].push_back(polygs_coords[i] * (points[it[j]] - points[it[0]]));
		}
	}
	if (this->convex)
	{
		//����� � ������� �������� ����� ���� �� ������������ � ��������� �� �� ���� ���� ��� ������ ����������
		for (int i = 0; i < normals.size(); ++i)
		{
			if (dot(normals[i], 0.3333333 * (points[edges[i][0]] + points[edges[i][1]] + points[edges[i][2]])) < 0)
				normals[i] = -1 * normals[i];
		}
	}
	reCountBoundingBox();
}

void Polyhedron::countBoundingBox()
{
#ifdef BB_SPHERE
	return { sqrt(dot(bb, bb)),0,0 };
#endif
	Vector<3> base_bb_max = { 0,0, 0 };
	Vector<3> base_bb_min = { 0,0, 0 };
	for (auto& it : this->points)
	{
		base_bb_max = max(base_bb_max, it);
		base_bb_min = min(base_bb_min, it);
	}
	this->bounding_box_position = position + (base_bb_max + base_bb_min) * 0.5;
	this->bounding_box = (base_bb_max - base_bb_min) * 0.5;

}
#include<algorithm>
static bool __comp(const ISR& a, const ISR& b)
{
	return a.t < b.t;
}
std::vector<ISR> Polyhedron::_intersectLine(const Vector<3>& start, const Vector<3>& dir) const
{
 	std::vector<ISR> res;
	int c = 0;
	for (int i = 0; i < edges.size(); ++i)
	{

		double t = (dot(points[edges[i][0]], normals[i]) - dot(start, normals[i])) / dot(dir, normals[i]);
		Vector<3> proj = start + dir * t;
		proj = proj - points[edges[i][0]];
		Vector<2> p_in_polygon = polygs_coords[i] * proj;
		//return { {1, {p_in_polygon, 0}, true} };
		if (isPointInsidePolygon(p_in_polygon, polygons[i]) != PointPolygonRelation::OUTSIDE)
		{
			if (this->convex)
			{
				if (c == 1 && equal(t, res[0].t))
					continue;
			}
			else
			{
				//���� ������� ������ � �������������� �� ����� �� ������ ����� ���� ������� ��������� ���. ��� ������� ������� ��������� �������� ��� ����������� ���� ������������ ��� ����� ���������. �� ��� ����� ������ ������ ������������������ �����
				//��� ��� ���� ��������� ����� ��������� ���.
				//���� �� ������� ��������� �� ��������������� �� ����� ������ ����� � �������� ����� ������ �� ��������
				bool already_counted = false;
				for (auto& it : res)
				{
					if (equal(it.t, t))
					{
						already_counted = true;
						break;
					}
				}
				if (already_counted)
					continue;
			}
			res.push_back( { t, normals[i], dot(normals[i], dir)<0, this->id });
			++c;
			if (this->convex && c == 2)
				break;
		}
	}
	if (res.size() == 2)
	{
		if (res[0].t > res[1].t)
			std::swap(res[0], res[1]);
		return res;
	}
	else if (res.size() > 2)
		std::sort(res.begin(), res.end(), __comp);
	return res;
}

bool Polyhedron::isPointInside(const Vector<3>& p) const
{
	assert(false);
	return false;
}


ObjectType Box::getType() const
{
	return ObjectType::BOX;
}
ObjectType Prizm::getType() const
{
	return ObjectType::PRIZM;
}
ObjectType Cone::getType() const
{
	return ObjectType::CONE;
}
ObjectType Piramid::getType() const
{
	return ObjectType::PIRAMID;
}
ObjectType Cylinder::getType() const
{
	return ObjectType::CYLINDER;
}
ObjectType Sphere::getType() const
{
	return ObjectType::SPHERE;
}
ObjectType Polyhedron::getType() const
{
	return ObjectType::POLYHEDRON;
}

double Sphere::getRadius() const
{
	return this->rad;
}

Quat Object::getRotation() const
{
	return this->rotation;
}

double Cylinder::getHalfHeight() const
{
	return this->half_height;
}

double Cylinder::getRadius() const
{
	return this->rad;
}

const std::vector<Vector<2>>& Prizm::getBase() const
{
	return this->base;
}

const std::vector<Vector<3>>& Prizm::getNormals() const
{
	return this->normals;
}

double Prizm::getHalfHeight() const
{
	return this->half_height;
}

double Cone::getHeight() const
{
	return this->height;
}
double Cone::getRadius() const
{
	return this->rad;
}
double Cone::getRdivh() const
{
	return this->rdivh;
}

const std::vector<Vector<2>>& Piramid::getBase() const
{
	return base;
}
const std::vector<Vector<3>>& Piramid::getNormals() const
{
	return normals;
}
double Piramid::getHeight() const
{
	return height;
}

Vector<3> Box::getHsize() const
{
	return this->size;
}


const std::vector<Vector<3>>& Polyhedron::getPoints() const
{
	return this->points;
}
const std::vector<Vector<3>>& Polyhedron::getNormals() const
{
	return this->normals;
}
const std::vector<std::vector<int>>& Polyhedron::getEdges() const
{
	return this->edges;
}
const std::vector<std::vector<Vector<2>>> Polyhedron::getPolygons() const
{
	return this->polygons;
}
const std::vector<Matrix<3>>& Polyhedron::getCoords() const
{
	return this->polygs_coords;
}

void Object::setBoundingBoxHSize(const Vector<3>& bb)
{
	this->bounding_box = bb;
}

void Object::setBoundingBoxPosition(const Vector<3>& bb)
{
	this->bounding_box_position = bb;
}

bool Object::haveBoundingBox() const
{
	return this->getType() != ObjectType::BOX;
}

Vector<3> Object::getBoundingBoxPosition() const
{
	return this->bounding_box_position;

}

#include "objects_fabric.h"
std::unique_ptr<Object> getBoundingBoxAsObject(const std::unique_ptr<Object>& obj)
{
	auto res = makeBox(obj->getBoundingBox(), obj->getBoundingBoxPosition(), { 1,0,0,0 });
	res->setColor({ 1, 0, 0 });
	res->setAlpha(0.3);
	return std::move(res);
}
bool doSegmentsIntersect(Vector<2> a, Vector<2> b, Vector<2> c, Vector<2> d)
{
	auto segm = rayIntersectsSegment(a, normalize(b - a), c, d);
	return segm.first && segm.second >= 0 && segm.second <= length(b - a);
}
int findEar(const std::vector<Vector<2>>& polygon)
{
	//�� ����� ��������� ���� ����� ������������ ����������� �������. � ��� ����� ����� ����������, ��� ���� � ����
	for (int i = 0; i < polygon.size(); ++i)
	{
		Vector<2> a = polygon[(i + 1) % polygon.size()] - polygon[i];
		Vector<2> b = polygon[(i + polygon.size() - 1) % polygon.size()] - polygon[i];
		if (dot(a, b) / length(a) / length(b) < -0.99)
			return i;
	}
	for (int i = 0; i < polygon.size(); ++i)
	{
		bool is_ear = true;
		Vector<2> a = polygon[(i + 1) % polygon.size()];
		Vector<2> b = polygon[(i + polygon.size() - 1) % polygon.size()];
		if (polygon.size() > 4)
		{
			for (int j = (i + 2) % polygon.size(); j != (i - 2 + polygon.size()) % polygon.size(); j = (j + 1) % polygon.size())
				if (doSegmentsIntersect(a, b, polygon[j], polygon[(j + 1) % polygon.size()]))
				{
					is_ear = false;
					break;
				}
			//
			auto ne2 = polygon[(i + 2) % polygon.size()];
			auto ne1 = polygon[(i + 1) % polygon.size()];
			auto pe2 = polygon[(i + polygon.size() - 2) % polygon.size()];
			auto pe1 = polygon[(i + polygon.size() - 1) % polygon.size()];
			if (doSegmentsIntersect(a, b, polygon[(i + 1) % polygon.size()] + (polygon[(i + 2) % polygon.size()] - polygon[(i + 1) % polygon.size()]) * 0.01, polygon[(i + 2) % polygon.size()]) || (dot(b - a, ne2 - ne1) / length(b - a) / length(ne2 - ne1) > 0.999999))
			//if (dot(b-a, ne2- ne1)/length(b-a)/length(ne2-ne1) > 0.99)
				is_ear = false;
			else if (doSegmentsIntersect(a, b, polygon[(i + polygon.size() - 1) % polygon.size()] + 0.01 * (polygon[(i + polygon.size() - 2) % polygon.size()] - polygon[(i + polygon.size() - 1) % polygon.size()]),polygon[(i + polygon.size() - 2) % polygon.size()]) || (dot(a - b, pe2 - pe1) / length(a - b) / length(pe2 - pe1) > 0.999999))
			//else if (dot(a - b, pe2 - pe1)/length(a-b)/length(pe2-pe1) > 0.99)
				is_ear = false;
		}
		
		if (!is_ear)
			continue;
		if (isPointInsidePolygon((a + b) / 2, polygon) != PointPolygonRelation::OUTSIDE )
			return i;
	}
	assert(false);
	return -1;
}
#include <array>

std::vector<std::array<int, 3>> triangulatePolygon(const std::vector<Vector<2>>& polygon)
{
	if (polygon.size() == 3)
		return { {0, 1, 2} };
	int ear = findEar(polygon);
	std::vector<std::array<int, 3>>  res = { std::array<int, 3>{int ((ear - 1 + polygon.size()) % polygon.size()), ear, int((ear + 1) % polygon.size())} };
	std::vector<Vector<2>> new_polygon;
	for (int i = 0; i < polygon.size(); ++i)
	{
		if (i == ear)
			continue;
		new_polygon.push_back(polygon[i]);
	}
	auto recres = triangulatePolygon(new_polygon);
	for (auto& it : recres)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (it[i] >= ear)
				it[i] += 1;
		}
		res.push_back(it);
	}
	return res;
}

bool isPolygonConvex(const std::vector<Vector<2>>& polygon)
{
	int dir = sign(cross(Vector<3>(polygon[0] - polygon.back(), 0), Vector<3>(polygon[1] - polygon[0], 0)).z());
	for (int i = 1; i < polygon.size() - 1; ++i)
	{
		int curdir = sign(cross(Vector<3>(polygon[i] - polygon[i - 1], 0), Vector<3>(polygon[i + 1] - polygon[i], 0)).z());
		if (curdir != dir)
			return false;
	}
	return true;
}

std::vector<int> tryToAddTriagToConvexPolygon(const std::vector<int>& polygon, const std::array<int, 3>& triag, const std::vector<Vector<2>>& base)
{
	auto contains = [triag](int p)->bool { return triag[0] == p || triag[1] == p || triag[2] == p; };
	for (int i = 0; i < polygon.size(); ++i)
	{
		if (contains(polygon[i]) && contains(polygon[(i + 1) % polygon.size()]))
		{
			int third = triag[0] + triag[1] + triag[2] - polygon[i] - polygon[(i + 1) % polygon.size()];
			std::vector<Vector<2>> combine_new;
			for (int j = 0; j <= i; ++j)
				combine_new.push_back(base[polygon[j]]);
			combine_new.push_back(base[third]);
			for (int j = i + 1; j < polygon.size(); ++j)
				combine_new.push_back(base[polygon[j]]);
			if (isPolygonConvex(combine_new))
			{
				std::vector<int> res;
				for (int j = 0; j <= i; ++j)
					res.push_back(polygon[j]);
				res.push_back(third);
				for (int j = i + 1; j < polygon.size(); ++j)
					res.push_back(polygon[j]);
				return res;
			}
			else
				return {};
		}
	}
	return {};
}
std::vector < std::vector<Vector<2>>> splitPolygonIntoConvexParts(const std::vector<Vector<2>>& polygon)
{
	if (isPolygonConvex(polygon))
		return { polygon };
	auto triangulation = triangulatePolygon(polygon);
	std::vector < std::vector<Vector<2>>> res;
	while (triangulation.size() > 0 )
	{
		std::vector<int> new_part = { triangulation[0][0], triangulation[0][1], triangulation[0][2] };
		triangulation.erase(triangulation.begin());
		bool added_trig = true;
		while (added_trig)
		{
			added_trig = false;
			for (auto tri_inter = triangulation.begin(); tri_inter != triangulation.end(); ++tri_inter)
			{
				auto addres = tryToAddTriagToConvexPolygon(new_part, *tri_inter, polygon);
				if (addres.size() > 0)
				{
					new_part = addres;
					added_trig = true;
					tri_inter = triangulation.erase(tri_inter);
					break;
				}
			}
		}
		std::vector<Vector<2>> new_res;
		for (auto& it : new_part)
			new_res.push_back(polygon[it]);
		res.push_back(new_res);
	}
	return res;
}