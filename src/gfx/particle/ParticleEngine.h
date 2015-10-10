/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <memory/Alloc.h>
#include "../GFXLibraryDefine.h"
#include "gfx/Shader/ShaderBank.h"
#include "ParticleBucket.h"
#include <gfx/Texture.h>
#include <gfx/DepthBuffer.h>

#define PARTICLE_ENGINE_MAX_NR_OF_PARTICLES 400000
// Needs to be same in shader
#define PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS 4096

namespace gfx
{
typedef unsigned int ParticleTypeIdentifier;
class ParticleEngine
{
public:
	GFX_API ~ParticleEngine();
	GFX_API void Initialize();
	GFX_API void Shutdown();
	GFX_API void Update( float deltaTime );
	GFX_API void Render(
			const glm::mat4& projViewWorld, const glm::vec3& cameraPosition, const GLuint depthBuffer,
		   	float nearPlane, float farPlane, float width, float height);
	GFX_API ParticleTypeIdentifier CreateParticleType( const ParticleTypeSettings& bucketSettings );
	GFX_API ParticleSystem* Spawn( ParticleTypeIdentifier particleType, const ParticleSystem& particleSystem );
	GFX_API const ParticleSystem& GetDefaultParticleSystem( ParticleTypeIdentifier particleType ) const;
	GFX_API ParticleBucket::BucketUpdateInfo GetParticleBucketUpdateInfo( ParticleTypeIdentifier particleType ) const;

	GFX_API struct Particle
	{
		glm::vec3 	Position;
		float 		TimeToLive;
		glm::vec3 	Direction;
		float 		Speed;
	};

private:
	// Buffer for storing all the particles, memory will be managed in blocks and buckets. Buckets contain blocks
	rString 			m_ParticleBuffer 	= "ParticleBuffer";
	GLuint 				m_ParticleBufferHandle = 0;
	GLuint 				m_ParticleVertexArrayHandle = 0;

	rVector<ParticleBucket*> 	m_ParticleBuckets;
	rVector<ParticleSystem> 	m_ParticleSystems;

	gfx::ShaderProgram*			m_FXDraw = nullptr;
	ParticleBucket::ParticleBucketSettings 		m_CurrentParticleBucketSettings;
	
};
}
