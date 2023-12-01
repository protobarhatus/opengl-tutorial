#version 430
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform ivec2 screen_size;


struct Quat
{
//a.x - a0, a - a.yzw
//к сожалению придется так из за padding
	vec4 a;
};
struct Primitive
{
	uint type;
	uint data_index;
	//сколько элементов в data_array она занимает. Кроме многогранника, для которого это количество вершин
	uint data_count;
	//для многогранника сначала пойдут точки а потом сразу нормали
	uint normals_index;
	vec3 position;
	//rdivh для конуса
	float sc3;
	Quat rotation;
	//для многогранника это количество граней
	uint normals_count;
	//половина высоты для призмы и цилиндра, высота для пирамиды и конуса, радиус для сферы, 
	float sc1;
	//радиус для цилиндра и конуса
	float sc2;
	//для многогранника. Остальная информация для него будет идти в int_data. [0] это индекс в матричный массив, затем таблица, задающая грани:на каждую строчку идет [количество элементов в массиве] и сам массив.
	//для многоугольников в собственных координатах это идет просто подряд без указания размера, поэтому итерироваться они обязаны одновременно с гранями.
	uint int_data_index;
};

const int OBJECTS_ADD = -1;
const int OBJECTS_MULT = -2;
const int OBJECTS_SUB = -3;

struct ComposedObjectNode
{
//положительный означает примитив. Отрицательный - операцию. 
	int operation;
};

layout(binding = 6) buffer composed_objects_array
{
	ComposedObjectNode composed_objects[];
};

#define ComposedObjectNode_isPrimitive(X) (X.operation >= 0)
#define ComposedObjectNode_getPrimitiveIndex(X) (X.operation)

#define ComposedObjectNode_left(X, N) (2*((N)+1) - 1)
#define ComposedObjectNode_right(X, N) (2*((N)+1))
#define ComposedObjectNode_parent(X, N) (((N)+1)/2 - 1)


uniform int primitives_count;
uniform int data_count;
uniform int normals_count;
uniform int composed_object_nodes_count;

layout(binding = 1) buffer primitives_array
{
	Primitive primitives[];
};
layout(binding = 2, std430) buffer data_array
{
	vec2 base_points[];
};
layout(binding = 3) buffer normals_array
{
	vec3 normals[];
};
layout(binding = 4) buffer int_array
{
	int int_data[];
};
layout(binding = 5) buffer matrix_array
{
	mat3 matrix_data[];
};


//Возможно стоит сделать эти вещи дефайнами
uint Polyhedron_vertexAmount(Primitive obj)
{
	return obj.data_count;
}

uint Polyhedron_edgesAmount(Primitive obj)
{
	return obj.normals_count;
}

uint Polyhedron_pointsStart(Primitive obj)
{
	return obj.normals_index;
}
uint Polyhedron_polygons2dStart(Primitive obj)
{
	return obj.data_index;
}
uint Polyhedron_matrixesStart(Primitive obj)
{
	return int_data[obj.int_data_index];
}
uint Polyhedron_edges2dStart(Primitive obj)
{
	return obj.int_data_index + 1;
}
uint Polyhedron_normalsStart(Primitive obj)
{
	return obj.data_index + Polyhedron_vertexAmount(obj);
}

struct Intersection
{
	vec3 n;
	float t;
};

struct IRO
{
	int inters;
	Intersection first, second;
};


const uint BOX = 0;
const uint PRIZM = 1;
const uint CONE = 2;
const uint PIRAMID = 3;
const uint CYLINDR = 4;
const uint SPHERE = 5;
const uint POLYHEDRON = 6;
const uint COMPOSED_OBJECT = 7;

//некоторая инкапсуляция для примитивов. sc1, sc2 - это еще ладно но обращение к буферам точно надо
uint Prizm_polygonStart(Primitive obj)
{
	return obj.data_index;
}

uint Prizm_polygonSize(Primitive obj)
{
	return obj.data_count;
}

uint Prizm_normalsStart(Primitive obj)
{
	return obj.normals_index;
}

uint Prizm_normalsSize(Primitive obj)
{
	return obj.normals_count;
}

uint Piramid_polygonStart(Primitive obj)
{
	return obj.data_index;
}

uint Piramid_polygonSize(Primitive obj)
{
	return obj.data_count;
}

uint Piramid_normalsStart(Primitive obj)
{
	return obj.normals_index;
}

uint Piramid_normalsSize(Primitive obj)
{
	return obj.normals_count;
}

Quat mult(in Quat a, in Quat b)
{
	return Quat(vec4(a.a.x * b.a.x - dot(a.a.yzw, b.a.yzw), a.a.x * b.a.yzw + b.a.x * a.a.yzw + cross(a.a.yzw, b.a.yzw)));
}

Quat inverseRot(in Quat q)
{
	return Quat(vec4(q.a.x, -1 * q.a.yzw));
}

mat4 rotation(in Quat q)
{
	float a0 = q.a.x;
	float a1 = q.a.y;
	float a2 = q.a.z;
	float a3 = q.a.w;

	return mat4(a1*a1 + a0*a0 - a2*a2 - a3*a3, 2*a2*a1 + 2*a0*a3, 2*a3*a1 - 2*a0*a2, 0, 2*a2*a1 - 2*a3*a0, a2*a2 + a0*a0 - a3*a3 - a1*a1, 2*a3*a2 + 2*a1*a0, 0, 2*a3*a1 + 2*a2*a0, 2*a3*a2 - 2*a1*a0, a3*a3 + a0*a0 - a2*a2 - a1*a1, 0, 0, 0, 0, 1);
}
void rotate(inout Quat q, float angle, vec3 n) {
	q = mult(q, Quat(vec4(cos(angle / 2), sin(angle / 2) * normalize(n))));
}






