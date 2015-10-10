/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "ParticleEngine.h"
#include "../BufferManager.h"

using namespace gfx;

ParticleEngine::~ParticleEngine()
{
	Shutdown();
}

void ParticleEngine::Initialize()
{
	m_ParticleSystems.resize( PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS );

	// Create buffer to be managed
	m_ParticleBufferHandle = g_BufferManager.CreateBuffer(
			m_ParticleBuffer, GL_SHADER_STORAGE_BUFFER, 
			PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS * sizeof(ParticleSystem) + PARTICLE_ENGINE_MAX_NR_OF_PARTICLES * sizeof(Particle), GL_DYNAMIC_DRAW );
	// Set data (TODOJM: does it need to be run?) --No create buffer allocates memory on the gpu
	//g_BufferManager.UpdateBuffer( m_ParticleBuffer, 0, nullptr,

	glBindBuffer( GL_ARRAY_BUFFER, m_ParticleBufferHandle );
	glGenVertexArrays( 1, &m_ParticleVertexArrayHandle );
	glBindVertexArray( m_ParticleVertexArrayHandle );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glBindVertexBuffer( 0, m_ParticleBufferHandle, PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS * sizeof(ParticleSystem), sizeof(Particle) );
	glVertexAttribFormat( 0, 3, GL_FLOAT, GL_FALSE, offsetof( ParticleSystem, EmitterPosition ) );
	glVertexAttribFormat( 1, 1, GL_FLOAT, GL_FALSE, offsetof( ParticleSystem, TimeToLive ) );
	glVertexAttribBinding( 0, 0 );
	glVertexAttribBinding( 1, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	ShaderProgramHandle handle = g_ShaderBank.LoadShaderProgram( "../../../shader/particle/DrawParticles.glsl" );
	m_FXDraw = g_ShaderBank.GetProgramFromHandle( handle );

	m_CurrentParticleBucketSettings.BufferName = m_ParticleBuffer;
	m_CurrentParticleBucketSettings.ParticleSystemsBuffer = &m_ParticleSystems;
	m_CurrentParticleBucketSettings.Offset = 0;
	m_CurrentParticleBucketSettings.BlockOffset = 0;
}

void ParticleEngine::Shutdown()
{
	for ( auto& bucket : m_ParticleBuckets )
	{
		pDelete(bucket);
	}
	m_ParticleBuckets.clear();
	m_ParticleSystems.clear();
}

void ParticleEngine::Update( float deltaTime )
{
	for ( auto& bucket : m_ParticleBuckets )
	{
		bucket->UpdateSystems( deltaTime );
	}
	g_BufferManager.UpdateBuffer( m_ParticleBuffer, 0, m_ParticleSystems.data(), PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS * sizeof(ParticleSystem) );
	for ( auto& bucket : m_ParticleBuckets )
	{
		bucket->UpdateParticles( deltaTime );
	}
}

void ParticleEngine::Render( const glm::mat4& projViewWorld, const glm::vec3& cameraPosition, const GLuint depthBuffer, float nearPlane, float farPlane, float width, float height )
{
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	m_FXDraw->Apply();

	m_FXDraw->SetUniformMat4( "g_ProjViewWorld", projViewWorld );
	m_FXDraw->SetUniformVec3( "g_CameraPosition", cameraPosition );
	m_FXDraw->SetUniformMat4( "g_World", glm::mat4(1) );

	// For linearizing depths
	m_FXDraw->SetUniformFloat( "g_NearPlane", 	nearPlane );
	m_FXDraw->SetUniformFloat( "g_FarPlane", 	farPlane );
	m_FXDraw->SetUniformFloat( "g_Width", 		width );
	m_FXDraw->SetUniformFloat( "g_Height", 		height );

	glBindVertexArray( m_ParticleVertexArrayHandle );

	g_BufferManager.BindBufferToProgram( m_ParticleBuffer, m_FXDraw, 0 );
	glBindVertexBuffer( 0, m_ParticleBufferHandle, PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS * sizeof(ParticleSystem), sizeof(Particle) );

	glBindTexture( GL_TEXTURE_2D, 0 );
	GLuint depthBufferLocation = m_FXDraw->FetchUniform( "g_DepthBuffer" );
	glUniform1i( depthBufferLocation, 0 );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, depthBuffer );

	for ( auto& bucket : m_ParticleBuckets )
	{
		bucket->DrawParticles( m_FXDraw );
	}

	glBindVertexBuffer( 0, 0, 0, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

ParticleTypeIdentifier ParticleEngine::CreateParticleType( const ParticleTypeSettings& particleTypeSettings )
{
	// Set settings to what the callers parameter is
	m_CurrentParticleBucketSettings.ParticleTypeSettings = particleTypeSettings;
	ParticleBucket* particleBucket = pNew(ParticleBucket);
	// This increases particle buffer offset and block offset for the m_CurrentParticleBucketSettings variable 
	// after the new bucket has been initialized.
	particleBucket->Initialize( m_CurrentParticleBucketSettings );

	// Make sure we are not overflowing the particle systems buffer
	assert( m_CurrentParticleBucketSettings.BlockOffset <= PARTICLE_ENGINE_MAX_NR_OF_SYSTEMS );
	// Make sure we are not overflowing the particle buffer
	assert( m_CurrentParticleBucketSettings.Offset <= PARTICLE_ENGINE_MAX_NR_OF_PARTICLES );

	// Base new identifier on where the particle type/bucket will be placed
	ParticleTypeIdentifier newIdentifier = static_cast<ParticleTypeIdentifier>(m_ParticleBuckets.size());
	m_ParticleBuckets.push_back(particleBucket);
	return newIdentifier;
}

ParticleSystem* ParticleEngine::Spawn( ParticleTypeIdentifier particleType, const ParticleSystem& particleSystem )
{
	assert( particleType < m_ParticleBuckets.size() );
	return m_ParticleBuckets.at(particleType)->Spawn( particleSystem );
}

const ParticleSystem& ParticleEngine::GetDefaultParticleSystem( ParticleTypeIdentifier particleType ) const
{
	assert( particleType < m_ParticleBuckets.size() );
	return m_ParticleBuckets.at(particleType)->GetDefaultParticleSystem();
}

ParticleBucket::BucketUpdateInfo ParticleEngine::GetParticleBucketUpdateInfo( ParticleTypeIdentifier particleType ) const
{
	assert( particleType < m_ParticleBuckets.size() );
	return m_ParticleBuckets.at(particleType)->GetBucketUpdateInfo();
}
