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
out float texRepeat;
void main()
{
	float l = length(pos1 - pos2);
	gl_Position = vp * vec4(pos1,1);
	texcoord = vec2(0,0);
	texRepeat = l;
	EmitVertex();
	gl_Position = vp * vec4(pos2,1);
	texcoord = vec2(1,1);
	texRepeat = l;
	EmitVertex();
}
#end_shader

#new_shader fragment
#version 430
uniform vec4 Color;
uniform sampler2D Tex;
uniform float animation;
in vec2 texcoord;
in float texRepeat;

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out vec4 FragmentGlow;
void main()
{
	FragmentColor = texture(Tex, texcoord * (texRepeat) - animation) * Color; //insert texture repeat
	FragmentGlow = vec4(0);
}
#end_shader