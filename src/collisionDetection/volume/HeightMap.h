/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "Volume.h"

#include <memory/Alloc.h>

typedef std::function<float( const float x, const float z )> GetHeightFunction;

class HeightMap : public Volume
{
public:
    HeightMap( GetHeightFunction getHeightFunction );
	Volume*					CalculateWorld	( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const override;
	void					CalculateWorld	( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const override;
	const VOLUME_TYPE		GetVolumeType	() const override;

    GetHeightFunction		HeightFunction;
};
