#version 430

uniform float 	g_DeltaTime;
uniform uint 	g_BlockOffset;
uniform uint 	g_BlockSize;
uniform uint 	g_Offset;

struct Particle
{
	vec3 	Position;
	float 	TimeToLive;
	vec3 	Direction;
	float 	Speed;
};

struct ParticleSystem
{
	vec3 	Position;
	float 	TimeToLive;

	vec4 	Colour;

	vec3 	Direction;
	float 	ParticlesSpeed;

	vec2 	Size;
	float 	ParticlesTimeToLive;
	float 	DoDraw; // Will be 1.0 for true, 0.0 for false
	vec2 	TransparencyPolynome;
	vec2 	CullBoxSize;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
	ParticleSystem g_Systems[4096]; // Needs to be same on CPU side
	Particle g_Particles[];
};

ParticleSystem GetParticleSystem( uint invocationID )
{
	// Local id
	uint inBlock = uint(floor(invocationID / g_BlockSize));
	uint block = g_BlockOffset + inBlock;
	ParticleSystem particleSystem = g_Systems[block];
	return particleSystem;
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return ((f - 1.0) * 2) - 1.0;          // Range [-1:1]
}

// Pseudo-random value in half-open range [-1:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
