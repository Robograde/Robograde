/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>

//----+-------------------------+----------------------
//----| ParticleEmitterType			|
//----+-------------------------+----------------------
//----+-----------------+------------------------------
enum ParticleEmitterType : short
{
	PARTICLE_EMITTER_TYPE_SMOKE,
	PARTICLE_EMITTER_TYPE_FIRE,
};

//+===+================================================
//----| ParticleEmitterComponent	|
//----+---------------------+-------------------------
//+===+================================================
struct ParticleEmitterComponent
{
	rString				Name;
	ParticleEmitterType	ParticleType;
	glm::vec3			Pos;
	glm::vec4			Col;
};