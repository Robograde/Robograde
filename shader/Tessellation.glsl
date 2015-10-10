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
layout (binding = 2) uniform sampler2D testTex;
layout (binding = 3) uniform sampler2D roughTex;
layout (binding = 4) uniform sampler2D FogOfWarTex;
layout (binding = 5) uniform sampler2DShadow g_ShadowMap;
layout (binding = 7) uniform samplerCube g_LightCubeTex;
layout (binding = 8) uniform samplerCube g_IrradianceCube;
uniform vec3 color;
uniform bool hasTexture;
uniform vec3 eyepos;
uniform vec2 g_WorldSize;
uniform int textureRepeat;
uniform uint gNumTiles;
uniform uint gTileSize;
uniform float heightScale;
uniform mat4 gShadowMat;

struct SurfaceMaterial{
	vec4	BaseColor;
	vec4	SpecularColor;
	ivec4	AlbedoNormalRoughMetalTexture;
};

#include LightStruct.txt

#include LightingFunc.txt


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

out vec4 outColor;

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
void main(){
	// vec3 norm = (normalW + 1) * 0.5f;

	// outColor = vec4(texture(normalMap	, texCoord).rgb, 1.0f);

	//outColor = vec4(normalW, 1.0f);

	//vec3 color = texture( normalMap, texCoord ).rgb;

	float roughness = 1.0f;
	//ndotl += dot(normalW,normalize(vec3(0.5f,0.2f,0.0f))) * 0.7f + 0.0f;
	if(hasTexture)
	{
		vec2 tmpTC = texCoord * textureRepeat;
		vec4 tempColor = pow(texture(testTex, tmpTC), vec4(2.2));
		float strength = 1.0f - ((color.x + color.y + color.z) / 3.0f);
		outColor = tempColor * vec4(color,1); //		clamp(vec4(mix(color, tempColor, strength), 1),0.0f,1.0f);
		roughness =  clamp( texture(roughTex, texCoord).r, 0.03, 1.0);
	}
	else
	{
		outColor = clamp(vec4(color, 1),0.0f,1.0f);
	}
	vec4 lightcolor = vec4(0);
	SurfaceMaterial mat;
	mat.BaseColor = vec4(1);
	mat.SpecularColor = vec4(0.4);
	//light tiles
	uvec2 tile = uvec2(gl_FragCoord.xy / gTileSize);
	uint tileindex = tile.x + (tile.y * gNumTiles);
	uint pointLightCount = gLightSizes[tileindex];
	tileindex = tile.x * MAX_LIGHTS_PER_TILE + (tile.y * gNumTiles * MAX_LIGHTS_PER_TILE);

	for(int i = 0; i < numPLights;i++)
	{
		Light p = lights[i];
		lightcolor += CalcPLight(p, normalW, posW.xyz, g_CamPos.xyz, mat, roughness , 0);
	}
	for(int i = 0; i < numDLights;i++)
	{
		Light d = lights[MAX_POINTLIGHTS + i];
		lightcolor += CalcDLight(d, normalW, posW.xyz, g_CamPos.xyz, mat, roughness, 0);
	}
	//IBL
	vec3 toeye = normalize(g_CamPos.xyz - posW.xyz);
	vec3 reflected = reflect(-toeye,normalW);

	ivec2 texDim = textureSize(g_LightCubeTex,0);
	float numMips = ceil(log2(max(texDim.x,texDim.y))) - 1.0f;
	float mipLevel = numMips * roughness;
	vec4 iblSpec = textureLod(g_LightCubeTex, reflected, mipLevel);
	iblSpec = pow(iblSpec, vec4(2.2));

	texDim = textureSize(g_IrradianceCube,0);
	numMips = ceil(log2(max(texDim.x,texDim.y))) - 1.0f;
	mipLevel = numMips * roughness;
	vec4 iblDiff = textureLod(g_IrradianceCube, normalW, mipLevel);
	iblDiff = pow(iblDiff, vec4(2.2));

	float hdotl = clamp(dot(normalize(normalW + toeye),normalW),0,1);
	iblSpec *= vec4(FresnelSchlick(hdotl,mat.SpecularColor.xyz),1);
	vec4 iblColor = mix(iblSpec,iblDiff,roughness);
	//outColor *= iblColor;

	vec2 fogTexCoord = vec2( posW.x / g_WorldSize.x, posW.z / g_WorldSize.y );
	float fog = clamp( 1.0f - texture(FogOfWarTex, fogTexCoord).a, 0.0f, 1.0f );
	vec4 shadowPos = gShadowMat * vec4(posW,1);
	float shadow = CalcShadowFactor(shadowPos);
	lightcolor = clamp((lightcolor + iblColor) * fog, 0.0f, 1.0f) * shadow;

	outColor = pow(outColor * lightcolor, vec4(1.0 / 2.2));
	//vec4 colorFlags = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	////////// texelFetch vs texture vs CPU-height ////////////////
/*
	if(posW.z < 170)
	{
		ivec2 tmp = ivec2(texelCoord.s, texelCoord.t);
		vec4 colorVal = texelFetch(heightMap, tmp, 0);
		float finalOffset = heightScale * ((colorVal.x * 255.0f) - 128.0f);
		float diff = 128.0f + finalOffset;
		colorFlags.x = diff / 255.0f;
		colorFlags.z = diff / 10.0f;
	}
	else if(posW.z < 340)
	{
		float heightOffset = texture( heightMap, texCoord.st).g;
		float finalOffset = ((heightOffset * 255.0f) - 128.0f);
		float diff = 128.0f + finalOffset;
		colorFlags.x = diff / 255.0f;
		colorFlags.z = diff / 10.0f;
	}
	else
	{
		float diff = 128.0f + posW.y;
		colorFlags.x = diff / 255.0f;
		colorFlags.z = diff / 10.0f;
	}
*/

////////////// Comparing r,g,b ////////////////////
/*
if(posW.z < 170)
{
	float heightOffset = texture( heightMap, texCoord.st).r;
	float finalOffset = ((heightOffset * 255.0f) - 128.0f);
	float diff = 128.0f + finalOffset;
	colorFlags.x = diff / 255.0f;
	colorFlags.z = diff / 10.0f;
}
else if(posW.z < 340)
{
	float heightOffset = texture( heightMap, texCoord.st).g;
	float finalOffset = ((heightOffset * 255.0f) - 128.0f);
	float diff = 128.0f + finalOffset;
	colorFlags.x = diff / 255.0f;
	colorFlags.z = diff / 10.0f;
}
else
{
	float heightOffset = texture( heightMap, texCoord.st).b;
	float finalOffset = ((heightOffset * 255.0f) - 128.0f);
	float diff = 128.0f + finalOffset;
	colorFlags.x = diff / 255.0f;
	colorFlags.z = diff / 10.0f;
}
*/


////////// 1, 2, 4  on r,g, CPU /////////////////////
/*
if(posW.z < 170)
{
	float heightOffset = texture( heightMap, texCoord.st).r;
	float finalOffset = ((heightOffset * 255.0f) - 128.0f);
	float diff = 128.0f + finalOffset;
	if(diff > 0.9f && diff < 1.1f)
	{
		colorFlags.x = 1.0f;
	}
	else if(diff > 1.9f && diff < 2.1f)
	{
		colorFlags.y = 1.0f;
	}
	else if(diff > 3.9f && diff < 4.1f)
	{
		colorFlags.z = 1.0f;
	}
}
else if(posW.z < 340)
{
	float heightOffset = texture( heightMap, texCoord.st).g;
	float finalOffset = ((heightOffset * 255.0f) - 128.0f);
	float diff = 128.0f + finalOffset;
	if(diff > 0.9f && diff < 1.1f)
	{
		colorFlags.x = 1.0f;
	}
	else if(diff > 1.9f && diff < 2.1f)
	{
		colorFlags.y = 1.0f;
	}
	else if(diff > 3.9f && diff < 4.1f)
	{
		colorFlags.z = 1.0f;
	}
}
else
{
	float diff = 128.0f + posW.y;
	if(diff > 0.9f && diff < 1.1f)
	{
		colorFlags.x = 1.0f;
	}
	else if(diff > 1.9f && diff < 2.1f)
	{
		colorFlags.y = 1.0f;
	}
	else if(diff > 3.9f && diff < 4.1f)
	{
		colorFlags.z = 1.0f;
	}
}
*/

//outColor = colorFlags;

	//if(colorVal.x > 0.5f)
	//{
	//	outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//}

	//float lerp = 1.0 - ((pointLightCount / 64.0f));
	//outColor = mix(vec4(1,0.5f,0,1),vec4(0,0.5f,1,1),lerp);

	//outColor = vec4(normalW * 0.5 + 0.5, 1);
	//outColor = vec4((color + vec3((posW.y/128))*1.0)*0.5, 1.0f);
}
#end_shader

