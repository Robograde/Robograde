#new_shader vertex
#version 430

#define MAX_J 27

#ifdef DRAWID_EXTENSION
#extension GL_ARB_shader_draw_parameters : require
#endif

layout (location = 0) in vec4 PosL;
layout (location = 1) in vec4 NormalL;
layout (location = 2) in vec4 TexIn;
layout (location = 3) in vec4 TangentL;
layout (location = 4) in vec4 JointIDs;
layout (location = 5) in vec4 Weights;

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

layout(std430, binding = 10) buffer SkeletonInput
{
  mat4 matrices[];
};

layout(std430, binding = 14) buffer BindposeInput
{
  mat4 bindPoses[];
};

#ifndef DRAWID_EXTENSION
uniform uint drawID;
#endif
uniform uint animationID;
uniform uint animationOffset;
uniform uint BatchCounts;
void main()
{
  #ifdef DRAWID_EXTENSION
  uint DrawIndex = (gl_DrawIDARB + BatchCounts) + g_InstanceCount[(gl_DrawIDARB + BatchCounts)] + gl_InstanceID;
  #else
  uint DrawIndex = (drawID + BatchCounts) + g_InstanceCount[(drawID + BatchCounts)] + gl_InstanceID;
  #endif

  uint SkeletonIndex = animationOffset + gl_InstanceID * MAX_J;


  vec4 origPos = vec4(PosL.xyz, 1);

//  vec4 DrawIndexColor = vec4(0);
//
//  if (gl_InstanceID < 10)
//{
//	  DrawIndexColor.x = gl_InstanceID / 10.0f;
//}
//  else if (gl_InstanceID < 20)
//{
//	  DrawIndexColor.y = (gl_InstanceID - 10) / 10.0f;
//}
//  else if (gl_InstanceID < 30)
//{
//	  DrawIndexColor.z = (gl_InstanceID - 20) / 10.0f;
//}


mat4 matX = bindPoses[MAX_J * animationID + int(JointIDs.x)];
mat4 matY = bindPoses[MAX_J * animationID + int(JointIDs.y)];
mat4 matZ = bindPoses[MAX_J * animationID + int(JointIDs.z)];
mat4 matW = bindPoses[MAX_J * animationID + int(JointIDs.w)];

vec4 posXBoneSpace = inverse(matX) * origPos;
vec4 posYBoneSpace = inverse(matY) * origPos;
vec4 posZBoneSpace = inverse(matZ) * origPos;
vec4 posWBoneSpace = inverse(matW) * origPos;

vec4 transXPos = matrices[SkeletonIndex + int(JointIDs.x)] * posXBoneSpace;
vec4 transYPos = matrices[SkeletonIndex + int(JointIDs.y)] * posYBoneSpace;
vec4 transZPos = matrices[SkeletonIndex + int(JointIDs.z)] * posZBoneSpace;
vec4 transWPos = matrices[SkeletonIndex + int(JointIDs.w)] * posWBoneSpace;

 vec4 newPos =  Weights.x * transXPos;
 newPos +=   Weights.y * transYPos;
 newPos +=   Weights.z * transZPos;
 newPos +=  Weights.w *  transWPos;

  //vec4 newPos = origPos;
  mat4 world = g_Inputs[DrawIndex].World;
  gl_Position = g_ViewProj * world * newPos;
}
#end_shader

#new_shader fragment
#version 430
out vec4 FragmentColor;
void main()
{
}
#end_shader
