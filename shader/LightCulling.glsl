#new_shader compute
#version 430
#define WORK_GROUP_SIZE 16
layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE) in;

struct Light
{
	vec3 Position;
	float Range;
	vec3 Direction;
	float Intensity;
	vec4 Color;
};
#define MAX_POINTLIGHTS 4096
#define MAX_DIRLIGHTS 4
#define MAX_NUM_LIGHTS MAX_POINTLIGHTS + MAX_DIRLIGHTS
#define MAX_LIGHTS_PER_TILE 256
#define LIGHT_SIZES 8192

layout(std430, binding = 3) readonly buffer LightBuffer
{
	Light lights[MAX_NUM_LIGHTS];
};

layout(std430, binding = 5) buffer LightIndexBuffer
{
	uint gLightSizes[LIGHT_SIZES];
	uint gLightIndices[];
};
uniform sampler2D DepthBuffer;
uniform vec2 gScreenSize;
uniform uint gTotalLightCount;
//Matrices
uniform mat4 gProj;
uniform mat4 gView;
uniform mat4 gInvProjView;

shared uint gMaxDepth;
shared uint gMinDepth;
shared uint gPointLightCount;

void main()
{
	if(gl_LocalInvocationIndex == 0)
	{
		gMaxDepth = 0xFFFFFFFF;
		gMinDepth = 0;
		gPointLightCount = 0;
	}
	barrier(); //sync

	ivec2 screenPos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(screenPos) / vec2(gScreenSize - 1);
	float depth = texelFetch(DepthBuffer,screenPos.xy,0).r * 2.0f - 1.0f;
	float zNear = gProj[3][2] / (gProj[2][2] - 1.0f);
	float zFar 	= gProj[3][2] / (gProj[2][2] + 1.0f);
	float clipDelta = zFar - zNear;

	//Worldpos
	vec4 sPos = vec4(uv * 2 - 1, depth,1);
	sPos = gInvProjView * sPos;
	vec4 worldPos = vec4(sPos.xyz / sPos.w , sPos.w);

	float linearDepth = (1.0f / worldPos.w - zNear) / clipDelta;
	uint unsignedDepth = uint(linearDepth * uint(0xFFFFFFFF));

	atomicMin(gMinDepth, unsignedDepth);
	atomicMax(gMaxDepth, unsignedDepth);

	barrier(); //sync

	float minDepthZ = (float(gMinDepth) / float(0xFFFFFFFF));
	float maxDepthZ = (float(gMaxDepth) / float(0xFFFFFFFF));

	minDepthZ = minDepthZ * clipDelta + zNear;
	maxDepthZ = maxDepthZ * clipDelta + zNear;

	vec2 tileScale = (gScreenSize / float(2 * WORK_GROUP_SIZE));
	vec2 tileBias = tileScale - vec2(gl_WorkGroupID.xy);

	vec4 col1 = vec4(-gProj[0][0] * tileScale.x , gProj[0][1] 					, tileBias.x, gProj[0][3]);
	vec4 col2 = vec4( gProj[1][0] 				, -gProj[1][1] * tileScale.y 	, tileBias.y, gProj[1][3]);
	vec4 col4 = vec4( gProj[3][0] 				, gProj[3][1] 					, -1.0f 	, gProj[3][3]);

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

	Light currentLight;
	vec4 pos;
	float radius;
	bool inFrustrum = false;
	float dist;
	uint id = 0;
	uint lightIndex = 0;

	uint tileIndex = (gl_WorkGroupID.x * MAX_LIGHTS_PER_TILE) + (gl_WorkGroupID.y * MAX_LIGHTS_PER_TILE * gl_NumWorkGroups.x);
 	// gl_NumWorkGroups.x * gl_WorkGroupID.y + gl_WorkGroupID.x;
	//tileIndex = tileIndex * 64;

	for(uint pass = 0; pass < passCount; ++pass)
	{
		lightIndex = pass * threadCount + gl_LocalInvocationIndex;

		if(lightIndex > gTotalLightCount)
			break;

		currentLight = lights[lightIndex];
		pos =  (gView) * vec4(currentLight.Position,1);
		radius = currentLight.Range;

		if(gPointLightCount < MAX_LIGHTS_PER_TILE)
		{

			inFrustrum = true;
			for(uint i = 5; i >= 0 && inFrustrum; i-- )
			{
				dist = dot(frustrumPlanes[i], pos);
				inFrustrum = (-radius <= dist);
			}
			if(inFrustrum && gPointLightCount < MAX_LIGHTS_PER_TILE)
			{
				id = atomicAdd(gPointLightCount,1);
				gLightIndices[tileIndex + id] = lightIndex;
			}
		}
	}
	barrier();

	if(gl_LocalInvocationIndex == 0)
	{
		uint tile = gl_NumWorkGroups.x * gl_WorkGroupID.y + gl_WorkGroupID.x;
		gLightSizes[tile] = gPointLightCount;
	}
}
#end_shader
