#new_shader compute
#version 430
#define WORK_GROUP_SIZE 16
layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE) in;
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

#define MAX_MATERIALS 100

#include LightStruct.txt

struct SurfaceMaterial{
	vec4	BaseColor;
	vec4	SpecularColor;
	ivec4	AlbedoNormalRoughMetalTexture;
	ivec4	GlowAOPadd2Texture;
};

layout(std140, binding = 4) uniform MaterialBuffer
{
	SurfaceMaterial Materials[MAX_MATERIALS];
};

//GBuffers
uniform sampler2D gAlbedoBuffer;
uniform sampler2D gNormalBuffer;
uniform sampler2D gRoughMetalBuffer;
uniform sampler2D gDepthBuffer;
layout(binding = 5) uniform usampler2D gMaterialBuffer;
layout(binding = 7) uniform sampler2D gShadowMap;
layout(binding = 8) uniform samplerCube g_LightCubeTex;
layout(binding = 9) uniform samplerCube g_IrradianceCube;


layout(rgba8, binding = 0) writeonly restrict uniform image2D output_img;
//uniforms
uniform vec2 gScreenSize;
uniform uint gTotalLightCount;
uniform mat4 gProj;
uniform mat4 gView;
uniform mat4 gInvProjView;
uniform vec4 gCamPos;
uniform sampler2D gFogTex;
uniform vec2 gWorldSize;
uniform uint gDecalCount;
uniform mat4 gShadowMat;
//shared
shared int sMaxDepth;
shared int sMinDepth;
shared uint sPointLightCount;
shared uint sPointLightIndex[MAX_LIGHTS_PER_TILE];

#include LightingFunc.txt

float CalcShadowFactor(in vec4 shadowpos)
{
	const float eps = 0.0001f;
	float offset = 1.0 / textureSize(gShadowMap, 0).x;
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
			//vec3 uvc = vec3(uv + offsets, z + eps);
			float bufferDepth = texture(gShadowMap, uv + offsets ).r;
			factor += (z + eps) < bufferDepth ? 1.0 : 0.0;
		}
	}
	factor /= 9.0f;
	return (0.1) + factor * 0.9; //dont do complete shadowing
}

