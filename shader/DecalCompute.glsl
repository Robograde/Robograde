#new_shader compute
#version 430
#define WORK_GROUP_SIZE 16
layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE) in;
struct Decal
{
  mat4  ViewProj;
  vec3  Tint;
  uint  Texture;
};

struct SurfaceMaterial{
  vec4	BaseColor;
  vec4	SpecularColor;
  ivec4	AlbedoNormalRoughMetalTexture;
  ivec4	GlowAOPadd2Texture;
};

layout(std140, binding = 4) uniform MaterialBuffer
{
  SurfaceMaterial gMaterials[MAX_MATERIALS];
};

layout(std140, binding = 12) uniform DecalBuffer
{
  Decal gDecals[MAX_MATERIALS];
};

uniform uint gDecalCount;
uniform sampler2D gDecalAlbedo;
uniform sampler2D gDepthTex;
uniform mat4 gInvViewProj;
uniform vec2 gScreenSize;

layout(binding = 5) uniform usampler2D gMaterialBuffer;

layout(rgba8, binding = 0) writeonly restrict uniform image2D output_img; //albedo texture


void main()
{

}

#end_shader
