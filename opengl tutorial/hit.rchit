#version 460
#extension GL_EXT_ray_tracing : require

struct Intersection
{
    vec3 n;
    float t;
};
#define _STACK_SIZE 128
struct RayHitList
{
    uint intersections_map[_STACK_SIZE/2];
    Intersection intersections_data[_STACK_SIZE];
    uint intersections_amount;
    vec3 color;
};
struct IntersectionListUnit
{
    int next_index;
    bool is_in;
    int object_index;
};

IntersectionListUnit intersections_stack[_STACK_SIZE];

layout(location = 0) rayPayloadInEXT RayHitList ray_data;



struct IRO
{
    int inters;
    Intersection first, second;
};

struct ComposedObjectNode
{
//положительный означает примитив. Отрицательный - операцию. 
  int operation;
//положительное число - указатель на bb_data. -1 - отсутствие боундинг бокса
  int bb_index;
};
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
    //пока что цвет потом может заменится на текстурную координату или типо того
    vec4 color;
};

layout(binding = 2) uniform u_obj_obj {
    vec3 camera_pos;
    ivec2 screen_size;
    int primitives_count;
    int data_count;
    int normals_count;
    int composed_object_nodes_count;
    int STACK_SIZE;
    int batches_count;
} ubo;

layout(binding = 3) buffer primitives_array
{
    Primitive primitives[];
};

#define OBJECTS_ADD  -1
#define OBJECTS_MULT  -2
#define OBJECTS_SUB  -3

layout(binding = 8) buffer composed_objects_array
{
  ComposedObjectNode composed_objects[];
};

//количество доступных точек. точки которые вылазят за стек будут отброшены. ЕСЛИ ИЗМЕНЯТЬ ТО ОДНОВРЕМЕННО ЗДЕСЬ, в intersection.rint и в коде при создании буфера!!!
//#define STACK_SIZE 100




//hitAttributeEXT RayHitList ray_data;

#define ComposedObjectNode_isPrimitive(X) (X.operation >= 0)
#define ComposedObjectNode_getPrimitiveIndex(X) (X.operation)

#define ComposedObjectNode_left(X, N) (2*((N)+1) - 1)
#define ComposedObjectNode_right(X, N) (2*((N)+1))
#define ComposedObjectNode_parent(X, N) (((N)+1)/2 - 1)

int intersectWithPrimitiveAsNode(int node)
{
    //return int(IDX * ubo.STACK_SIZE);
    //uint prim_index = IDX * ubo.primitives_count + ComposedObjectNode_getPrimitiveIndex(composed_objects[node]);
    //int pre = int(IDX * ubo.STACK_SIZE) + int((primitives_map[prim_index / 4] >> (8*(3 - prim_index % 4))) & 0xFF);
  //return primitives_map[IDX * ubo.primitives_count + ComposedObjectNode_getPrimitiveIndex(composed_objects[node])]
  //return -1;
  //return ray_data.intersections_map[ComposedObjectNode_getPrimitiveIndex(composed_objects[node])];


  /*for (int i = 0; i < 8; ++i)
  {
    for (int j = 1; j <= ray_data.intersections_map[i * 8]; ++j)
    {
      if ((ray_data.intersections_map[i * 8 + j] >> 8) == ComposedObjectNode_getPrimitiveIndex(composed_objects[node]))
        return int(ray_data.intersections_map[i * 8 + j] & 0xFF);
    }
  }
  return -1;*/


  uint bucket = ComposedObjectNode_getPrimitiveIndex(composed_objects[node]) % 8;
  for (int i = 1; i <= ray_data.intersections_map[bucket * 8]; ++i)
  {
    if ((ray_data.intersections_map[bucket * 8 + i] >> 8) == ComposedObjectNode_getPrimitiveIndex(composed_objects[node]))
      return int(ray_data.intersections_map[bucket * 8 + i] & 0xFF);
  }
  return -1;



  for (int i = 0; i < ray_data.intersections_amount; ++i)
  {
    if (ray_data.intersections_map[i] == ComposedObjectNode_getPrimitiveIndex(composed_objects[node]))
      return 2*i;
  }
  return -1;
}


