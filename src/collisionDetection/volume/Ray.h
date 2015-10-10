/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "Volume.h"

#include <memory/Alloc.h>

class Ray : public Volume
{
public:
	Volume*					CalculateWorld	( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const override;
	void					CalculateWorld	( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const override;
	const VOLUME_TYPE		GetVolumeType	() const override;

	glm::vec3				Position;
	glm::vec3				Direction;		// Should be normalized.
};