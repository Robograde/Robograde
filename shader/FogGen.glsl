#new_shader compute
#version 430
layout(local_size_x = 32, local_size_y = 32) in;

struct VisionGiver
{
	vec3	Position;
	float	Range;
};

layout (std140, binding = 11) buffer VisionBuffer
{
	VisionGiver g_VisionGivers[9216];
};

layout(binding = 0, rgba8) writeonly uniform image2D g_FogTex;
uniform uint 	g_VisionGiverCount;
uniform vec2	g_TileSize;

void main()
{
	ivec2 threadIndex = ivec2( gl_GlobalInvocationID.xy );
	vec2 tilePos = vec2( (threadIndex.x + 0.5f) * g_TileSize.x, (threadIndex.y + 0.5f) * g_TileSize.y );
	
	float fogValue = 0.8f;
	uint loopCount = g_VisionGiverCount;
	for ( uint i = 0; i < loopCount; ++i )
	{
		VisionGiver visionGiver = g_VisionGivers[i];
		float distance = length( tilePos - vec2( visionGiver.Position.x, visionGiver.Position.z ) );
		if ( distance <= visionGiver.Range )
		{
			fogValue = 0.0f;
		}
	}
	
	imageStore( g_FogTex, threadIndex, vec4( 0.0f, 0.0f, 0.0f, fogValue ) );
}

#end_shader