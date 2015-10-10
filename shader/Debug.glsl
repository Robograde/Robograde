#new_shader vertex
#version 430
layout (location = 0) in vec3 PosL;
uniform mat4 wvp;

void main()
{
	gl_Position = wvp * vec4(PosL,1);
}

#end_shader

#new_shader fragment
#version 430
uniform vec3 Color;

out vec4 FragmentColor;
void main()
{
	FragmentColor = vec4(Color,1);
}
#end_shader
