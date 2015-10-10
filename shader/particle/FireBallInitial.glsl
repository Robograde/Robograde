#new_shader compute
#include ParticleCommon.glsl

uniform uint g_Start;
uniform uint g_End;

#include InitialFunctions.glsl

layout (local_size_x = 64) in;
void main()
{
	uint i = g_Start + gl_GlobalInvocationID.x;
	if ( i < g_End )
	{
		FireBallSpawn( i, gl_GlobalInvocationID.x );
	}
}
#end_shader
