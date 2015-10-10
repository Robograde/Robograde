/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "Plane.h"

Volume* Plane::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const
{
	Volume* worldVolume = pNew( Plane );
	CalculateWorld( world, position, orientation, scale, worldVolume );
	return worldVolume;
}

void Plane::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const
{
	Plane* worldPlane		= static_cast<Plane*>( outWorldVolume );
	worldPlane->Position	= glm::vec3( world * glm::vec4( this->Position,	1.0f ) );
	worldPlane->Normal		= glm::normalize( glm::vec3( world * glm::vec4( this->Normal, 0.0f ) ) );
}

const VOLUME_TYPE Plane::GetVolumeType() const
{
	return VOLUME_TYPE_PLANE;
}