const IRO ZERO_IRO = IRO(0, Intersection( vec3(0,0,0), 0), Intersection( vec3(0,0,0),0));



IRO intersectSphereWithLine(in Primitive sphere, in vec3 start, in vec3 dir)
{

	float rad = sphere.sc1;
	float A = dot(dir, dir);
	float B_half = dot(start, dir);
	float C = dot(start, start) - rad * rad;

	float D_4 = B_half * B_half - A * C;
	if (D_4 < 0)
	{
		return ZERO_IRO;
	}
	float D_sq = sqrt(D_4);
	float t1 = (-B_half - D_sq) / A;
	float t2 = (-B_half + D_sq) / A;

	IRO res = {2, { (start + dir * t1)*(1.0/rad),t1}, { (start + dir * t2)*(1.0/rad),t2}};
	return res;
}

bool equal(float a, float b)
{
	return abs(a - b) < 1e-5;
}
IRO intersectLineWithCircle(in vec2 p, in vec2 dir, float rad)
{
	float A = (dir.x * dir.x + dir.y * dir.y);
	float B_half = p.x * dir.x + p.y * dir.y;
	float C = p.x * p.x + p.y * p.y - rad * rad;
	float D = B_half * B_half - A * C;
	if (D < 0)
		return ZERO_IRO;
	float D_sq = sqrt(D);
	//не уверен что это нужно и полезно
	if (equal(D_sq, 0))
	{
		float t = -B_half / A;
		vec2 point = p + t * dir;
		IRO res = {1, {vec3(p.xy, 0),t }, { vec3(p.xy, 0),t}};
		return res;
	}
	float t1 = (-B_half + D_sq) / A;
	float t2 = (-B_half - D_sq) / A;
	vec2 p1 = (p + dir * t1) * (1.0/rad);
	vec2 p2 = (p + dir * t2) * (1.0/rad);
	IRO res = { 2, { vec3(p1,0),t1}, { vec3(p2, 0),t2} };
	return res;
}

void swap(inout Intersection a, inout Intersection b)
{
	Intersection c = a;
	a = b;
	b = c;
}

void swap(inout float a, inout float b)
{
	float c = a;
	a = b;
	b = c;
}

IRO intersectCylinderWithLine(in Primitive obj, in vec3 start, in vec3 dir)
{
	float rad = obj.sc2;
	float half_height = obj.sc1;
	if (equal(dir.x, 0.0) && equal(dir.y, 0.0))
	{
		bool in_res = dot(start.xy, start.xy) < rad * rad;
		if (in_res)
		{
			IRO res = {2, { {0,0,1},(half_height - start.z) / dir.z}, {{0,0,-1},(-half_height - start.z) / dir.z}};
			return res;
		}
		return ZERO_IRO;
	}
	IRO shade_intersect = intersectLineWithCircle(start.xy, dir.xy, rad);

	if (shade_intersect.inters == 0)
	{
		return shade_intersect;
	}

	Intersection t1 = shade_intersect.first;
	float point_z = start.z + dir.z * t1.t;
	if (shade_intersect.inters == 1)
	{
		if (point_z >= -half_height && point_z <= half_height)
		{
			return shade_intersect;
		}
		return ZERO_IRO;
	}
	Intersection t2 = shade_intersect.second;
	float point_z2 = start.z + dir.z * t2.t;
	if (!(point_z >= -half_height && point_z <= half_height) && (point_z2 >= -half_height && point_z2 <= half_height))
	{
		swap(t1, t2);
		swap(point_z, point_z2);
		swap(shade_intersect.first, shade_intersect.second);
	}
	if (point_z >= -half_height && point_z <= half_height)
	{
		if (point_z2 >= -half_height && point_z2 <= half_height)
			return shade_intersect;
		//z != 0 т к произошел подьем

		if (point_z2 > half_height)
		{
			float t_out = (half_height - start.z) / dir.z;
			IRO res = {2, t1, { {0,0,1},t_out}};
			return res;
		}
		if (point_z2 < -half_height)
		{
			float t_out = (-half_height - start.z) / dir.z;
			IRO res = {2, t1, { {0, 0, -1},t_out}};
			return res;

		}
	}
	//осталось возможна только ситуация когда линия проходит через оба основания либо вообще не проходит
	if (equal(dir.z, 0))
	{
		return ZERO_IRO;
	}
	float t_out = (half_height - start.z) / dir.z;
	vec2 up_point = (start + dir * t_out).xy;
	if (dot(up_point, up_point) < rad * rad)
	{
		IRO res = {2, { {0,0,1},t_out}, { {0,0,-1},(-half_height - start.z) / dir.z}};
		return res;
	}
	
	return IRO(0, Intersection( vec3(0,0,0),0), Intersection( vec3(0,0,0),0));
}

uint POINT_ON_BOUNDARY = 0;
uint POINT_IN_POLYGON = 1;
uint POINT_OUT_OF_POLYGON = 2;

//специфичное сравнение для проверки на принадлежность многоугольнику
int compareDouble(double a, double b, double eps)
{
	if (abs(a - b) < eps)
		return 0;
	else if (a < b)
		return -1;
	else
		return 1;
}

