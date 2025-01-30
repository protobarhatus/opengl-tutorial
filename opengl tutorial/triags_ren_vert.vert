#version 450

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 0) in vec3 vertex_pos;
layout(binding = 0) uniform u_obj_obj {
    vec3 camera_pos;
    ivec2 screen_size;
    int primitives_count;
    int data_count;
    int normals_count;
    int composed_object_nodes_count;
    vec3 camera_right_vec;
    vec3 camera_direction;
    vec3 camera_up_vector;
    mat4 proj_view;
} ubo;



void main() {
    gl_Position = ubo.proj_view * vec4(vertex_pos, 1);
    //fragColor = colors[(gl_VertexIndex + un_int.obj) % 3];
    fragColor = vec3(1,0,0);
    
}