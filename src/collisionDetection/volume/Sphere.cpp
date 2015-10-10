/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "Sphere.h"

Volume* Sphere::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const
{
	Volume* worldVolume = pNew( Sphere );
	CalculateWorld( world, position, orientation, scale, worldVolume );
	return worldVolume;
}

void Sphere::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const
{
	Sphere* worldSphere		= static_cast<Sphere*>( outWorldVolume );
	worldSphere->Position	= glm::vec3( world * glm::vec4( this->Position,	1.0f ) );
	worldSphere->Radius		= scale * this->Radius;
}

const VOLUME_TYPE Sphere::GetVolumeType() const
{
	return VOLUME_TYPE_SPHERE;
}