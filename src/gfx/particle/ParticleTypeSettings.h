/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <GL/glew.h>
#include "../Texture.h"

namespace gfx
{
struct ParticleTypeSettings
{
	// Path to shader that initializes a particle effect
	rString InitialShaderPath;
	// Path to shader that updates particle movment
	rString UpdateShaderPath;
	// How many blocks that can be allocated
	unsigned int MaxNumberOfBlocks;
	// How large a block is
	unsigned int BlockSize;
	// Name to identify bucket for logging messages
	rString IdentifyingName = "";
	// What texture to use
	gfx::Texture* Texture;
	// Specifies how the red, green, blue, and alpha source blending factors are computed.
	GLenum SourceFactorBlending = GL_SRC_ALPHA;
	// Specifies how the red, green, blue, and alpha destination blending factors are computed.
	GLenum DestinationFactorBlending = GL_ONE_MINUS_SRC_ALPHA;
	// Default particle system, used to remove the need for setting all stats when spawning a particle
	ParticleSystem DefaultParticleSystem;
};
}