int sideOfPointRelativeToLine(vec2 point, vec2 low, vec2 high)
{
	//considering low.y < high.y
	vec2 v1 = high - low;
	vec2 v2 = point - low;
	float z_orient = v1.x * v2.y - v1.y * v2.x;

	return compareDouble(0, z_orient, 1e-4);
}

uint isPointInsidePolygon(vec2 p, uint pol_start, uint pol_size)
{

	int current = 0;
	if (pol_size < 2)
		return POINT_OUT_OF_POLYGON;

	bool IsInside = false;
	int cur_y_comp_res = compareDouble(base_points[pol_start + current].y, p.y, 1e-4);
	int next = 1;
	do 
	{
		
		int next_y_comp_res = compareDouble(base_points[pol_start + next].y, p.y, 1e-4);
		switch (cur_y_comp_res) {
		case -1:
			switch (next_y_comp_res) {
			case -1:
				break;
			case 0:
				switch (compareDouble(p.x, base_points[pol_start + next].x, 1e-4)) {
				case -1: IsInside = !IsInside; break;
				case 0:   return POINT_ON_BOUNDARY;
				case 1:  break;
				}
				break;
			case 1:
				switch (sideOfPointRelativeToLine(p, base_points[pol_start + current], base_points[pol_start + next])) {
				case -1: IsInside = !IsInside; break;
				case  0: return POINT_ON_BOUNDARY;
				}
				break;
			}
			break;
		case 0:
			switch (next_y_comp_res) {
			case -1:
				switch (compareDouble(p.x, base_points[pol_start + current].x, 1e-4)) {
				case -1: IsInside = !IsInside; break;
				case 0:   return POINT_ON_BOUNDARY;
				case 1:  break;
				}
				break;
			case 0:
				switch (compareDouble(p.x, base_points[pol_start + current].x, 1e-4)) {
				case -1:
					if (compareDouble(p.x, base_points[pol_start + next].x, 1e-4) != -1)
						return POINT_ON_BOUNDARY;
					break;
				case 0: return POINT_ON_BOUNDARY;
				case 1:
					if (compareDouble(p.x, base_points[pol_start + next].x, 1e-4) != 1)
						return POINT_ON_BOUNDARY;
					break;
				}
				break;
			case 1:
				if (compareDouble(p.x, base_points[pol_start + current].x, 1e-4) == 0) {
					return POINT_ON_BOUNDARY;
				}
				break;
			}
			break;
		case 1:
			switch (next_y_comp_res) {
			case -1:
				switch (sideOfPointRelativeToLine(p, base_points[pol_start + next], base_points[pol_start + current])) {
				case -1: IsInside = !IsInside; break;
				case  0: return POINT_ON_BOUNDARY;
				}
				break;
			case 0:
				if (compareDouble(p.x, base_points[pol_start + next].x, 1e-4) == 0) {
					return POINT_ON_BOUNDARY;
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
		if (next == pol_size)
			next = 0;
	} while (current != 0);

	return IsInside ? POINT_IN_POLYGON : POINT_OUT_OF_POLYGON;
}



float getParam(vec2 st, vec2 dir, vec2 p)
{
	if (equal(dir.x, 0))
	{
		return (p.y - st.y) / dir.y;
	}
	return (p.x - st.x) / dir.x;
}

struct BF_pair
{
	bool b;
	float f;
};

BF_pair rayIntersectsSegment(vec2 p, vec2 dir, vec2 a, vec2 b)
{
	float div = dir.x * (b.y - a.y) - dir.y * (b.x - a.x);
	float num = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
	
	if (equal(div, 0))
	{
		if (equal(num, 0))
		{
			if (dot(b - p, a - p) < 0)
			{
				BF_pair res =  { true, 0 };
				return res;
			}
			else if (dot(dir, a - p) < 0)
			{
				BF_pair res =  { false, 0 };
				return res;
			}
			else if (dot(a - p, a - p) < dot(b - p, b - p))
			{
				BF_pair res =  { true, getParam(p, dir, a) };
				return res;
			}
			else
			{
				BF_pair res =  { true,getParam(p, dir, b) };
				return res;
			}

		}
		else
			{
				BF_pair res =  { false, 0 };
				return res;
			}
	}
	else
	{
		float t = num / div;
		//t /= 10;
		vec2 point = p + dir * t;
		//это место все еще может давать артефакты, в зависимости от числа в сравнении. 1e-6 работает вроде хорошо. Возможно стоит сделать проверку если point в окрестности a или b но это будет занимать время
		if (dot(b - point, a - point) <= 1e-6)
		{
			BF_pair res = {true, t};
			return res;
		}
		BF_pair res =  { false, 0 };
		return res;
	}

}


IRO WHITE_IRO = {2, { {1, 1, 1},1}, { {0,-1,0},2}};
IRO RED_IRO = {2, { {1,0,0},1}, { {0,0,0},2}};
IRO GREEN_IRO = {2, { {0,1,0},1}, { {0,0,0},2}};
IRO BLUE_IRO = {2, { {0,0,1},1}, { {0,0,0},2}};
IRO YELLOW_IRO = {2, { {1,1,0},1}, { {0,0,0},2}};
IRO MAGENTA_IRO = {2, { {1,0,1},1}, { {0,0,0},2}};
IRO LIGHT_IRO = {2, { {0,1,1},1}, { {0,0,0},2}};
IRO _debugIRO(vec3 n)
{
	IRO Deb_r = {2, { n,1}, { {0,0,0},2}};
	return Deb_r;
}

Intersection WHITE_INTER = { {1, 1, 1},1};
Intersection RED_INTER =  { {1,0,0},1};
Intersection GREEN_INTER =  { {0,1,0},1};
Intersection BLUE_INTER =  { {0,0,1},1};
Intersection YELLOW_INTER =  {{1,1,0},1};
Intersection MAGENTA_INTER = {{1,0,1},1};
Intersection LIGHT_INTER = {{0,1,1},1};
Intersection _debugINTER(vec3 n)
{
	Intersection Deb_r = { n,1};
	return Deb_r;
}

IRO rayIntersectsPolygon(vec2 p, in vec2 n, uint polygon_start, uint polygon_size, uint normals_start, uint normals_size)
{
	int c = 0;
	Intersection res[2];
	
	for (int i = 0; i < polygon_size; ++i)
	{
		BF_pair int_res = rayIntersectsSegment(p, n, base_points[polygon_start + i],base_points[polygon_start + (i < polygon_size - 1 ? i + 1 : 0)]);
		
		if (!int_res.b)
			continue;
		vec2 point = p + n * int_res.f;

		if (c == 1 && equal(int_res.f, res[0].t))
			continue;
		res[c].t = int_res.f;
		res[c].n = normals[normals_start + i];
		c++;
		if (c == 2)
			break;
	}

	//if (res[0].t < res[1].t)
	//	res[0].in = true;
	//else
	//	res[1].in = true;
	IRO irores = { c, res[0], res[1] };
	return irores;

}

float indic(float x)
{
	if (x < 0)
		return 0;
	return 1;
}
//только выпуклые
IRO intersectPrizmWithLine(Primitive obj, in vec3 start, in vec3 dir)
{
	float EPSILON = 1e-6;
	float half_height = obj.sc1;

	if (equal(dir.x, 0) && equal(dir.y, 0))
	{
		uint in_res = isPointInsidePolygon(start.xy, Prizm_polygonStart(obj), Prizm_polygonSize(obj));
		if (in_res != POINT_OUT_OF_POLYGON)
		{
			float t_up = (half_height - start.z) / dir.z;
			float t_down = (-half_height - start.z) / dir.z;
			if (t_up < t_down)
			{
				IRO res = {2,  { {0,0,1},t_up}, { {0,0,-1},t_down }};
				return res;
			}
			else
			{
				IRO res = {2,  { {0,0,-1},t_down} , { {0,0,1},t_up} };
				return res;
			}
		}
		return ZERO_IRO;
	}
	IRO shade_intersect = rayIntersectsPolygon(start.xy, dir.xy, Prizm_polygonStart(obj), Prizm_polygonSize(obj), Prizm_normalsStart(obj), Prizm_normalsSize(obj));

	if (shade_intersect.inters == 0)
	{
		return ZERO_IRO;
	}

	Intersection t1 = shade_intersect.first;
	float point_z = start.z + dir.z * t1.t;
	

	if (shade_intersect.inters == 1)
	{
		//если токо одно пересечение то пока не рисуем
		return ZERO_IRO;
		if (point_z >= -half_height - EPSILON && point_z <= half_height + EPSILON)
		{
			//если все же вернусь сюда то надо не забыть проставить нормально in и out
			IRO res = { 1, t1, t1 };
			return res;
		}
		return ZERO_IRO;
	}
	Intersection t2 = shade_intersect.second;
	float point_z2 = start.z + dir.z * t2.t;
	
	if (!(point_z >= -half_height - EPSILON && point_z <= half_height + EPSILON) && (point_z2 >= -half_height - EPSILON && point_z2 <= half_height + EPSILON))
	{
		swap(t1, t2);
		swap(point_z, point_z2);
		swap(shade_intersect.first, shade_intersect.second);
	}

	if (point_z >= -half_height - EPSILON && point_z <= half_height + EPSILON)
	{
		if (point_z2 >= -half_height - EPSILON && point_z2 <= half_height + EPSILON)
		{
			if (t1.t < t2.t)
			{
				IRO res = {2, t1, t2 };
				return res;
			}
			else
			{
				IRO res = {2, t2, t1};
				return res;
			}
		}
		
		//z != 0 т к произошел подьем
		if (point_z2 > half_height + EPSILON)
		{
			float t_out = (half_height - start.z) / dir.z;
			if (t1.t < t_out)
			{
				IRO res = {2,  t1, { {0, 0, 1},t_out } };
				return res;
			}
			else
			{
				IRO res = {2,  { {0, 0, 1},t_out }, t1 };
				return res;
			}
			//невозможная ситуация, т к если он не выходит в стене то должен выходить наверху но мало ли
			IRO res = {2, t1, t1 };
			return res;
		}
		if (point_z2 < -half_height - EPSILON)
		{
			
			float t_out = (-half_height - start.z) / dir.z;
			if (t1.t < t_out)
			{
				IRO res = {2, t1, { {0, 0, -1},t_out } };
				return res;
				
			}
			else
			{
				IRO res = {2, { {0, 0, -1},t_out }, t1 };
				return res;
			}
			IRO res = {2,  t1, t1 };
			
			return res;
			
		}
		
	}
	
	//осталось возможна только ситуация когда линия проходит через оба основания, через верхнее/нижнее ребро, либо вообще не проходит
	if (equal(dir.z, 0))
	{
		return ZERO_IRO;
	}
	if (point_z < -half_height && point_z2 < -half_height)
		return ZERO_IRO;
	if (point_z > half_height && point_z2 > half_height)
		return ZERO_IRO;
	float t_out = (half_height - start.z) / dir.z;
	vec2 up_point = start.xy + dir.xy * t_out;
	
	//if (isPointInsidePolygon(up_point, Prizm_polygonStart(obj), Prizm_polygonSize(obj)) != POINT_OUT_OF_POLYGON)
	{
		float t_bot = (-half_height - start.z) / dir.z;
		if (t_out < t_bot)
		{
			IRO res = {2, { {0,0,1},t_out},  { {0,0,-1},t_bot} };
			return res;
		}
		else
		{
			IRO res = {2, { {0,0,-1},t_bot},  { {0,0,1},t_out} };
			return res;
		}
	}

	return ZERO_IRO;
}


IRO intersectConeWithLine(in Primitive object, in vec3 start, in vec3 dir)
{
	float height = object.sc1;
	float rad = object.sc2;
	float rdivh = object.sc3;

	float M_s = rdivh * rdivh;
	float A = dir.x * dir.x + dir.y * dir.y - M_s * dir.z * dir.z;
	float B_half = dir.x * start.x + dir.y * start.y - M_s * dir.z * start.z;
	float C = start.x * start.x + start.y * start.y - M_s * start.z * start.z;
	float D_4 = B_half * B_half - A * C;
	if (D_4 < 0)
		return ZERO_IRO;
	float D_sq_2 = sqrt(D_4);
	float t1 = (-B_half - D_sq_2) / A;
	float t2 = (-B_half + D_sq_2) / A;

	float z1 = start.z + dir.z * t1;
	float z2 = start.z + dir.z * t2;

	bool z1_good = z1 >= -height && z1 <= 0;
	bool z2_good = z2 >= -height && z2 <= 0;
	if (z1_good && z2_good)
	{
		vec2 p1 = start.xy + dir.xy * t1;
		vec2 p2 = start.xy + dir.xy * t2;

		float zn1 = -dot(p1, p1) / z1;
		float zn2 = -dot(p2, p2) / z2;

		IRO res = {2, { normalize(vec3(p1, zn1)),t1}, { normalize(vec3(p2, zn2)),t2}};
		return res;
	}
	if (!z1_good && !z2_good)
		return ZERO_IRO;
	if (z2_good)
	{
		swap(t1, t2);
		swap(z1, z2);
	}
	//если линия пересекает усеченный конус в одном месте и не выходит в другом, то она гарантированно пройдет через основание
	vec2 p1 = start.xy + dir.xy * t1;
	float zn1 = -dot(p1, p1) / z1;

	float t_base = (-height - start.z) / dir.z;
	if (t_base < t1)
	{
		IRO res = {2, { {0,0,-1},t_base}, { normalize(vec3(p1, zn1)),t1}};
		return res;
	}
	IRO res = {2, { normalize(vec3(p1, zn1)),t1}, { {0,0,-1},t_base}};
	return res;
}


BF_pair intersectLineWithTriangle(vec3 p, vec3 dir, vec3 A, vec3 B, vec3 C)
{
	vec3 a = C - A, b = B - A;
	vec3 n = cross(a, b);
	if (equal(dot(n, dir), 0))
	{
		BF_pair r = {false, 0};
		return r;
	}
	float t = (dot(A, n) - dot(p, n)) / dot(dir, n);
	vec3 projection = p + dir * t;

	float sa = sign(dot(cross(projection - A, B - A), n));
	BF_pair res = { equal(sa, sign(dot(cross(projection - B, C - B), n))) && equal(sa, sign(dot(cross(projection - C, A - C), n))) , t };
	return res;
}


IRO intersectPiramidWithLine(in Primitive obj, in vec3 start, in vec3 dir)
{
	float height = obj.sc1;

	int count = 0;
	Intersection t[2];
	if (!equal(dir.z, 0))
	{
		float t_lev = (-height - start.z) / dir.z;
		vec2 base_inter = start.xy + dir.xy * t_lev;
		if (isPointInsidePolygon(base_inter, Piramid_polygonStart(obj), Piramid_polygonSize(obj)) != POINT_OUT_OF_POLYGON)
		{
			t[count].t = t_lev;
			t[count].n = vec3(0,0,-1);
			count++;
		}
	}
	uint base_start = Piramid_polygonStart(obj);
	uint base_size = Piramid_polygonSize(obj);
	uint normals_start = Piramid_normalsStart(obj);
	uint normals_size = Piramid_normalsSize(obj);
	for (int i = 0; i < base_size; ++i)
	{
		BF_pair inter = intersectLineWithTriangle(start, dir, vec3(base_points[base_start + i].xy, -height), vec3(base_points[base_start + (i < base_size - 1 ? i + 1 : 0)], -height), vec3( 0,0,0 ));
		if (inter.b)
		{
			t[count].t = inter.f;
			t[count].n = normals[normals_start + i];
			count++;
		}
		if (count == 2)
			break;
	}
	if (t[0].t < t[1].t)
	{
		IRO res = {count, t[0], t[1]};
		return res;
	}
	IRO res = {count, t[1], t[0]};
	return res;
}




struct BoolISRBool
{
	bool have_intersect;
	Intersection inter;
	int isin;
};
BoolISRBool intersectLineWithBoxOnSide(int sig, int dir_c, int c1, int c2, vec3 bounding_box, vec3 start, vec3 dir)
{
	float xpT = (sig*bounding_box[dir_c] - start[dir_c]) / dir[dir_c];
	float yatx = start[c1] + dir[c1] * xpT;
	float zatx = start[c2] + dir[c2] * xpT;
	if (abs(yatx) <= bounding_box[c1] && abs(zatx) <= bounding_box[c2])
	{
		vec3 n;
		n[dir_c] = sig;
		n[c1] = 0;
		n[c2] = 0;

		BoolISRBool res = { true,  { n,xpT}, int((sig * sign(dir[dir_c])) < 0) };
		return res;
	}
	BoolISRBool res = {false, {{0,0,0},0}, 0};
	return res;

}

IRO intersectBoxWithLine(in Primitive obj, in vec3 start, in vec3 dir)
{
	int c = 0;
	Intersection res[2];
	vec3 size = vec3(obj.sc1, obj.sc2, obj.sc3);
	BoolISRBool intres = intersectLineWithBoxOnSide(1, 0, 1, 2, size, start, dir);
	int previn = -1;
	if (intres.have_intersect)
	{
		res[1 - intres.isin] = intres.inter;
		previn = intres.isin;
		++c;
	}
	intres = intersectLineWithBoxOnSide(-1, 0, 1, 2, size, start, dir);
	if (intres.have_intersect && intres.isin != previn)
	{
		res[1 - intres.isin] = intres.inter;
		previn = intres.isin;
		++c;
	}
	if (c == 2)
	{
		IRO res = {2, res[0], res[1]};
		return res;
	}
	intres = intersectLineWithBoxOnSide(1, 1, 0, 2, size, start, dir);
	if (intres.have_intersect && intres.isin != previn)
	{
		res[1 - intres.isin] = intres.inter;
		previn = intres.isin;
		++c;
	}
	if (c == 2)
	{
		IRO res = {2, res[0], res[1]};
		return res;
	}
	intres = intersectLineWithBoxOnSide(-1, 1, 0, 2, size, start, dir);
	if (intres.have_intersect && intres.isin != previn)
	{
		res[1 - intres.isin] = intres.inter;
		previn = intres.isin;
		++c;
	}
	if (c == 2)
	{
		IRO res = {2, res[0], res[1]};
		return res;
	}
	intres = intersectLineWithBoxOnSide(1, 2, 1, 0, size, start, dir);
	if (intres.have_intersect && intres.isin != previn)
	{
		res[1 - intres.isin] = intres.inter;
		previn = intres.isin;
		++c;
	}
	if (c == 2)
	{
		IRO res = {2, res[0], res[1]};
		return res;
	}
	intres = intersectLineWithBoxOnSide(-1, 2, 1, 0, size, start, dir);
	if (intres.have_intersect && intres.isin != previn)
	{
		res[1 - intres.isin] = intres.inter;
		previn = intres.isin;
		++c;
	}
	if (c == 2)
	{
		IRO res = {2, res[0], res[1]};
		return res;
	}
	IRO ires = {0, { {0,0,0},0}, { {0,0,0},0}};
	return ires;
}



IRO intersectPolyhedronWithLine(Primitive obj, vec3 start,vec3 dir)
{
	
	Intersection res[2];
	int c = 0;
	uint int_data_iterator_sub1 = Polyhedron_edges2dStart(obj);
	uint polygons_iterator = Polyhedron_polygons2dStart(obj);
	uint normals_start = Polyhedron_normalsStart(obj);
	uint points_start = Polyhedron_pointsStart(obj);

	
	for (int i = 0; i < Polyhedron_edgesAmount(obj); ++i)
	{
		float t = (dot(normals[points_start + int_data[int_data_iterator_sub1 + 1]], normals[normals_start + i]) - dot(start, normals[normals_start + i])) / dot(dir, normals[normals_start + i]);
		vec3 proj = start + dir * t;
		proj = proj - normals[points_start + int_data[int_data_iterator_sub1 + 1]];
		vec2 p_in_polygon = (matrix_data[Polyhedron_matrixesStart(obj) + i] * proj).xy;

		if (isPointInsidePolygon(p_in_polygon, polygons_iterator, int_data[int_data_iterator_sub1]) != POINT_OUT_OF_POLYGON)
		{
			if (c == 1 && equal(t, res[0].t))
				continue;
			//СЧИТАЕМ ВЫПУКЛЫМ
			/*else
			{
				//если границы входят в многоугольники то точка на ребрах может быть учитана несколько раз. Это немного сломает алгоритмы операций над множествами хотя модификацией это можно устранить. Но где будет больше потеря производительности здесь
				//или там если учитывать точки несколько раз.
				//если же границы исключать из многоугольников то будут черные линии с которыми особо ничего не сделаешь
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
			}*/
			Intersection to_res = { normals[normals_start + i],t};
			res[c] = to_res;
			++c;
			if (c == 2)
				break;
			
		}
		polygons_iterator += int_data[int_data_iterator_sub1];
		int_data_iterator_sub1 += int_data[int_data_iterator_sub1] + 1;
	}
	if (c == 2)
	{
		if (res[0].t > res[1].t)
			swap(res[0], res[1]);
		IRO irores = {2, res[0], res[1]};
		return irores;
	}
	
	return ZERO_IRO;
}


IRO intersectWithRay(in Primitive object, in vec3 ray_start, in vec3 direction)
{
	//Matrix<4> transposition(Vector<4>(1, 0, 0, -position.x()), Vector<4>(0, 1, 0, -position.y()), Vector<4>(0, 0, 1, -position.z()), Vector<4>(0, 0, 0, 1));
	mat4 transposition = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -object.position.x, -object.position.y, -object.position.z, 1);

	mat4 rot = rotation(inverseRot(object.rotation));
	mat4 transformation = rot * transposition;

	vec3 start = (transformation * vec4(ray_start, 1)).xyz;
	vec3 dir = (rot * vec4(direction, 1)).xyz;

	IRO inter_res;

	switch(object.type)
	{
		case PRIZM:
			inter_res = intersectPrizmWithLine(object, start, dir);
			break;
		case PIRAMID:
			inter_res = intersectPiramidWithLine(object, start, dir);
			break;
		case CYLINDR:
			inter_res = intersectCylinderWithLine(object, start, dir);
			break;
		case SPHERE:
			inter_res = intersectSphereWithLine(object, start, dir);
			break;
		case CONE:
			inter_res = intersectConeWithLine(object, start, dir);
			break;
		case BOX:
			inter_res = intersectBoxWithLine(object, start, dir);
			break;
		case POLYHEDRON:
			inter_res = intersectPolyhedronWithLine(object, start, dir);
			break;
	}
	if (inter_res.inters == 0)
		return inter_res;
	if (inter_res.first.t < 0 && inter_res.second.t < 0)
	{
		IRO res = {0, {{0,0,0},0 }, { {0,0,0},0}};
		return res;
	}
	
	
	if (inter_res.second.t < inter_res.first.t && inter_res.second.t >= 0)
		swap(inter_res.first, inter_res.second);
	
	if (inter_res.second.t < 0)
	{
		inter_res.inters = 1;
	}
	
	//.first может быть < 0
	mat4 back_rotation = rotation(object.rotation);
	inter_res.first.n = (back_rotation * vec4(inter_res.first.n, 1)).xyz;
	inter_res.second.n = (back_rotation * vec4(inter_res.second.n, 1)).xyz;
	return inter_res;
}

float sq(float x)
{
	return x*x;
}


//точки которые вылазят за стек будут отброшены.
#define STACK_SIZE 200

struct IntersectionListUnit
{
	Intersection data;
	int next_index;
	bool is_in;
};
IntersectionListUnit intersections_stack[STACK_SIZE];
//поскольку удаление не предусмотренно, мы просто идем вправо. То есть есть ограничение по в целом количеству пересечений на обьекте.
int stack_index = 0;
//листы по инлексу должны совпадать с нодами, которых они представляют. Это по сути просто отображение из нодов в начало листа с их данными
int lists_stack[STACK_SIZE];

void intersectWithPrimitiveAsNode(int node, vec3 camera_pos, vec3 dir)
{
	IRO inter_res = intersectWithRay(primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[node])], camera_pos, dir);
	switch(inter_res.inters)
	{
		case 0:
			lists_stack[node] = -1;
			return;
		case 1:
			//это просто касание, и не стоит это рассматривать. Потому что тогда is_in всеравно кривоватое.
			lists_stack[node] = -1;
			return;
			/*if (inter_res.first.t >= 0)
			{
				lists_stack[node] = stack_index;
				intersections_stack[stack_index++] = IntersectionListUnit(inter_res.first, -1);
				return;
			}
			lists_stack[node] = -1;
			return;*/
		case 2:
			if (inter_res.first.t >= 0) //если .first >= 0 то и .second >= 0 точно
			{
				lists_stack[node] = stack_index;
				intersections_stack[stack_index] = IntersectionListUnit(inter_res.first, stack_index + 1, true);
				stack_index++;
				intersections_stack[stack_index++] = IntersectionListUnit(inter_res.second, -1, false);
				return;
			}
			//.second точно >= 0, иначе бы вернулось ZERO_IRO
			lists_stack[node] = stack_index;
			intersections_stack[stack_index++] = IntersectionListUnit(inter_res.second, -1, false); //вообще конечно вопрос как это повлияет на алгоритмы т-м операций но вроде бы должно быть нормально.
			return;
	}
}

