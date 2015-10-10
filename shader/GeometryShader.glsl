#new_shader vertex
#version 430

#ifdef DRAWID_EXTENSION
#extension GL_ARB_shader_draw_parameters : require
#endif

layout (location = 0) in vec4 PosL;
layout (location = 1) in vec4 NormalL;
layout (location = 2) in vec4 TexIn;
layout (location = 3) in vec4 TangentL;

layout (location = 0) out vec4 PosW;
layout (location = 1) out vec4 NormalW;
layout (location = 2) out vec4 TexOut;
layout (location = 3) out vec4 TangentW;
layout (location = 4) out vec4 ColorOut;
layout (location = 5) out vec4 ShadowPos;
layout (location = 6) flat out uint MatID;
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
uniform uint drawID;
#endif
uniform uint BatchCounts;
uniform mat4 ShadowMat;
void main()
{
	#ifdef DRAWID_EXTENSION
    uint DrawIndex = (gl_DrawIDARB + BatchCounts) + g_InstanceCount[(gl_DrawIDARB + BatchCounts)] + gl_InstanceID;
  #else
    uint DrawIndex = (drawID + BatchCounts) + g_InstanceCount[(drawID + BatchCounts)] + gl_InstanceID;
  #endif

    mat4 world = g_Inputs[DrawIndex].World;
    gl_Position = (g_ViewProj * world) * vec4(PosL.xyz,1);
    PosW = vec4((world * vec4(PosL.xyz,1)).xyz,1);
    NormalW = vec4((world * vec4(NormalL.xyz,0)).xyz,0);
    TangentW = vec4((world * vec4(TangentL.xyz,0)).xyz,0);
    TexOut = vec4(TexIn.xy,0,0);
    ColorOut = g_Inputs[DrawIndex].Color;
    MatID = g_Inputs[DrawIndex].MatIDPadd3.x;
    ShadowPos = ShadowMat * PosW;
}
#end_shader

#new_shader fragment
#version 430
layout (location = 0) in vec4 PosW;
layout (location = 1) in vec4 NormalW;
layout (location = 2) in vec4 TexOut;
layout (location = 3) in vec4 TangentW;
layout (location = 4) in vec4 ColorOut;
layout (location = 5) in vec4 ShadowPos;
layout (location = 6) flat in uint MatID;

out vec4 FragmentColor;
#define MAX_MATERIALS 100
#define LIGHT_SIZES 8192
layout ( binding = 0) uniform sampler2D 	       g_DiffuseTex;
layout ( binding = 1) uniform sampler2D 	       g_NormalTex;
layout ( binding = 2) uniform sampler2D 	       g_RoughnessTex;
layout ( binding = 3) uniform sampler2D 	       g_MetalTex;
layout ( binding = 4) uniform samplerCube 	     g_LightCubeTex;
layout ( binding = 5) uniform samplerCube 	     g_IrradianceCube;
layout ( binding = 6) uniform sampler2D		       g_FogOfWarTex;
layout ( binding = 7) uniform sampler2DShadow		 g_ShadowMap;
uniform bool 		       useDiffuse;
uniform bool 		       useNormal;
uniform bool 		       useRoughness;
uniform bool 		       useMetal;
uniform uint           gNumTiles;
uniform uint           gTileSize;
uniform vec2			     g_WorldSize;

#include LightStruct.txt

struct SurfaceMaterial{
  vec4	BaseColor;
  vec4	SpecularColor;
  ivec4	AlbedoNormalRoughMetalTexture;
};

struct TextureFrame{
	float S;
  float T;
  float NormWidth;
  float NormHeight;
};

layout(std140, binding = 0) uniform SceneInputs{
    mat4 g_ViewProj;
    vec4 g_CamPos;
};

layout(std140, binding = 4) uniform MaterialBuffer{
	SurfaceMaterial Materials[MAX_MATERIALS];
};

layout(std430, binding = 5) buffer LightIndexBuffer
{
  uint gLightSizes[LIGHT_SIZES];
  uint gLightIndices[];
};

layout(std140,binding = 6) uniform TextureBuffer{
	TextureFrame g_TextureFrames[MAX_MATERIALS * 4];
};

#include LightingFunc.txt

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

float CalcShadowFactor(in vec4 shadowpos)
{
  const float eps = 0.0001f;
  float offset = 1.0 / textureSize(g_ShadowMap, 0).x;
  vec3 projCoord = shadowpos.xyz / shadowpos.w;
  vec2 uv;
  uv.x = 0.5f * projCoord.x + 0.5f;
  uv.y = 0.5f * projCoord.y + 0.5f;
  float z = 0.5f * projCoord.z + 0.5f;

  float factor = 0.0f;
  for(int y = -1 ; y <= 1; y++)
  {
    for(int x = -1; x <= 1; x++)
    {
      vec2 offsets = vec2(x * offset, y * offset);
      vec3 uvc = vec3(uv + offsets, z + eps);
      factor += texture(g_ShadowMap, uvc);
    }
  }
  return 1;
}

