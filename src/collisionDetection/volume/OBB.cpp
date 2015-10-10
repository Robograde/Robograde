/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "OBB.h"

Volume* OBB::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const
{
	Volume* worldVolume = pNew( OBB );
	CalculateWorld( world, position, orientation, scale, worldVolume );
	return worldVolume;
}

void OBB::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const
{
	OBB* worldOBB		= static_cast<OBB*>( outWorldVolume );
	worldOBB->Position	= glm::vec3( world * glm::vec4( this->Position,	1.0f ) );
	for ( int i = 0; i < OBB_DIRECTIONS; ++i )
	{
		worldOBB->Directions[i]	= glm::vec3( world * glm::vec4( this->HalfSizes[i] * this->Directions[i], 0.0f ) );
		worldOBB->HalfSizes[i]	= glm::length( worldOBB->Directions[i] );
		worldOBB->Directions[i]	/= worldOBB->HalfSizes[i];
	}
}

const VOLUME_TYPE OBB::GetVolumeType() const
{
	return VOLUME_TYPE_OBB;
}