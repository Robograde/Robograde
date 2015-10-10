/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "Ray.h"

Volume* Ray::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const
{
	Volume* worldVolume = pNew( Ray );
	CalculateWorld( world, position, orientation, scale, worldVolume );
	return worldVolume;
}

void Ray::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const
{
	Ray* worldRay		= static_cast<Ray*>( outWorldVolume );
	worldRay->Position	= glm::vec3( world * glm::vec4( this->Position,  1.0f ) );
	worldRay->Direction	= glm::normalize( glm::vec3( world * glm::vec4( this->Direction, 0.0f ) ) );
}

const VOLUME_TYPE Ray::GetVolumeType() const
{
	return VOLUME_TYPE_RAY;
}