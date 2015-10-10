#new_shader vertex
#include ParticleCommon.glsl

uniform mat4 g_World;
uniform mat4 g_ProjViewWorld;

layout (location = 0) in vec3 vPosL;
layout (location = 1) in float vTimeToLive;

out vec3 gPosW;
out float gParticleSystemTimeToLive;
out float gDoDraw;
out vec4 gColour;
out vec2 gSize;

void main( )
{
	gl_Position	= vec4( vPosL, 1.0 ) * g_ProjViewWorld;
	gPosW 		= ( vec4( vPosL, 1.0 ) * g_World ).xyz;
	
	uint inBlock = uint(floor((gl_VertexID - g_Offset) / g_BlockSize));
	ParticleSystem particleSystem = g_Systems[g_BlockOffset + inBlock];
	gParticleSystemTimeToLive = particleSystem.TimeToLive;

	float x = 1.0 - vTimeToLive / particleSystem.ParticlesTimeToLive;
	float transparency = particleSystem.TransparencyPolynome.x * x * x + particleSystem.TransparencyPolynome.y * x + particleSystem.Colour.w;
	gColour = vec4(particleSystem.Colour.xyz, particleSystem.Colour.w * transparency);
	gSize = particleSystem.Size;
	gDoDraw = particleSystem.DoDraw;
}

#end_shader

#new_shader geometry
#version 430

uniform vec3 g_CameraPosition;
uniform mat4 g_ProjViewWorld;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 gPosW[];
in float gParticleSystemTimeToLive[];
in float gDoDraw[];
in vec4 gColour[];
in vec2 gSize[];

out vec2 fTexCoord;
out vec4 fColour;

void main( )
{
	if ( gParticleSystemTimeToLive[0] >= 0.0 && (gDoDraw[0] == 1.0 || gDoDraw[0] == 2.0))
	{
		vec3 pos = gPosW[0];

		vec3 look 	= normalize( g_CameraPosition - pos );
		vec3 right 	= normalize( cross( vec3( 0, 1, 0 ), look ) );
		vec3 up 	= cross( right, look );

		float halfWidth 	= gSize[0].x;
		float halfHeight 	= gSize[0].y;

		gl_Position = g_ProjViewWorld * vec4( pos - halfWidth * right - halfHeight * up, 1.0 );
		fTexCoord = vec2( 1.0, 1.0 );
		fColour = gColour[0];
		EmitVertex();

		gl_Position = g_ProjViewWorld * vec4( pos - halfWidth * right + halfHeight * up, 1.0 );
		fTexCoord = vec2( 0.0, 1.0 );
		EmitVertex();

		gl_Position = g_ProjViewWorld * vec4( pos + halfWidth * right - halfHeight * up, 1.0 );
		fTexCoord = vec2( 1.0, 0.0 );
		EmitVertex();

		gl_Position = g_ProjViewWorld * vec4( pos + halfWidth * right + halfHeight * up, 1.0 );
		fTexCoord = vec2( 0.0, 0.0 ); 
		EmitVertex();

		EndPrimitive();
	}
}

#end_shader

#new_shader fragment
#version 430

layout (location = 0) uniform sampler2D g_DepthBuffer;
layout (location = 1) uniform sampler2D g_Texture;
layout (location = 2) uniform float g_Width;
layout (location = 3) uniform float g_Height;
layout (location = 4) uniform float g_NearPlane;
layout (location = 5) uniform float g_FarPlane;

in vec2 fTexCoord;
in vec4 fColour;

layout(location = 0) out vec4 oDiffuseColor;

void main( )
{
	float f = g_FarPlane;
	float n = g_NearPlane;
	vec2 fragTextureCoord = vec2(gl_FragCoord.x / g_Width, gl_FragCoord.y / g_Height);
	// Linearize depth values
	float scenedepth = (2 * n) / (f + n - texture2D( g_DepthBuffer, fragTextureCoord.xy ).r * (f - n));
	float fragdepth = (2 * n) / (f + n - gl_FragCoord.z * (f - n));
	
	float diff = abs(scenedepth - fragdepth);

	if ( scenedepth < fragdepth )
	{
		// Manual depth test discarding
		discard;
	}
	else if ( diff < 0.01 )
	{
		// Blend depending on differences
		oDiffuseColor = fColour * texture(g_Texture, fTexCoord) * vec4(1, 1, 1, diff * 100.0);
	}
	else
	{
		// Just draw it normally
		oDiffuseColor = fColour * texture(g_Texture, fTexCoord);
	}
}
#end_shader
