void ExplosionSpawn( uint i, uint indexInBlock )
{
	// TODOJM: Fetch per thread group?
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );

	Particle particle;
	particle.Position 	= particleSystem.Position + 0.5 * vec3(random(float(i)) / 5.0, random(float(i)/1.2) / 5.0, random(float(i)/1.3));;
	particle.Direction 	= vec3(0.2*random(float(i)/1.0), random(float(i)/1.0+1) + 1.0, 0.2*random(float(i)/1.0+2));
	particle.TimeToLive = particleSystem.ParticlesTimeToLive;
	particle.Speed 		= particleSystem.ParticlesSpeed + random(float(i)/3.5) * particleSystem.ParticlesSpeed / 5.0;
	g_Particles[i] 		= particle;
}

void LaserSpawn( uint i, uint indexInBlock )
{
	// TODOJM: Fetch per thread group?
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );

	Particle particle;
	particle.Position 	= particleSystem.Position + particleSystem.Direction * float(indexInBlock) / g_BlockSize;
	particle.Direction 	= particleSystem.Direction;
	particle.TimeToLive = particleSystem.ParticlesTimeToLive;
	particle.Speed 		= 0.0f;
	g_Particles[i] 		= particle;
}

void SmokeCloudSpawn( uint i, uint indexInBlock )
{
	// TODOJM: Fetch per thread group?
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );

	Particle particle;
	particle.Position 	= particleSystem.Position + 2.0 * vec3(random(float(i)/1.0), random(float(i)/1.0+1), random(float(i)/1.0+2));
	particle.Direction 	= normalize(vec3(random(float(i)/1.0), random(float(i)/1.0+1), random(float(i)/1.0+2))) / 10.0;
	particle.TimeToLive = particleSystem.ParticlesTimeToLive + particleSystem.ParticlesTimeToLive * 0.3 * random(float(i)/1.4);
	particle.Speed 		= particleSystem.ParticlesSpeed + random(float(i)/3.5) * particleSystem.ParticlesSpeed / 5.0;
	g_Particles[i] 		= particle;
}

void SmokeTrailSpawn( uint i, uint indexInBlock )
{
	// TODOJM: Fetch per thread group?
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );

	Particle particle;
	particle.Position		= particleSystem.Position;
	particle.Direction 	= normalize(vec3(random(float(i)/1.0), random(float(i)/1.0+1), random(float(i)/1.0+2)));
	particle.TimeToLive	= particleSystem.ParticlesTimeToLive *  random(float(i)/1.337);
	particle.Speed 		= particleSystem.ParticlesSpeed;
	g_Particles[i]			= particle;
}

void FireBallSpawn( uint i, uint indexInBlock )
{
	// TODOJM: Fetch per thread group?
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );
	
	Particle particle;
	particle.Position 	= particleSystem.Position + 0.5 * vec3(random(float(i)) / 5.0, random(float(i)/1.2) / 5.0, random(float(i)/1.3));
	particle.Direction 	= normalize( vec3( sin((float(indexInBlock) / 64.0) * 6.28), 0, cos((float(indexInBlock) / 64.0) * 6.28) ) );
	particle.TimeToLive	= particleSystem.ParticlesTimeToLive;
	particle.Speed 		= particleSystem.ParticlesSpeed + random(float(i)/3.5) * particleSystem.ParticlesSpeed / 5.0;
	g_Particles[i]			= particle;
}

void ConeSpawn( uint i, uint indexInBlock )
{
	// TODOJM: Fetch per thread group?
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );
	
	vec3 origin	= particleSystem.Position;
	vec3 dest		= particleSystem.Position + particleSystem.Direction;
	vec3 normal	= normalize(vec3( dest.z - origin.z, 0.0f, origin.x - dest.x ));
	
	Particle particle;
	particle.Position		= particleSystem.Position;
	particle.Direction 	= particleSystem.Direction + (normal * random(float(i)*14.456f) * particleSystem.ParticlesSpeed);
	particle.TimeToLive	= particleSystem.ParticlesTimeToLive * (1.0f + random(float(i)+22.3f)) / 2.0f;
	particle.Speed 		= 1.0f / particle.TimeToLive;
	g_Particles[i]			= particle;
}

void BulletSpawn( uint i, uint indexInBlock )
{
	ParticleSystem particleSystem = GetParticleSystem( indexInBlock );
	
	vec3 origin	= particleSystem.Position;
	vec3 normal	= particleSystem.Direction;
	
	Particle particle;
	particle.Position	= particleSystem.Position + particleSystem.Direction * (float(indexInBlock)/(20.0f));
	particle.Direction 	= particleSystem.Direction;
	particle.TimeToLive	= particleSystem.ParticlesTimeToLive;
	particle.Speed 		= particleSystem.ParticlesSpeed;
	g_Particles[i]		= particle;
}