void main()
{
	if(gl_LocalInvocationIndex == 0)
	{
		sMaxDepth = int(0xFFFFFFFF);
		sMinDepth = 0;
		sPointLightCount = 0;
	}
	barrier(); //sync

	ivec2 screenPos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(screenPos) / vec2(gScreenSize - 1);
	float depth = texelFetch(gDepthBuffer,screenPos.xy,0).r * 2.0f - 1.0f;
	float zNear = gProj[3][2] / (gProj[2][2] - 1.0f);
	float zFar 	= gProj[3][2] / (gProj[2][2] + 1.0f);
	float clipDelta = zFar - zNear;

	//Worldpos
	vec4 sPos = vec4(uv * 2 - 1, depth,1);
	sPos = gInvProjView * sPos;
	vec4 posW = vec4(sPos.xyz / sPos.w , sPos.w);

	float linearDepth = (1.0f / posW.w - zNear) / clipDelta;
	int unsignedDepth = floatBitsToInt(linearDepth);

	atomicMin(sMinDepth, unsignedDepth);
	atomicMax(sMaxDepth, unsignedDepth);

	barrier(); //sync

	float minDepthZ = intBitsToFloat( sMinDepth );
	float maxDepthZ = intBitsToFloat( sMaxDepth );

	minDepthZ = minDepthZ * clipDelta + zNear;
	maxDepthZ = maxDepthZ * clipDelta + zNear;

	vec2 tileScale = (gScreenSize / float(2 * WORK_GROUP_SIZE));
	vec2 tileBias = tileScale - vec2(gl_WorkGroupID.xy);

	vec4 col1 = vec4(-gProj[0][0] * tileScale.x , 0 					, tileBias.x , 0);
	vec4 col2 = vec4( 0 				, -gProj[1][1] * tileScale.y 	, tileBias.y , 0);
	vec4 col4 = vec4( 0 				, 0 					                 , -1.0f 	   , 0);

	vec4 frustrumPlanes[6];
	//left plane
	frustrumPlanes[0] = col4 + col1;
	//right plane
	frustrumPlanes[1] = col4 - col1;
	//top plane
	frustrumPlanes[2] = col4 - col2;
	//bottom plane
	frustrumPlanes[3] = col4 + col2;
	//far plane
	frustrumPlanes[4] = vec4(0.0f, 0.0f,  1.0f, maxDepthZ);
	//near plane
	frustrumPlanes[5] = vec4(0.0f, 0.0f, -1.0f, -minDepthZ);
	//normalize planes
	for(int i = 0; i < 4;i++)
	{
		frustrumPlanes[i].xyz *= 1.0f / length(frustrumPlanes[i].xyz);
	}
	uint threadCount = WORK_GROUP_SIZE * WORK_GROUP_SIZE;
	//calc how many lights this thread should check
	uint passCount = (gTotalLightCount + threadCount - 1) / threadCount;

	for(uint pass = 0; pass < passCount; ++pass)
	{
		uint lightIndex = pass * threadCount + gl_LocalInvocationIndex;

		if(lightIndex > gTotalLightCount)
			break;

		Light currentLight = lights[lightIndex];
		vec4 pos =  (gView) * vec4(currentLight.Position,1);
		float radius = currentLight.Range;
		bool inFrustrum = true;
		for(uint i = 0; i <= 5 && inFrustrum; i++ )
		{
			float dist = dot(frustrumPlanes[i], pos);
			inFrustrum = (-radius <= dist);
		}
		if(inFrustrum)
		{
			uint id = atomicAdd(sPointLightCount,1);
			sPointLightIndex[id] = lightIndex;
		}
	}
	barrier();

	vec4 albedo = pow(texture(gAlbedoBuffer, uv),vec4(2.2));
	vec3 normal = texture(gNormalBuffer, uv).xyz;
	vec2 roughnessMetal = texture(gRoughMetalBuffer,uv).xy;
	uint matID = texture(gMaterialBuffer, uv).x;

	SurfaceMaterial mat = Materials[matID];

	vec4 lightColor = vec4(0.0);
	uint i;
	//for(i = 0; i < numPLights; ++i)
	for(i = 0; i < sPointLightCount; ++i)
	{
		Light p = lights[sPointLightIndex[i]];
		lightColor += CalcPLight(p, normal, posW.xyz, gCamPos.xyz, mat , roughnessMetal.x, roughnessMetal.y );
	}
	for(i = 0; i < numDLights; ++i)
	{
		Light d = lights[MAX_POINTLIGHTS + i];
		lightColor += CalcDLight(d, normal, posW.xyz, gCamPos.xyz, mat, roughnessMetal.x, roughnessMetal.y );
	}
  for(i = 0; i < numLLights; ++i)
  {
    Light ll = lights[MAX_POINTLIGHTS + MAX_DIRLIGHTS + i];
    lightColor += CalcLineLight(ll, normal, posW.xyz, gCamPos.xyz, mat, roughnessMetal.x, roughnessMetal.y);
  }
	//IBL
	vec3 toeye = normalize(gCamPos.xyz - posW.xyz);
	vec3 reflected = reflect(-toeye,normal);

	ivec2 texDim = textureSize(g_LightCubeTex,0);
	float numMips = ceil(log2(max(texDim.x,texDim.y))) - 1.0f;
	float mipLevel = numMips * roughnessMetal.x;
	vec4 iblSpec = textureLod(g_LightCubeTex,reflected, mipLevel);
	iblSpec = pow(iblSpec, vec4(2.2));

	texDim = textureSize(g_IrradianceCube,0);
	numMips = ceil(log2(max(texDim.x,texDim.y))) - 1.0f;
	mipLevel = numMips * roughnessMetal.x;
	vec4 iblDiff = textureLod(g_IrradianceCube, normal, mipLevel);
	iblDiff = pow(iblDiff, vec4(2.2));

	float vdotn = clamp(dot(normalize(normal + toeye),normal),0,1);
	iblSpec *= vec4(FresnelSchlick(vdotn,mat.SpecularColor.xyz),1);
	vec4 iblColor = mix(iblSpec,iblDiff,roughnessMetal.x);

	//FogOfWar
	vec2 fogPos = vec2(posW.x / gWorldSize.x, posW.z / gWorldSize.y);
	float fog = clamp( 1.0f - texture(gFogTex, fogPos).a, 0.0f, 1.0f );
	fog = 1.0f;
	vec4 shadowPos = gShadowMat * vec4(posW.xyz,1);

	float shadow = CalcShadowFactor(shadowPos);
	float fade = 1.0;
	float fadeDist = 25.0;
	float horisontalDist = abs( posW.x - (gWorldSize.x * 0.5));
	float verticalDist = abs( posW.z - (gWorldSize.y * 0.5));
	if(horisontalDist > (gWorldSize.x * 0.5) - fadeDist)
	{
		fade = 1.0 - (abs(horisontalDist - ((gWorldSize.x * 0.5) - fadeDist)) / fadeDist);
	}
	if(verticalDist > (gWorldSize.y * 0.5) - fadeDist)
	{
		fade = fade * (1.0 - (abs(verticalDist - ((gWorldSize.y * 0.5) - fadeDist)) / fadeDist));
	}
	vec4 outColor = (lightColor + iblColor) * albedo * fog * shadow * fade;
	imageStore(output_img, screenPos, pow(outColor, vec4(1.0 / 2.2)));
	//imageStore(output_img, screenPos, vec4(float(maxDepthZ/1.00), 0, 0, 1 ));
}
#end_shader
