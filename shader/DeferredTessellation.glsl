#new_shader vertex
#version 430
layout (location = 0) in vec3 posL;
layout (location = 1) in vec3 normalL;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec2 texelCoord;
uniform mat4 world;
out vec3 posW_CS_in;
out vec3 normalW_CS_in;
out vec2 texCoord_CS_in;
out vec2 texelCoord_CS_in;
out vec4 ShadowCoord_CS_in;


void main(){
	posW_CS_in = (world * vec4(posL,1)).xyz;
	normalW_CS_in = (world * vec4(normalL,0)).xyz;
	texCoord_CS_in = texCoord;
	texelCoord_CS_in = texelCoord;
}
#end_shader


#new_shader fragment
#version 430
in vec3 posW;
in vec3 normalW;
in vec2 texCoord;
in vec2	texelCoord;

#define LIGHT_SIZES 8192
layout (binding = 2) uniform sampler2D Albedo1;
layout (binding = 7) uniform sampler2D Albedo2;
layout (binding = 8) uniform sampler2D Albedo3;
layout (binding = 9) uniform sampler2D Albedo4;
layout (binding = 10) uniform sampler2D Blendmap;

layout (binding = 3) uniform sampler2D roughTex;
layout (binding = 4) uniform sampler2DShadow g_ShadowMap;

uniform vec3 color;
uniform bool hasTexture;
uniform vec3 eyepos;
uniform vec2 g_WorldSize;
uniform int textureRepeat;
uniform float heightScale;
uniform mat4 gShadowMat;

struct SurfaceMaterial{
	vec4	BaseColor;
	vec4	SpecularColor;
	ivec4	AlbedoNormalRoughMetalTexture;
	ivec4	GlowAOPadd2Texture;
};

layout(std140, binding = 0) uniform SceneInputs
{
	mat4 g_ViewProj;
	vec4 g_CamPos;
};

layout(std430, binding = 5) buffer LightIndexBuffer
{
  uint gLightSizes[LIGHT_SIZES];
  uint gLightIndices[];
};

layout(location = 0) out vec3 FragmentColor;
layout(location = 1) out vec3 FragmentNormal;
layout(location = 2) out vec2 FragmentRoughMetal;
layout(location = 3) out uint FragmentMatID;
//layout(location = 4) out vec3 FragmentEmissive;
layout(location = 4) out vec4 FragmentGlow;


float CalcShadowFactor(in vec4 shadowpos)
{
	const float eps = 0.00f;
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
	return (factor / 9.0f);
}
vec3 FresnelSchlick(in float HdotL, in vec3 spec)
{
	return spec + (vec3(1.0f) - spec) * pow(1.0 - HdotL,5);
}
void main(){
	float roughness = 1.0f;
	SurfaceMaterial mat;
	mat.BaseColor = vec4(1);
	mat.SpecularColor = vec4(1);

	vec4 outColor;
	if(hasTexture)
	{
		vec2 tmpTC = texCoord * textureRepeat;
		//vec4 tempColor = pow(texture(testTex, tmpTC), vec4(2.2));
		vec4 tex1 = pow(texture(Albedo1, tmpTC), vec4(2.2));
		vec4 tex2 = pow(texture(Albedo2, tmpTC), vec4(2.2));
		vec4 tex3 = pow(texture(Albedo3, tmpTC), vec4(2.2));
		vec4 tex4 = pow(texture(Albedo4, tmpTC), vec4(2.2));
		vec4 blend = pow(texture(Blendmap, texCoord), vec4(2.2));
		//blend = clamp(blend, 0.0, 1.0);
		//float strength = 1.0f - ((color.x + color.y + color.z) / 3.0f);
		vec4 tempColor = (tex1 * blend.x * blend.w) + (tex2 * blend.y * blend.w) + (tex3 * blend.z * blend.w) + (tex4 * (1.0 - blend.w));

		outColor = tempColor * vec4(color,1);
		roughness =  clamp( texture(roughTex, texCoord).r, 0.03, 1.0);
	}
	else
	{
		outColor = clamp(vec4(color, 1),0.0f,1.0f);
	}
	//vec4 shadowPos = gShadowMat * vec4(posW.xyz,1);
	//float shadow = CalcShadowFactor(shadowPos);
	FragmentColor = pow(outColor.xyz, vec3(1.0 / 2.2));
	FragmentNormal = normalW.xyz;
	FragmentRoughMetal = vec2(roughness, 0.0f);
	FragmentMatID = 0;

	//FragmentEmissive = vec3(0);
	FragmentGlow = vec4(0);
}
#end_shader