#new_shader control
#version 430
layout (vertices = 3) out;

uniform vec4 tessFactor;

in vec3 posW_CS_in[];
in vec3 normalW_CS_in[];
in vec2 texCoord_CS_in[];
in vec2 texelCoord_CS_in[];

out vec3 posW_EV_in[];
out vec3 normalW_EV_in[];
out vec2 texCoord_EV_in[];
out vec2 texelCoord_EV_in[];

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

		gl_TessLevelOuter[0] = tessFactor.x;
		gl_TessLevelOuter[1] = tessFactor.y;
		gl_TessLevelOuter[2] = tessFactor.z;
		gl_TessLevelInner[0] = tessFactor.w;

}

#end_shader

#new_shader evaluation
#version 430
layout(triangles, equal_spacing, cw) in;
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
	posW = interpolate3D(posW_EV_in[0], posW_EV_in[1], posW_EV_in[2]);
	normalW = interpolate3D(normalW_EV_in[0], normalW_EV_in[1], normalW_EV_in[2]);
	normalW = normalize(normalW);
	texCoord = interpolate2D(texCoord_EV_in[0], texCoord_EV_in[1], texCoord_EV_in[2]);
	texelCoord = interpolate2D(texelCoord_EV_in[0], texelCoord_EV_in[1], texelCoord_EV_in[2]);
	//posW += normalW;

	float t,b,l,r,tl,tr,bl,br;
	float dX = 5.0f / mapX;
	float dZ = 5.0f / mapZ;
