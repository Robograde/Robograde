/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "FrustumSphere.h"

#include "../volume/Frustum.h"
#include "../volume/Sphere.h"
#include "PlaneSphere.h"

bool IntersectionTest::FrustumSphere( const Volume* frustumVolume, const Volume* sphereVolume, glm::vec3* outIntersectionPoint )
{
	const Frustum*	frustum		= static_cast< const Frustum*	>( frustumVolume );
	const Sphere*	sphere		= static_cast< const Sphere*	>( sphereVolume	 );

	for ( int i = 0; i < 6; ++i )
	{
		if ( !PlaneSphere( &frustum->Planes[i], sphere, outIntersectionPoint ) )
		{
			return false;
		}
	}
	return true;
}

bool IntersectionTest::SphereFrustum( const Volume* sphereVolume, const Volume* frustumVolume, glm::vec3* outIntersectionPoint )
{
	return FrustumSphere( frustumVolume, sphereVolume, outIntersectionPoint );
}