void pushBackToList(int object, inout int last_pushed, int to_push)
{
	if (last_pushed == -1)
		lists_stack[object] = to_push;
	else
		intersections_stack[last_pushed].next_index = to_push;
	last_pushed = to_push;
}

//при этом листы, относящиеся к дочерним нодам разрушаются.
void uniteObjects(int current, int left, int right)
{
	int left_it = lists_stack[left];
	int right_it = lists_stack[right];

	bool in_a = false;
	bool in_b = false;

	int last_pushed = -1;

	while (left_it != -1 && right_it != -1)
	{
		if (intersections_stack[left_it].data.t < intersections_stack[right_it].data.t)
		{
			if (!in_b)
				pushBackToList(current, last_pushed, left_it);
			in_a = intersections_stack[left_it].is_in;
			left_it = intersections_stack[left_it].next_index;
		}
		else
		{
			if (!in_a)
				pushBackToList(current, last_pushed, right_it);
			in_b = intersections_stack[right_it].is_in;
			right_it = intersections_stack[right_it].next_index;
		}
	}
	//сейчас одно из них дошло точно до конца поэтому никаких ифов можно не делать
	while (right_it != -1)
	{
		pushBackToList(current, last_pushed, right_it);
		right_it = intersections_stack[right_it].next_index;
	}
	while (left_it != -1)
	{
		pushBackToList(current, last_pushed, left_it);
		left_it = intersections_stack[left_it].next_index;
	}
	//это по сути закрытие листа, иначе финальный нод будет ссылаться на рандомный другой
	pushBackToList(current, last_pushed, -1);
}

