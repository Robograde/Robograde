/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "ParticleBucket.h"

#include <utility/Logger.h>
#include <profiler/AutoGPUProfiler.h>
#include "../Shader/ShaderBank.h"
#include "../BufferManager.h"

using namespace gfx;

void ParticleBucket::Initialize( ParticleBucketSettings& settings )
{
	// Settings for the engine
	m_ParticleSystemsBuffer = settings.ParticleSystemsBuffer;
	m_Offset 				= settings.Offset;
	m_BufferName 			= settings.BufferName;
	m_BlockOffset 			= settings.BlockOffset;

	// Shader creation
	ShaderProgramHandle handle;
	handle = g_ShaderBank.LoadShaderProgram( settings.ParticleTypeSettings.UpdateShaderPath );
	m_UpdateProgram 	= g_ShaderBank.GetProgramFromHandle( handle );
	handle = g_ShaderBank.LoadShaderProgram( settings.ParticleTypeSettings.InitialShaderPath );
	m_InitialProgram 	= g_ShaderBank.GetProgramFromHandle( handle );

	// Settings for API caller
	m_IdentifyingName 			= settings.ParticleTypeSettings.IdentifyingName;
	m_MaxNumberOfBlocks 		= settings.ParticleTypeSettings.MaxNumberOfBlocks;
	m_BlockSize 				= settings.ParticleTypeSettings.BlockSize;
	m_Texture 					= settings.ParticleTypeSettings.Texture;
	m_SourceFactorBlending 		= settings.ParticleTypeSettings.SourceFactorBlending;
	m_DestinationFactorBlending = settings.ParticleTypeSettings.DestinationFactorBlending;
	m_DefaultParticleSystem 	= settings.ParticleTypeSettings.DefaultParticleSystem;

	// Increase offsets for the engines buffers
	settings.Offset 		+= m_BlockSize * m_MaxNumberOfBlocks;
	settings.BlockOffset 	+= settings.ParticleTypeSettings.MaxNumberOfBlocks;
}

ParticleSystem* ParticleBucket::SpawnDefault( )
{
	return this->Spawn( GetDefaultParticleSystem() );
}

ParticleSystem* ParticleBucket::Spawn( const ParticleSystem& particleSystem )
{
	int localBlock = AllocateBlock();
	if ( localBlock == -1 )
		return nullptr;
	unsigned int block = m_BlockOffset + localBlock;
	ParticleSystem* returnParticleSystem = &m_ParticleSystemsBuffer->at( block );
	*returnParticleSystem = particleSystem;
	
	// Reflect new allocation to the GPU buffer
	g_BufferManager.UpdateBuffer( m_BufferName,
			sizeof(ParticleSystem) * (block), &m_ParticleSystemsBuffer->at(block), sizeof(ParticleSystem) );

	m_InitialProgram->Apply();

	g_BufferManager.BindBufferToProgram( m_BufferName, m_InitialProgram, 0 );

	unsigned int start = m_Offset + localBlock * m_BlockSize;
	m_InitialProgram->SetUniformUInt( "g_Start", start );
	m_InitialProgram->SetUniformUInt( "g_End", start + m_BlockSize );
	m_InitialProgram->SetUniformUInt( "g_BlockOffset", block );
	m_InitialProgram->SetUniformUInt( "g_BlockSize", m_BlockSize );
	m_InitialProgram->SetUniformUInt( "g_Offset", m_Offset );

    glDispatchCompute ( ( GLuint ) ceilf ( m_BlockSize / 64.0f ), 1, 1 );

	return returnParticleSystem;
}

void ParticleBucket::UpdateSystems( float deltaTime )
{
	if ( !IsEmpty( ) )
	{
		// TODOJM: Profile basic for loop and iterator for loop
		for ( unsigned int i = m_BlockOffset; i < m_BlockOffset + m_NextBlock; ++i )
		{
			if ( m_ParticleSystemsBuffer->at( i ).TimeToLive >= 0.0 )
			{
				m_ParticleSystemsBuffer->at( i ).TimeToLive -= deltaTime;
				if ( m_ParticleSystemsBuffer->at( i ).TimeToLive < 0.0 )
				{
					ReleaseBlock( i - m_BlockOffset );
				}
			}
		}
	}
}

void ParticleBucket::UpdateParticles( float deltaTime )
{
	if ( !IsEmpty( ) )
	{
		m_UpdateProgram->Apply( );

		m_UpdateProgram->SetUniformFloat( "g_DeltaTime", deltaTime );
		m_UpdateProgram->SetUniformUInt( "g_Offset", m_Offset );
		m_UpdateProgram->SetUniformUInt( "g_BlockOffset", m_BlockOffset );
		m_UpdateProgram->SetUniformUInt( "g_BlockSize", m_BlockSize );

		g_BufferManager.BindBufferToProgram( m_BufferName, m_UpdateProgram, 0 );

		glDispatchCompute( (GLuint)ceilf( m_NextBlock * m_BlockSize / 64.0f ), 1, 1 );
	}
}

void ParticleBucket::DrawParticles( ShaderProgram* drawProgram )
{
	if ( !IsEmpty( ) )
	{
		glBlendFunc( m_SourceFactorBlending, m_DestinationFactorBlending );
		drawProgram->SetUniformUInt( "g_BlockOffset", m_BlockOffset );
		drawProgram->SetUniformUInt( "g_Offset", m_Offset );
		drawProgram->SetUniformUInt( "g_BlockSize", m_BlockSize );
		m_Texture->Apply( drawProgram->FetchUniform( "g_Texture" ), 1 );
		GPU_PROFILE( AutoGPUProfiler gpDraw( "DrawParticles" + m_IdentifyingName ); );
		glDrawArrays( GL_POINTS, m_Offset, m_BlockSize * m_NextBlock );
		GPU_PROFILE( gpDraw.Stop( ); );
	}
}

const ParticleSystem& ParticleBucket::GetDefaultParticleSystem() const
{
	return m_DefaultParticleSystem;
}

int ParticleBucket::AllocateBlock()
{
	if ( m_FreeBlocks.size() > 0 )
	{
		int returnValue = m_FreeBlocks.back();
		m_FreeBlocks.pop_back();
		return returnValue;
	}
	else
	{
		// Don't allocate outside our block space
		if ( m_NextBlock == m_MaxNumberOfBlocks )
		{
			Logger::Log( "Can't spawn more particle systems of type: " + m_IdentifyingName +
					", maximum number supported: " + rToString(m_MaxNumberOfBlocks) + ".", "ParticleBucket", LogSeverity::WARNING_MSG );
			return -1;
		}
		return m_NextBlock++;
	}
}

void ParticleBucket::ReleaseBlock( int block )
{
	m_FreeBlocks.push_back( block );
}

ParticleBucket::BucketUpdateInfo ParticleBucket::GetBucketUpdateInfo( ) const
{
	return BucketUpdateInfo{m_BlockOffset, m_BlockOffset + m_NextBlock, m_ParticleSystemsBuffer};
}

bool ParticleBucket::IsEmpty( ) const
{
	return m_NextBlock == m_FreeBlocks.size( );
}