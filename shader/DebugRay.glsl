#new_shader vertex
#version 430
void main()
{
}

#end_shader

#new_shader geometry
#version 430
layout(points) in;
layout(line_strip, max_vertices = 2) out;
uniform vec3 pos1;
uniform vec3 pos2;
uniform mat4 vp;

out vec2 texcoord;
void main()
{
	gl_Position = vp * vec4(pos1,1);
	texcoord = vec2(0,0);
	EmitVertex();
	gl_Position = vp * vec4(pos2,1);
	texcoord = vec2(1,1);
	EmitVertex();
}
#end_shader

#new_shader fragment
#version 430
uniform vec3 Color;

in vec2 texcoord;
out vec4 FragmentColor;
void main()
{
	FragmentColor = vec4(Color,1);
}
#end_shader