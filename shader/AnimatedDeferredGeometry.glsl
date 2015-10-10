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

layout (location = 0) out vec4 PosW;
layout (location = 1) out vec4 NormalW;
layout (location = 2) out vec4 TexOut;
layout (location = 3) out vec4 TangentW;
layout (location = 4) out vec4 ColorOut;
layout (location = 5) flat out uint MatID;
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
  PosW = vec4((world * vec4(newPos.xyz,1)).xyz,1);
  NormalW = vec4((world * vec4(NormalL.xyz,0)).xyz,0);
  TangentW = vec4(1,0,0,0);//vec4((world * vec4(TangentL.xyz,0)).xyz,0);
  TexOut = vec4(TexIn.xy,0,0);
  ColorOut = g_Inputs[DrawIndex].Color;
  MatID = g_Inputs[DrawIndex].MatIDPadd3.x;
}
#end_shader

#new_shader fragment
#version 430
//input
layout (location = 0) in vec4 PosW;
layout (location = 1) in vec4 NormalW;
layout (location = 2) in vec4 TexOut;
layout (location = 3) in vec4 TangentW;
layout (location = 4) in vec4 ColorOut;
layout (location = 5) flat in uint MatID;
//output
layout(location = 0) out vec3 FragmentColor;
layout(location = 1) out vec3 FragmentNormal;
layout(location = 2) out vec2 FragmentRoughMetal;
layout(location = 3) out uint FragmentMatID;
//layout(location = 4) out vec3 FragmentEmissive;
layout(location = 4) out vec3 FragmentGlow;
//uniforms
uniform bool           useDiffuse;
uniform bool           useNormal;
uniform bool           useRoughness;
uniform bool           useMetal;
uniform bool           useGlow;
layout ( binding = 0) uniform sampler2D          g_DiffuseTex;
layout ( binding = 1) uniform sampler2D          g_NormalTex;
layout ( binding = 2) uniform sampler2D          g_RoughnessTex;
layout ( binding = 3) uniform sampler2D          g_MetalTex;
layout ( binding = 6) uniform sampler2D          g_GlowTex;

layout ( binding = 4) uniform samplerCube        g_LightCubeTex;
layout ( binding = 5) uniform samplerCube        g_IrradianceCube;
#define MAX_MATERIALS 100
//buffers & structs
struct TextureFrame{
  float S;
  float T;
  float NormWidth;
  float NormHeight;
};

struct SurfaceMaterial{
  vec4  BaseColor;
  vec4  SpecularColor;
  ivec4 AlbedoNormalRoughMetalTexture;
  ivec4 GlowAODecalPaddTexture;
};

layout(std140,binding = 6) uniform TextureBuffer{
  TextureFrame g_TextureFrames[MAX_MATERIALS * 8];
};