//	dX = 0.01f;
//	dZ = 0.01f;

	float heightOffset = texture( heightMap, (texCoord.st) * heightmapRepeat).r;

	bl = texture( heightMap, vec2(-dX,-dZ) + texCoord.st ).r;
	l  = texture( heightMap, vec2(-dX,  0) + texCoord.st ).r;
	tl = texture( heightMap, vec2(-dX, dZ) + texCoord.st ).r;
	t  = texture( heightMap, vec2(  0, dZ) + texCoord.st ).r;
	tr = texture( heightMap, vec2( dX, dZ) + texCoord.st ).r;
	r  = texture( heightMap, vec2( dX,  0) + texCoord.st ).r;
	br = texture( heightMap, vec2( dX,-dZ) + texCoord.st ).r;
	b  = texture( heightMap, vec2(  0 -dZ) + texCoord.st ).r;

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
	heightOffset = heightOffset + bl + l + tl + tr + r + br + b + t;
	heightOffset /= 9;


//	posW.y += heightScale * ((heightOffset * 255)-128);


//	ivec2 tmp = ivec2(int(texelCoord.s), int(texelCoord.t));
//	vec4 colorVal = texelFetch(heightMap, tmp, 0)
//	              + texelFetch(heightMap, tmp + ivec2(1,0), 0)
//	              + texelFetch(heightMap, tmp + ivec2(0,1), 0)
//	              + texelFetch(heightMap, tmp + ivec2(1,1), 0);
//    colorVal = colorVal / 4.0f;
//	posW.y += heightScale * ((colorVal.y * 255) - 128);
	// posW.y += colorVal.x;

	vec3 norm = texture(normalMap, texCoord).xzy * 2 - 1;
	//norm = texture(normalMap, texCoord).rgb;

	// TODOMW : Remove magic numbers when a proper heightmap is in place
	norm.x = heightScale * norm.x;
	norm.z = heightScale * norm.z;
	norm.y = horizontalScale * norm.y;

	normalW = normalize(norm);


	gl_Position = g_ViewProj * vec4(posW, 1.0f);
}

#end_shader
