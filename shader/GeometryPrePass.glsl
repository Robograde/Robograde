#new_shader vertex
#version 430
#ifdef DRAWID_EXTENSION
#extension GL_ARB_shader_draw_parameters : require
#endif
layout (location = 0) in vec4 PosL;
layout (location = 1) in vec4 NormalL;
layout (location = 2) in vec4 TexIn;
layout (location = 3) in vec4 TangentL;
struct ShaderInput
{
    mat4 World;
    vec4 Color;
    ivec4 MatIDPadd3;
};

layout(std140, binding = 0) uniform SceneInputs
{
    mat4 g_ViewProj;
    vec4 g_CamPos;
};

layout(std430, binding = 1) buffer ShaderInputs
{
    ShaderInput g_Inputs[];
};

layout(std430, binding = 2) buffer InstanceInput
{
    uint g_InstanceCount[];
};
#ifndef DRAWID_EXTENSION
uniform uint did;
#endif
uniform uint BatchCounts;

void main()
{
	#ifdef DRAWID_EXTENSION
    uint DrawIndex = (gl_DrawIDARB + BatchCounts) + g_InstanceCount[(gl_DrawIDARB + BatchCounts)] + gl_InstanceID;
    #else
    uint DrawIndex = (did + BatchCounts) + g_InstanceCount[(did + BatchCounts)] + gl_InstanceID;
    #endif

	  ShaderInput si = g_Inputs[DrawIndex];
    gl_Position = (g_ViewProj * si.World) * vec4(PosL.xyz,1);
}
#end_shader

#new_shader fragment
#version 430
out vec4 FragmentColor;
void main()
{
}

#end_shader
