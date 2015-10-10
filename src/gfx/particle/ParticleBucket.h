/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include "../Shader/ShaderBank.h"
#include "ParticleSystem.h"
#include "ParticleTypeSettings.h"

namespace gfx
{
class ParticleBucket
{
public:
	struct BucketUpdateInfo
	{
		unsigned int StartBlock;
		unsigned int EndBlock;
		rVector<ParticleSystem>* Buffer;
	};
	struct ParticleBucketSettings
	{
		// Name of the buffer to write particle data to
		rString BufferName;
		// Offset in block array
		unsigned int BlockOffset;
		// Where in the buffer to start writing
		unsigned int Offset;
		// Buffer for particle systems
		rVector<ParticleSystem>* ParticleSystemsBuffer;
		// Settings set by the API caller
		ParticleTypeSettings ParticleTypeSettings;
	};

	// Changes settings to reflect what this bucket allocated
	void 		 	Initialize		( ParticleBucketSettings& settings );
	ParticleSystem* SpawnDefault 	( );
	ParticleSystem*	Spawn			( const ParticleSystem& particleSystem  );
	void 			UpdateSystems 	( float deltaTime );
	void 			UpdateParticles	( float deltaTime );
	void 			DrawParticles	( ShaderProgram* drawProgram );

	const ParticleSystem& GetDefaultParticleSystem() const;

	void 			ReleaseBlock	( int block );

	BucketUpdateInfo GetBucketUpdateInfo( ) const;

private:
	int 			AllocateBlock	( );
	bool			IsEmpty			( ) const;

	rString 				m_IdentifyingName	= "";
	rString					m_BufferName		= "";		
	unsigned int 			m_Offset 			= 0;
	unsigned int 			m_BlockSize 		= 64;
	unsigned int 			m_BlockOffset		= 0;
	unsigned int 			m_NextBlock			= 0;
	unsigned int 			m_MaxNumberOfBlocks = 128;
	gfx::ShaderProgram* 	m_UpdateProgram 	= nullptr;
	gfx::ShaderProgram* 	m_InitialProgram	= nullptr;
	//gfx::ShaderProgram* 	m_DrawProgram 		= nullptr;
	gfx::Texture* 			m_Texture 			= nullptr;
	rVector<unsigned int> 	m_FreeBlocks;
	rVector<ParticleSystem>* m_ParticleSystemsBuffer;
	GLenum 					m_SourceFactorBlending 		= GL_SRC_ALPHA;
	GLenum 					m_DestinationFactorBlending = GL_ONE_MINUS_SRC_ALPHA;
	ParticleSystem 			m_DefaultParticleSystem;
};
}