void intersectObjects(int current, int left, int right)
{
	bool in_a = false;
	bool in_b = false;
	
	int last_pushed = -1;
	int left_it = lists_stack[left];
	int right_it = lists_stack[right];

	while (left_it != -1 && right_it != -1)
	{
		if (intersections_stack[left_it].data.t < intersections_stack[right_it].data.t)
		{
			if (in_b)
				pushBackToList(current, last_pushed, left_it);
			in_a = intersections_stack[left_it].is_in;
			left_it = intersections_stack[left_it].next_index;
		}
		else
		{
			if (in_a)
				pushBackToList(current, last_pushed, right_it);
			in_b = intersections_stack[right_it].is_in;
			right_it = intersections_stack[right_it].next_index;
		}
	}
	pushBackToList(current, last_pushed, -1);
}

void subtractObjects(int current, int left, int right)
{
	bool in_a = false;
	bool in_b = false;

	int left_it = lists_stack[left];
	int right_it = lists_stack[right];

	int last_pushed = -1;
	while (left_it != -1 && right_it != -1)
	{
		if (intersections_stack[left_it].data.t < intersections_stack[right_it].data.t - 1e-3 * (1 - 2*int(in_a)))
		{
			if (!in_b)
				pushBackToList(current, last_pushed, left_it);
			in_a = intersections_stack[left_it].is_in;
			left_it = intersections_stack[left_it].next_index;
		}
		else
		{
			if (in_a)
			{
				pushBackToList(current, last_pushed, right_it);
				intersections_stack[last_pushed].is_in = !intersections_stack[right_it].is_in;
				intersections_stack[last_pushed].data.n = -1 * intersections_stack[right_it].data.n;
			}
			in_b = intersections_stack[right_it].is_in;
			right_it = intersections_stack[right_it].next_index;
		}
	}
	while (left_it != -1)
	{
		pushBackToList(current, last_pushed, left_it);
		left_it = intersections_stack[left_it].next_index;
	}
	pushBackToList(current, last_pushed, -1);
}