void pushBackToList(inout int list_start, inout int last_pushed, int to_push)
{
  if (last_pushed == -1)
    list_start = to_push;
  else
    intersections_stack[last_pushed].next_index = to_push;
  last_pushed = to_push;
}

//при этом листы, относящиеся к дочерним нодам разрушаются.
int uniteObjects(int left_it, int right_it)
{
  //int left_it = lists_stack[left];
  //int right_it = lists_stack[right];
  
  bool in_a = false;
  bool in_b = false;

  int last_pushed = -1;
  //это будет либо left_it, либо right_it. заполняется в pushBackToList
  int new_list_start = -1;
  while (left_it != -1 && right_it != -1)
  {
    if (ray_data.intersections_data[left_it].t < ray_data.intersections_data[right_it].t)
    {
    // точки не выбрасываем, потому что даже если обьект не прозрачный, то потом при вычитании из него дырки нужно все равно хранить внутренние обьекты
      //if (!in_b)
        pushBackToList(new_list_start, last_pushed, left_it);
      in_a = intersections_stack[left_it].is_in;
      left_it = intersections_stack[left_it].next_index;
    }
    else
    {
      //if (!in_a)
        pushBackToList(new_list_start, last_pushed, right_it);
      in_b = intersections_stack[right_it].is_in;
      right_it = intersections_stack[right_it].next_index;
    }
  }
  //сейчас одно из них дошло точно до конца поэтому никаких ифов можно не делать
  while (right_it != -1)
  {
    pushBackToList(new_list_start, last_pushed, right_it);
    right_it = intersections_stack[right_it].next_index;
  }
  while (left_it != -1)
  {
    pushBackToList(new_list_start, last_pushed, left_it);
    left_it = intersections_stack[left_it].next_index;
  }
  //это по сути закрытие листа, иначе финальный нод будет ссылаться на рандомный другой
  pushBackToList(new_list_start, last_pushed, -1);
  return new_list_start;
}

int intersectObjects(int left_it, int right_it)
{

  int in_a = 0;
  int in_b = 0;

  int last_pushed = -1;
  //int left_it = lists_stack[left];
  //int right_it = lists_stack[right];
  int new_list_start = -1;
  while (left_it != -1 && right_it != -1)
  {
    if (ray_data.intersections_data[left_it].t < ray_data.intersections_data[right_it].t)
    {
      if (in_b > 0)
        pushBackToList(new_list_start, last_pushed, left_it);
      in_a += 2*int(intersections_stack[left_it].is_in) - 1;
      left_it = intersections_stack[left_it].next_index;
    }
    else
    {
      if (in_a > 0)
        pushBackToList(new_list_start, last_pushed, right_it);
      in_b += 2*int(intersections_stack[right_it].is_in) - 1;
      right_it = intersections_stack[right_it].next_index;
    }
  }
  pushBackToList(new_list_start, last_pushed, -1);
  return new_list_start;
}

int subtractObjects(int left_it, int right_it)
{
  int in_a = 0;
  int in_b = 0;

  //int left_it = lists_stack[left];
  //int right_it = lists_stack[right];

  int new_list_start = -1;
  int last_pushed = -1;
  //это для того, чтобы при вычитании обьектов на стенках отображалась правильная текстура, т е текстура самого внутреннего обьекта. но глубина такого запоминания ограничена
  #define MAX_SUB_DEPTH 10
  int left_rememb[MAX_SUB_DEPTH];
  while (left_it != -1 && right_it != -1)
  {
    if (ray_data.intersections_data[left_it].t < ray_data.intersections_data[right_it].t - 1e-3 * (1 - 2*int(in_a)))
    {
      if (in_b == 0)
      {
        pushBackToList(new_list_start, last_pushed, left_it);
        
      }
      left_rememb[in_a] = intersections_stack[left_it].object_index;
      in_a += 2*int(intersections_stack[left_it].is_in) - 1;
      left_it = intersections_stack[left_it].next_index;
    }
    else
    {
      if (in_a > 0)
      {
        pushBackToList(new_list_start, last_pushed, right_it);
        //при этом last_pushed == right_it
        intersections_stack[last_pushed].is_in = !intersections_stack[right_it].is_in;
        //ray_data.intersections_data[last_pushed].n = -1 * ray_data.intersections_data[right_it].n;
        intersections_stack[last_pushed].object_index = left_rememb[min(in_a - 1, MAX_SUB_DEPTH)];
        in_b += 2*int(!intersections_stack[last_pushed].is_in) - 1;
      }
      else
        in_b += 2*int(intersections_stack[right_it].is_in) - 1;
      right_it = intersections_stack[right_it].next_index;
    }
  }
  while (left_it != -1)
  {
    pushBackToList(new_list_start, last_pushed, left_it);
    left_it = intersections_stack[left_it].next_index;
  }
  pushBackToList(new_list_start, last_pushed, -1);
  return new_list_start;
}