void main()
{
	vec4 lightColor = vec4(0);
  float Roughness = 0;
  float Metal = 0;
	SurfaceMaterial mat = Materials[MatID];
	vec4 textureColor = vec4(0);

	if(useDiffuse)
	{
		TextureFrame frame = g_TextureFrames[mat.AlbedoNormalRoughMetalTexture.x];
		vec2 texcoord;
    vec2 tex = clamp(fract(TexOut.xy),0.0,1.0);
		texcoord.x = float(frame.S + frame.NormWidth * tex.x); // mod(frame.NormWidth * TexOut.x ,frame.NormWidth);  //mix(frame.S, frame.S + frame.NormWidth, TexOut.x);
		texcoord.y = float(frame.T + frame.NormHeight * tex.y); //mod(frame.NormHeight * TexOut.y ,frame.NormHeight); //mix(frame.T, frame.T - frame.NormHeight, TexOut.y);

		textureColor = texture(g_DiffuseTex,texcoord);
    textureColor = pow(textureColor,vec4(2.2));
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
  		texcoord_norm.x = frame_normal.S + mod(frame_normal.NormWidth * TexOut.x ,frame_normal.NormWidth); //mix(frame_normal.S, frame_normal.S + frame_normal.NormWidth, TexOut.x);
  		texcoord_norm.y = frame_normal.T + mod(frame_normal.NormHeight * TexOut.y ,frame_normal.NormHeight); // mix(frame_normal.T, frame_normal.T - frame_normal.NormHeight, TexOut.y);
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
    texcoord.x = frame_Rough.S + mod(frame_Rough.NormWidth * TexOut.x ,frame_Rough.NormWidth);
    texcoord.y = frame_Rough.T + mod(frame_Rough.NormHeight * TexOut.y ,frame_Rough.NormHeight);

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
    texcoord.x = frame_metal.S + mod(frame_metal.NormWidth * TexOut.x ,frame_metal.NormWidth);
    texcoord.y = frame_metal.T + mod(frame_metal.NormHeight * TexOut.y ,frame_metal.NormHeight);
    Metal = texture(g_MetalTex, texcoord).r;
  }
  else
  {
    Metal = 1.0f; //should probably have a factor in the material
  }

  uvec2 tile = uvec2(gl_FragCoord.xy / gTileSize);
  uint tileindex = tile.x + tile.y * gNumTiles;
  uint pointLightCount = gLightSizes[tileindex];
  tileindex = (tile.x * MAX_LIGHTS_PER_TILE ) + (tile.y * gNumTiles * MAX_LIGHTS_PER_TILE );


  for(int i = 0; i < numPLights;i++)
  {
      Light p = lights[i];
      lightColor += CalcPLight(p, normal, PosW.xyz, g_CamPos.xyz, mat, Roughness , Metal);
  }

	for(int i = 0; i < numDLights;i++)
	{
    Light d = lights[MAX_POINTLIGHTS + i];
		lightColor += CalcDLight(d, normal, PosW.xyz, g_CamPos.xyz, mat, Roughness, Metal);
	}
  //IBL
  vec3 toeye = normalize(g_CamPos.xyz - PosW.xyz);
  vec3 reflected = reflect(-toeye,normal);

  ivec2 texDim = textureSize(g_LightCubeTex,0);
  float numMips = ceil(log2(max(texDim.x,texDim.y))) - 1.0f;
  float mipLevel = numMips * Roughness;
  vec4 iblSpec = textureLod(g_LightCubeTex,reflected, mipLevel);
  iblSpec = pow(iblSpec, vec4(2.2));
  texDim = textureSize(g_IrradianceCube,0);
  numMips = ceil(log2(max(texDim.x,texDim.y))) - 1.0f;
  mipLevel = numMips * Roughness;
  vec4 iblDiff = textureLod(g_IrradianceCube, normal, mipLevel);
  iblDiff = pow(iblDiff, vec4(2.2));
  float vdotn = clamp(dot(normalize(normal + toeye),normal),0,1);
  iblSpec *= vec4(FresnelSchlick(vdotn,mat.SpecularColor.xyz),1);
  vec4 iblColor = mix(iblSpec,iblDiff,Roughness);

  vec2 fogTexCoord = vec2( PosW.x / g_WorldSize.x, PosW.z / g_WorldSize.y );
  float fog = clamp( 1.0f - texture(g_FogOfWarTex, fogTexCoord).a, 0.0f, 1.0f );

  FragmentColor = textureColor * ColorOut;
  lightColor = clamp((lightColor + iblColor) * CalcShadowFactor(ShadowPos) * fog, 0.0f, 1.0f);
  //Gamma correction
  FragmentColor = pow(FragmentColor * lightColor, vec4(1.0 / 2.2));
  //float lerp = 1.0 - (pointLightCount / 64.0f);
  //FragmentColor = mix(vec4(1,0,0,1),vec4(0,0,1,1),lerp);
}
#end_shader