#new_shader control
#version 430
layout (vertices = 4) out;

uniform vec4 tessFactor;

in vec3 posW_CS_in[];
in vec3 normalW_CS_in[];
in vec2 texCoord_CS_in[];
in vec2 texelCoord_CS_in[];

out vec3 posW_EV_in[];
out vec3 normalW_EV_in[];
out vec2 texCoord_EV_in[];
out vec2 texelCoord_EV_in[];

layout(std140, binding = 0) uniform SceneInputs
{
	mat4 g_ViewProj;
	vec4 g_CamPos;
};

void main()
{
	// Set output
	posW_EV_in[gl_InvocationID] = posW_CS_in[gl_InvocationID];
	normalW_EV_in[gl_InvocationID] = normalW_CS_in[gl_InvocationID];
	texCoord_EV_in[gl_InvocationID] = texCoord_CS_in[gl_InvocationID];
	texelCoord_EV_in[gl_InvocationID] = texelCoord_CS_in[gl_InvocationID];

		// Calculate the tessellation factors here
		//float distanceFromEye = length(posW_CS_in[gl_InvocationID] - eyePosition);
		//int tessFactor = int(max(16-(distanceFromEye), 1));
		//distanceFromEye = abs(posW_CS_in[gl_InvocationID + 3] - eyePosition);
		//int nextTessFactor = int(max(16-(distanceFromEye), 1));
	//float dist = length(g_CamPos.xyz - posW_EV_in[gl_InvocationID]);

	//float tess = ( 1.0 - (dist / 512.0)) * tessFactor.x;

	//tess = max(tess, 1.0);

	gl_TessLevelOuter[0] = tessFactor.x;
	gl_TessLevelOuter[1] = tessFactor.x;
	gl_TessLevelOuter[2] = tessFactor.x;
	gl_TessLevelOuter[3] = tessFactor.x;

	gl_TessLevelInner[0] = tessFactor.x;
	gl_TessLevelInner[1] = tessFactor.x;

}

#end_shader

#new_shader evaluation
#version 430
layout(quads) in;
layout(std140, binding = 0) uniform SceneInputs
{
	mat4 g_ViewProj;
	vec4 g_CamPos;
};
layout (location = 0, binding = 0) uniform sampler2D heightMap;
layout (location = 1, binding = 1) uniform sampler2D normalMap;
layout (location = 2, binding = 2) uniform sampler2D testTex;
uniform float heightScale;
uniform float horizontalScale;
uniform int heightmapRepeat;
uniform int mapX;
uniform int mapZ;

in vec3 posW_EV_in[];
in vec3 normalW_EV_in[];
in vec2 texCoord_EV_in[];
in vec2 texelCoord_EV_in[];

out vec3 posW;
out vec3 normalW;
out vec2 texCoord;
out vec2 texelCoord;

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y)
	* v1 + vec3(gl_TessCoord.z) * v2;
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y)
	* v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolatePosition()
{
	vec3 v1 = mix(posW_EV_in[0], posW_EV_in[1], gl_TessCoord.x);
	vec3 v2 = mix(posW_EV_in[2], posW_EV_in[3], gl_TessCoord.x);
	return mix(v1,v2,gl_TessCoord.y);
}

vec3 interpolateNormal()
{
	vec3 v1 = mix(normalW_EV_in[0], normalW_EV_in[1], gl_TessCoord.x);
	vec3 v2 = mix(normalW_EV_in[2], normalW_EV_in[3], gl_TessCoord.x);
	return mix(v1,v2,gl_TessCoord.y);
}

