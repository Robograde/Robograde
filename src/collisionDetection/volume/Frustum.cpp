/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "Frustum.h"

Volume* Frustum::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const
{
	Volume* worldVolume = pNew( Frustum );
	CalculateWorld( world, position, orientation, scale, worldVolume );
	return worldVolume;
}

void Frustum::CalculateWorld( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const
{
	Frustum* worldFrustum = static_cast<Frustum*>( outWorldVolume );
	for ( int i = 0; i < 6; ++i )
	{
		this->Planes[i].CalculateWorld( world, position, orientation, scale, &(worldFrustum->Planes[i]) );
	}
}

const VOLUME_TYPE Frustum::GetVolumeType() const
{
	return VOLUME_TYPE_FRUSTUM;
}