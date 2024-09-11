#version 460
#extension GL_EXT_ray_tracing : require

#define STACK_SIZE 128
struct Intersection
{
    vec3 n;
    float t;
};
struct RayHitList
{
    uint intersections_map[STACK_SIZE/2];
    Intersection intersections_data[STACK_SIZE];
    uint intersections_amount;
    vec3 color;
};

layout(location = 0) rayPayloadInEXT RayHitList ray_data;


void main() {

    ray_data.color = vec3(0, 0, 0);

}