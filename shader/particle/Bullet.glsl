#new_shader compute
#include ParticleCommon.glsl
#include InitialFunctions.glsl

layout (local_size_x = 64) in;
void main()
{
	uint i = g_Offset + gl_GlobalInvocationID.x;
	Particle particle = g_Particles[i];
	particle.TimeToLive -= g_DeltaTime;
	if ( particle.TimeToLive < 0.0 )
	{
		BulletSpawn( i, gl_GlobalInvocationID.x );
	}
	else
	{
		ParticleSystem particleSystem = GetParticleSystem( gl_GlobalInvocationID.x );
		particle.Position += particle.Direction * g_DeltaTime * particle.Speed;
		g_Particles[i] = particle;
	}
}
#end_shader
