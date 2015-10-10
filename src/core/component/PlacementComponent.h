/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct PlacementComponent
{
	glm::vec3		Position		= glm::vec3( 0.0f );
	glm::vec3		Scale			= glm::vec3( 1.0f );
	glm::quat		Orientation		= glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );

	glm::mat4		World			= glm::mat4( 1.0f );
};