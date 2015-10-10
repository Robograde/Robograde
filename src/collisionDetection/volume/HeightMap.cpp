/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "HeightMap.h"

HeightMap::HeightMap( GetHeightFunction getHeightFunction )
    : HeightFunction( getHeightFunction ) {}

Volume* HeightMap::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const
{
    Volume* worldVolume = pNew( HeightMap, this->HeightFunction );
	CalculateWorld( world, position, orientation, scale, worldVolume );
	return worldVolume;
}

void HeightMap::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const
{
    HeightMap* worldHeightMap		= static_cast<HeightMap*>( outWorldVolume );
	worldHeightMap->HeightFunction	= this->HeightFunction;
}

const VOLUME_TYPE HeightMap::GetVolumeType() const
{
	return VOLUME_TYPE_HEIGHT_MAP;
}
