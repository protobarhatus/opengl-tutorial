#version 330 core

layout(location=0) in vec3 v_pos;
layout(location=1) in vec2 text_coord;

out vec2 v_text_coord;

uniform vec4 u_cr;
uniform vec3 u_pos;

void main()
{
	vec4 pos = vec4(v_pos, 1);


	float a0 = u_cr.x;
	float a1 = u_cr.y;
	float a2 = u_cr.z;
	float a3 = u_cr.w;

	mat4 coord_change = mat4(1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1);
	
	mat4 rotation = mat4(a1*a1 + a0*a0 - a2*a2 - a3*a3, 2*a2*a1 + 2*a0*a3, 2*a3*a1 - 2*a0*a2, 0, 2*a2*a1 - 2*a3*a0, a2*a2 + a0*a0 - a3*a3 - a1*a1, 2*a3*a2 + 2*a1*a0, 0, 2*a3*a1 + 2*a2*a0, 2*a3*a2 - 2*a1*a0, a3*a3 + a0*a0 - a2*a2 - a1*a1, 0, 0, 0, 0, 1);

	mat4 transition = mat4(1, 0, 0, 0,
							0, 1, 0, 0,
								0, 0, 1, 0,
									-u_pos.x, -u_pos.y, -u_pos.z+1, 1);

	mat4 projection = mat4(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, -1, -1,   0, 0, 0, 1);



	gl_Position = (projection * transition * rotation * coord_change) * pos;
	//gl_Position = vec4(v_pos.x, v_pos.y , 0, 2);

	v_text_coord = text_coord;
};