int combineObjects(int current, int left_it, int right_it)
{
  switch(composed_objects[current].operation)
  {
  case OBJECTS_ADD:
    return uniteObjects( left_it, right_it);
    ;
  case OBJECTS_MULT:
    return intersectObjects( left_it, right_it);
    ;
  case OBJECTS_SUB:
    return subtractObjects( left_it, right_it);
    ;
  }
}

struct FullIntersectionResult
{
  Intersection inter;
  vec4 color;
  //индекс примитива в дереве composed_objects с которым произошло первое пересечение. Для получения его индекса как примитива следует вызвать ComposedObjectNode_getPrimitiveIndex(composed_objects[object_index])
  int object_index;
};
#define MAX_TREE_DEPTH 20
FullIntersectionResult intersectWithRay(out bool has_intersection)
{
  vec3 dir = gl_WorldRayDirectionEXT;
  int current = 0;
  int prev = -1;
  if (ComposedObjectNode_isPrimitive(composed_objects[0]))
  {
    has_intersection = true;
    FullIntersectionResult res = {ray_data.intersections_data[0], primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[0])].color * (-dot(normalize(dir), ray_data.intersections_data[0].n)), 0};
    return res;
  }

  
  
  int left_stack_list_index[MAX_TREE_DEPTH];
  left_stack_list_index[0] = -1;
  int stack_list_depth = 0;
  //has_intersection = true;
  int prev_node_list_index = -1;
  /*if (composed_objects[0].bb_index > -1)
  {
    IRO bb_iro = intersectBoxShapeWithLine(bb_data[composed_objects[0].bb_index].hsize, camera_pos - bb_data[composed_objects[0].bb_index].position, dir);
    if (bb_iro.inters == 0)
    {
      current = -1;
      prev_node_list_index = -1;
    }
  }*/
  while (current != -1)
  {
    if (prev == ComposedObjectNode_parent(composed_objects[current], current))
    {
      int left = ComposedObjectNode_left(composed_objects[current], current);
      /*if (composed_objects[left].bb_index > -1)
      {
        IRO bb_iro = intersectBoxShapeWithLine(bb_data[composed_objects[left].bb_index].hsize, camera_pos - bb_data[composed_objects[left].bb_index].position, dir);
        if (bb_iro.inters == 0)
        {
          prev_node_list_index = -1;
          prev = left;
        }
      }*/
      if (ComposedObjectNode_isPrimitive(composed_objects[left]))
      {
        prev_node_list_index = intersectWithPrimitiveAsNode(left);
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
      left_stack_list_index[stack_list_depth] = prev_node_list_index;
      stack_list_depth++;
      
      int right = ComposedObjectNode_right(composed_objects[current], current);
      /*if (composed_objects[right].bb_index > -1)
      {
        IRO bb_iro = intersectBoxShapeWithLine(bb_data[composed_objects[right].bb_index].hsize, camera_pos - bb_data[composed_objects[right].bb_index].position, dir);
        if (bb_iro.inters == 0)
        {
          prev_node_list_index = -1;
          prev = right;
        }
      }*/
      if (ComposedObjectNode_isPrimitive(composed_objects[right]))
      {
        prev_node_list_index = intersectWithPrimitiveAsNode(right);
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
      stack_list_depth--;
      prev_node_list_index = combineObjects(current, left_stack_list_index[stack_list_depth], prev_node_list_index);
      prev = current;
      current = ComposedObjectNode_parent(composed_objects[current], current);
    }
  }

  if (prev_node_list_index == -1)
  {
    has_intersection = false;
    FullIntersectionResult res = { {{0,0,0},0}, {0,0,0,0},0};
    return res;
  }
  has_intersection = true;
  
  
  vec3 normalized_ray_dir = normalize(dir);
  //NORMAL COLOR BLENDING
  /**/vec4 color = primitives[intersections_stack[prev_node_list_index].object_index].color;
  color.rgb *= -dot(normalized_ray_dir, ray_data.intersections_data[prev_node_list_index].n);
   
  float passed_alph = 1 - color.a;
  int current_node = intersections_stack[prev_node_list_index].next_index;
  while (current_node != -1 && passed_alph > 0.01 )
  {
    int current_node_primitive = intersections_stack[current_node].object_index;
    color.rgb = mix(color.xyz, primitives[current_node_primitive].color.rgb *
                     abs(dot(normalized_ray_dir, ray_data.intersections_data[current_node].n)), passed_alph);
    passed_alph *= (1 - primitives[current_node_primitive].color.a);
    current_node = intersections_stack[current_node].next_index;
  }
  
  color.a = 1 - passed_alph;

  //Meshkin’s Method
  /*vec4 color = primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[lists_stack[0]].object_index])].color;
  color.rgb *= -dot(normalized_ray_dir, intersections_stack[lists_stack[0]].data.n);
  color.rgb *= color.a;
  float passed_alph = 1 - color.a;
  float alpha_sum = color.a;
  int current_node = intersections_stack[lists_stack[0]].next_index;
  while (current_node != -1 && passed_alph > 0.01)
  {
    int current_node_primitive = ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[current_node].object_index]);
    vec3 other_c = primitives[current_node_primitive].color.rgb * abs(dot(normalized_ray_dir, intersections_stack[current_node].data.n));
    color.rgb += other_c * primitives[current_node_primitive].color.a;
    current_node = intersections_stack[current_node].next_index;
    passed_alph *= (1 - primitives[current_node_primitive].color.a);
  }
  color.a = 1;*/

  //Bavoil and Myers’ Method
  /*vec4 color = primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[prev_node_list_index].object_index])].color;
  color.rgb *= -dot(normalized_ray_dir, intersections_stack[prev_node_list_index].data.n);
  int current_node = intersections_stack[prev_node_list_index].next_index;
  color.rgb *= color.a;
  int n = 1;
  float alpha_sum = color.a;
  float passed_alph = 1 - color.a;
  float prev_mult = color.a;
  while (current_node != -1 && passed_alph > 0.01)
  {
    int current_node_primitive = ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[current_node].object_index]);
    vec3 other_c = primitives[current_node_primitive].color.rgb * abs(dot(normalized_ray_dir, intersections_stack[current_node].data.n));
    color.rgb += other_c * primitives[current_node_primitive].color.a;
    n += 1;
    alpha_sum += primitives[current_node_primitive].color.a;
    current_node = intersections_stack[current_node].next_index;
    passed_alph *= (1 - primitives[current_node_primitive].color.a);
  }
  color.rgb = color.rgb / max(alpha_sum, 0.0001) * (1. - pow(1. - 1.0/n * alpha_sum, n));
  color.a = 1;*/

  //A New Blended OIT Method
  /*vec4 color = primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[lists_stack[0]].object_index])].color;
  color.rgb *= -dot(normalized_ray_dir, intersections_stack[lists_stack[0]].data.n);
  int current_node = intersections_stack[lists_stack[0]].next_index;
  color.rgb *= color.a;
  float alpha_sum = color.a;
  float passed_alph = 1 - color.a;
  float prev_mult = color.a;
  while (current_node != -1 && passed_alph > 0.01)
  {
    int current_node_primitive = ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[current_node].object_index]);
    vec3 other_c = primitives[current_node_primitive].color.rgb * abs(dot(normalized_ray_dir, intersections_stack[current_node].data.n));
    alpha_sum += primitives[current_node_primitive].color.a;
    passed_alph *= (1 - primitives[current_node_primitive].color.a);
    color.rgb += other_c * primitives[current_node_primitive].color.a;
    current_node = intersections_stack[current_node].next_index;
  }
  color.rgb = color.rgb / max(alpha_sum, 0.00001) * (1 - passed_alph);
  color.a = 1;*/

  //Blended OIT with weights
  /*vec4 color = primitives[ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[lists_stack[0]].object_index])].color;
  color.rgb *= -dot(normalized_ray_dir, intersections_stack[lists_stack[0]].data.n);
  int current_node = intersections_stack[lists_stack[0]].next_index;
  color.rgb *= color.a;
  float alpha_sum = color.a;
  float passed_alph = 1 - color.a;
  float prev_mult = color.a;
  float A = color.a;
  float Z = intersections_stack[lists_stack[0]].data.t;
  float W = A * max(0.01, min(3000, 10.0/(0.00001 + (Z/5)*(Z/5) +  pow((Z/200), 6)    )));
  color.rgb *= W;
  alpha_sum *= W;
  while (current_node != -1 && passed_alph > 0.01)
  {
    int current_node_primitive = ComposedObjectNode_getPrimitiveIndex(composed_objects[intersections_stack[current_node].object_index]);
    vec3 other_c = primitives[current_node_primitive].color.rgb * abs(dot(normalized_ray_dir, intersections_stack[current_node].data.n));
    A = primitives[current_node_primitive].color.a;
    Z = intersections_stack[current_node].data.t;
    W = A * max(0.01, min(3000, 10.0/(0.00001 + (Z/5)*(Z/5) +  pow((Z/200), 6)    )));
    alpha_sum += primitives[current_node_primitive].color.a * W;
    passed_alph *= (1 - primitives[current_node_primitive].color.a);
    color.rgb += other_c * primitives[current_node_primitive].color.a * W;
    current_node = intersections_stack[current_node].next_index;
  }
  color.rgb = color.rgb / max(alpha_sum, 0.00001) * (1 - passed_alph);
  color.a = 1;*/

  FullIntersectionResult res = {ray_data.intersections_data[prev_node_list_index], color, intersections_stack[prev_node_list_index].object_index};
  return res;


}


void main() {

    

    for (int i = 0; i < ray_data.intersections_amount; ++i)
    {
        intersections_stack[2*i].next_index = 2*i + 1;
        intersections_stack[2*i].is_in = true;

        intersections_stack[2*i + 1].next_index = -1;
        intersections_stack[2*i + 1].is_in = false;

        //intersections_stack[2*i].object_index = intersections_stack[2*i + 1].object_index = ray_data.intersections_map[i];
    }
    for (int i = 0; i < 8; ++i)
    {
      for (int j = 1; j <= ray_data.intersections_map[i * 8]; ++j)
      {
        uint stack_ind = ray_data.intersections_map[i * 8 + j] & 0xFF;
        uint obj_index = ray_data.intersections_map[i * 8 + j] >> 8;
        intersections_stack[stack_ind].object_index = intersections_stack[stack_ind + 1].object_index = int(obj_index);
      }
    }
    //for (int i = 0; i < 4096; ++i)
    //    if (ray_data.intersections_map[i] > -1)
    //        intersections_stack[ray_data.intersections_map[i]].object_index = intersections_stack[ray_data.intersections_map[i] + 1].object_index = i;

    
    bool has_intersection = false;
    FullIntersectionResult res = intersectWithRay(has_intersection);
    if (has_intersection)
    {
      ray_data.color = res.color.rgb;
    }
    else
      ray_data.color = vec3(0,0,0);
    
}