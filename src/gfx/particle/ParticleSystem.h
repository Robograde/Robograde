/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <glm/glm.hpp>

namespace gfx
{
struct ParticleSystem
{
	glm::vec3 	EmitterPosition 		= glm::vec3(0);
	float 		TimeToLive				= 0.5f;
	glm::vec4 	Colour 					= glm::vec4(0);
	glm::vec3 	Direction 				= glm::vec3(0);
	float 		ParticlesSpeed 			= 0.5f;
	glm::vec2 	Size					= glm::vec2(0.1f);
	float 		ParticlesTimeToLive 	= 0.5f;
	float 		DoDraw 					= 1.0f;
	glm::vec2 	TransparencyPolynome 	= glm::vec2(-1, 1);
	glm::vec2 	CullBoxSize				= glm::vec2(0);
};
}
