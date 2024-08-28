#version 460
#extension GL_EXT_ray_tracing : require

struct Payload
{
  vec3 color;
  float hitDistance;
};

layout(location = 0) rayPayloadInEXT Payload prd;


void main() {

    prd.color = vec3(0, 0, 0);

}