void combineObjects(int current, int left, int right)
{
	switch(composed_objects[current].operation)
	{
	case OBJECTS_ADD:
		uniteObjects(current, left, right);
		return;
	case OBJECTS_MULT:
		intersectObjects(current, left, right);
		return;
	case OBJECTS_SUB:
		subtractObjects(current, left, right);
		return;
	}
}

Intersection intersectWithRay(vec3 camera_pos, vec3 dir, out bool has_intersection)
{
	int current = 0;
	int prev = -1;
	if (ComposedObjectNode_isPrimitive(composed_objects[0]))
	{
		IRO inter_res = intersectWithRay(primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[0])], camera_pos, dir);
		has_intersection = (inter_res.inters > 0);
		return inter_res.first;
	}
	//has_intersection = true;
	while (current != -1)
	{
		if (prev == ComposedObjectNode_parent(composed_objects[current], current))
		{
			int left = ComposedObjectNode_left(composed_objects[current], current);
			if (ComposedObjectNode_isPrimitive(composed_objects[left]))
			{
				
				intersectWithPrimitiveAsNode(left, camera_pos, dir);
				//return MAGENTA_INTER;
				prev = left;
			}
			else
			{
				prev = current;
				current = left;
				continue;
			}
		}
		else if (prev == ComposedObjectNode_left(composed_objects[current], current))
		{
			int right = ComposedObjectNode_right(composed_objects[current], current);
			if (ComposedObjectNode_isPrimitive(composed_objects[right]))
			{
				intersectWithPrimitiveAsNode(right, camera_pos, dir);
				prev = right;
			}
			else
			{
				prev = current;
				current = right;
				continue;
			}
		}
		else
		{
			int left = ComposedObjectNode_left(composed_objects[current], current);
			int right = prev;
			combineObjects(current, left, right);
			prev = current;
			current = ComposedObjectNode_parent(composed_objects[current], current);
		}
	}

	if (lists_stack[0] == -1)
	{
		has_intersection = false;
		Intersection res = {{0,0,0},0};
		return res;
	}
	has_intersection = true;
	return intersections_stack[lists_stack[0]].data;
}



void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (composed_object_nodes_count > STACK_SIZE)
	{
		imageStore(img_output, pixel_coords, vec4(abs(LIGHT_INTER.n), 1));
		return;
	}

    float horizontal_step = 2.0/screen_size.x;
    float vertical_step = 2.0/screen_size.y;
    
    vec3 ray_dir = vec3(-1 + pixel_coords.x * horizontal_step, 1, -1 + pixel_coords.y * vertical_step);

    bool has_intersection;
    Intersection inter = intersectWithRay(camera_pos, ray_dir, has_intersection);

    if (!has_intersection)
	    imageStore(img_output, pixel_coords, vec4(0,0,0,1));
    else
    {
		float r = (0.99 * sqrt((sq(dot(ray_dir, inter.n)) / dot(ray_dir, ray_dir))));
		imageStore(img_output, pixel_coords, vec4(r,r,r,1));
		//imageStore(img_output, pixel_coords, vec4(abs(inter.n),1));
    }

  
}
























//Problems:
//1) нижнее isInsidePolygon действительно ли можно спокойно убрать и почему там проблемы если его включить
//2) в polyhedronе маленькая полоска
//3) небольшое мерцание на границе пирамиды
//4) box_with_windows точка по центру