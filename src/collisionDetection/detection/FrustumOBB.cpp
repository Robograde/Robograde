/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "FrustumOBB.h"

#include "../volume/Frustum.h"
#include "../volume/OBB.h"
#include "PlaneOBB.h"

bool IntersectionTest::FrustumOBB( const Volume* frustumVolume, const Volume* obbVolume, glm::vec3* outIntersectionPoint )
{
	const Frustum*	frustum	= static_cast< const Frustum*	>( frustumVolume );
	const OBB*		obb		= static_cast< const OBB*		>( obbVolume	 );

	for ( int i = 0; i < 6; ++i )
	{
		if ( !PlaneOBB( &frustum->Planes[i], obb, outIntersectionPoint ) )
		{
			return false;
		}
	}
	return true;
}

bool IntersectionTest::OBBFrustum( const Volume* obbVolume, const Volume* frustumVolume, glm::vec3* outIntersectionPoint )
{
	return FrustumOBB( frustumVolume, obbVolume, outIntersectionPoint );
}