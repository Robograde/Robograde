#new_shader compute
#version 430
layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rgba8) writeonly restrict uniform image2D g_FogTex;

shared float g_VisibilityBlock[1296]; // (32 + 4) * (32 + 4)
uniform sampler2D g_GenFogTex;
void main()
{
	ivec2 threadIndex	= ivec2( gl_GlobalInvocationID.xy );
	ivec2 groupIndex	= ivec2( gl_WorkGroupID.xy );
	//float fog 			= texelFetch( g_FogTex, threadIndex).w;
	float sumFog	 	= 0.0f;
	ivec2 firstIndex	= groupIndex * 32 - ivec2(2);
	if ( gl_LocalInvocationIndex == 0 )
	{
		ivec2 imageSize = textureSize(g_GenFogTex, 0);
		vec2 denominator = 1.0 / vec2(imageSize);
		for ( int y = 0; y < 36; ++y )
		{
			for ( int x = 0; x < 36; ++x )
			{
				g_VisibilityBlock[ y * 36 + x ] = texture( g_GenFogTex, ivec2( clamp( firstIndex.x + x, 0, imageSize.x - 1 ),
				 clamp( firstIndex.y + y, 0, imageSize.y - 1 ) ) * denominator ).a;
			}
		}
	}

	barrier();

	ivec2 visibilityBlockIndex;
	for ( int y = -2; y <= 2; ++y )
	{
		for ( int x = -2; x <= 2; ++x )
		{
			visibilityBlockIndex = threadIndex + ivec2(x,y) - firstIndex;
			sumFog += g_VisibilityBlock[ visibilityBlockIndex.y * 36 + visibilityBlockIndex.x ];
		}
	}
	imageStore( g_FogTex, threadIndex, vec4( 0.0f, 0.0f, 0.0f, sumFog / 25.0f ) );
}

#end_shader