vec2 inerpolateTexCoord()
{
	vec2 v1 = mix(texCoord_EV_in[0], texCoord_EV_in[1], gl_TessCoord.x);
	vec2 v2 = mix(texCoord_EV_in[2], texCoord_EV_in[3], gl_TessCoord.x);
	return mix(v1,v2,gl_TessCoord.y);
}
vec3 CalcBumpedNormal(in vec3 inNormal, in vec3 inTangent,in vec2 inTex){
	vec3 normal = normalize(inNormal);
	vec3 tangent = normalize(inTangent);
	tangent = normalize(tangent - dot(tangent,normal) * normal);
	vec3 bitangent = cross(tangent,normal);

	vec3 bump = texture(normalMap,inTex * heightmapRepeat).xyz;
	bump = (bump * 2.0) - 1.0;

	mat3 TBN = mat3(tangent,bitangent,normal);
	vec3 newNormal = TBN * bump;
	return normalize(newNormal);
}

void main()
{
	// Interpolate the attributes of the output vertex using the barycentric coordinates
	posW = interpolatePosition(); //interpolate3D(posW_EV_in[0], posW_EV_in[1], posW_EV_in[2]);
	normalW = interpolateNormal(); // interpolate3D(normalW_EV_in[0], normalW_EV_in[1], normalW_EV_in[2]);
	normalW = normalize(normalW);
	texCoord = inerpolateTexCoord(); // interpolate2D(texCoord_EV_in[0], texCoord_EV_in[1], texCoord_EV_in[2]);
	texelCoord = interpolate2D(texelCoord_EV_in[0], texelCoord_EV_in[1], texelCoord_EV_in[2]);

	vec2 hmcoord = vec2(texCoord.x, texCoord.y);
	posW.y = (heightScale * texture( heightMap, hmcoord).r) - (heightScale * 0.5);
	//posW += normalW;

	//float t,b,l,r,tl,tr,bl,br;
	//float dX = 5.0f / mapX;
	//float dZ = 5.0f / mapZ;
//	dX = 0.01f;
//	dZ = 0.01f;

	//float heightOffset = texture( heightMap, (texCoord.st) * heightmapRepeat).r;

	//bl = texture( heightMap, vec2(-dX,-dZ) + texCoord.st ).r;
	//l  = texture( heightMap, vec2(-dX,  0) + texCoord.st ).r;
	//tl = texture( heightMap, vec2(-dX, dZ) + texCoord.st ).r;
	//t  = texture( heightMap, vec2(  0, dZ) + texCoord.st ).r;
	//tr = texture( heightMap, vec2( dX, dZ) + texCoord.st ).r;
	//r  = texture( heightMap, vec2( dX,  0) + texCoord.st ).r;
	//br = texture( heightMap, vec2( dX,-dZ) + texCoord.st ).r;
	//b  = texture( heightMap, vec2(  0 -dZ) + texCoord.st ).r;

	// New heightmap calculation



// ------------------------------------------------
// Broken code for calculating normals.
// Leaving it in in case I want to fix it later.

//	vec3 botToTop = normalize(vec3(0,t-b,1));
//	vec3 leftToRight = normalize(vec3(1,r-l,0));
//	vec3 newNormal = -cross(botToTop, leftToRight);
//	normalW = normalize(newNormal);
// ------------------------------------------------

// 	Removed during debug - should be put back afterwards
	//heightOffset = heightOffset + bl + l + tl + tr + r + br + b + t;
	//heightOffset /= 9;


	//posW.y += heightScale * ((heightOffset * 255)-128);


//	ivec2 tmp = ivec2(int(texelCoord.s), int(texelCoord.t));
//	vec4 colorVal = texelFetch(heightMap, tmp, 0)
//	              + texelFetch(heightMap, tmp + ivec2(1,0), 0)
//	              + texelFetch(heightMap, tmp + ivec2(0,1), 0)
//	              + texelFetch(heightMap, tmp + ivec2(1,1), 0);
//    colorVal = colorVal / 4.0f;
//	posW.y += heightScale * ((colorVal.y * 255) - 128);
	// posW.y += colorVal.x;

	vec3 norm = texture(normalMap, texCoord).xyz;
	//norm = texture(normalMap, texCoord).rgb;

	// TODOMW : Remove magic numbers when a proper heightmap is in place
	//norm.x = heightScale * norm.x;
	//norm.z = heightScale * norm.z;
	//norm.y = horizontalScale * norm.y;

	normalW = normalize(norm);


	gl_Position = g_ViewProj * vec4(posW, 1.0f);
}

#end_shader