layout(std140, binding = 0) uniform SceneInputs{
    mat4 g_ViewProj;
    vec4 g_CamPos;
};
layout(std140, binding = 4) uniform MaterialBuffer{
  SurfaceMaterial Materials[MAX_MATERIALS];
};
//functions
vec3 CalcBumpedNormal(in vec3 inNormal, in vec3 inTangent,in vec2 inTex){
  vec3 normal = normalize(inNormal);
  vec3 tangent = normalize(inTangent);
  tangent = normalize(tangent - dot(tangent,normal) * normal);
  vec3 bitangent = cross(tangent,normal);

  vec3 bump = texture(g_NormalTex,inTex).xyz;
  bump = (bump * 2.0) - 1.0;

  mat3 TBN = mat3(tangent,bitangent,normal);
  vec3 newNormal = TBN * bump;
  return normalize(newNormal);
}
vec3 FresnelSchlick(in float HdotL, in vec3 spec)
{
  return spec + (vec3(1.0f) - spec) * pow(1.0 - HdotL,5);
}
void main()
{

  float Roughness = 0;
  float Metal = 0;
  SurfaceMaterial mat = Materials[MatID];
  vec4 textureColor;
  if(useDiffuse)
  {
    TextureFrame frame = g_TextureFrames[mat.AlbedoNormalRoughMetalTexture.x];
    vec2 texcoord;
    vec2 tex =  modf(TexOut.xy,texcoord);//fract(TexOut.xy);
    texcoord.x = float(frame.S + frame.NormWidth * tex.x); // mod(frame.NormWidth * TexOut.x ,frame.NormWidth);  //mix(frame.S, frame.S + frame.NormWidth, TexOut.x);
    texcoord.y = float(frame.T + frame.NormHeight * tex.y); //mod(frame.NormHeight * TexOut.y ,frame.NormHeight); //mix(frame.T, frame.T - frame.NormHeight, TexOut.y);

    textureColor = texture(g_DiffuseTex,texcoord);
    textureColor = pow(textureColor,vec4(2.2));
    if ( textureColor.w < 0.1 ) // TODOHJ: Is it suppose to be like this? //JM
    {
      discard;
    }
    //textureColor = vec4(1);
  }
  else
  {
    textureColor = vec4(1);
  }
  vec3 normal = vec3(0);
  if(useNormal)
  {
      TextureFrame frame_normal = g_TextureFrames[mat.AlbedoNormalRoughMetalTexture.y];
      vec2 texcoord_norm;
      vec2 tex =  modf(TexOut.xy,texcoord_norm);
      texcoord_norm.x = float(frame_normal.S + frame_normal.NormWidth * tex.x); //frame_normal.S + mod(frame_normal.NormWidth * TexOut.x ,frame_normal.NormWidth); //mix(frame_normal.S, frame_normal.S + frame_normal.NormWidth, TexOut.x);
      texcoord_norm.y = float(frame_normal.T + frame_normal.NormHeight * tex.y); //frame_normal.T + mod(frame_normal.NormHeight * TexOut.y ,frame_normal.NormHeight); // mix(frame_normal.T, frame_normal.T - frame_normal.NormHeight, TexOut.y);
      normal = CalcBumpedNormal(NormalW.xyz,TangentW.xyz,texcoord_norm);
  }
  else
  {
    normal = normalize(NormalW.xyz);
  }
  if(useRoughness)
  {
    TextureFrame frame_Rough = g_TextureFrames[mat.AlbedoNormalRoughMetalTexture.z];
    vec2 texcoord;
    vec2 tex =  modf(TexOut.xy,texcoord);
    texcoord.x = float(frame_Rough.S + frame_Rough.NormWidth * tex.x);
    texcoord.y = float(frame_Rough.T + frame_Rough.NormHeight * tex.y);

    Roughness = texture(g_RoughnessTex, texcoord).r;
  }
  else
  {
    Roughness = 1.0f; //should probably have a factor in the material
  }
  if(useMetal)
  {
    TextureFrame frame_metal = g_TextureFrames[mat.AlbedoNormalRoughMetalTexture.w];
    vec2 texcoord;
    vec2 tex =  modf(TexOut.xy,texcoord);
    texcoord.x = float(frame_metal.S + frame_metal.NormWidth * tex.x);
    texcoord.y = float(frame_metal.T + frame_metal.NormHeight * tex.y);
    Metal = texture(g_MetalTex, texcoord).r;
  }
  else
  {
    Metal = 0.0f; //should probably have a factor in the material
  }
  if(useGlow)
  {
    TextureFrame frame_Glow = g_TextureFrames[mat.GlowAODecalPaddTexture.x];
    vec2 texcoord;
    vec2 tex =  modf(TexOut.xy,texcoord);
    texcoord.x = float(frame_Glow.S + frame_Glow.NormWidth * tex.x);
    texcoord.y = float(frame_Glow.T + frame_Glow.NormHeight * tex.y);
    FragmentGlow = texture(g_GlowTex, texcoord).xyz * ColorOut.xyz;
  }
  else
  {
    FragmentGlow = vec3(0);
  }

  //texture + tint + fog
  FragmentColor = pow((textureColor).xyz, vec3(1.0 / 2.2));
  FragmentNormal = normal;
  FragmentMatID = MatID;
  FragmentRoughMetal.x = Roughness;
  FragmentRoughMetal.y = Metal;

  //FragmentEmissive =  vec3(0);
}

#end_shader
