/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "RayHeightMap.h"

#include "../volume/Ray.h"
#include "../volume/HeightMap.h"

#define RAY_VS_HEIGHT_MAP_MAX_ITERATIONS	1000

bool IntersectionTest::RayHeightMap( const Volume* rayVolume, const Volume* heightMapVolume, glm::vec3* outIntersectionPoint )
{
	const Ray*			ray			= static_cast< const Ray*		>( rayVolume		);
	const HeightMap*	heightMap	= static_cast< const HeightMap*	>( heightMapVolume	);

	glm::vec3 pointOnRay = ray->Position;
	
	for ( int i = 0; i < RAY_VS_HEIGHT_MAP_MAX_ITERATIONS; ++i )
	{
		float mapHeight = heightMap->HeightFunction( pointOnRay.x, pointOnRay.z );
		if ( pointOnRay.y <= mapHeight )
		{
			*outIntersectionPoint = glm::vec3( pointOnRay.x, mapHeight, pointOnRay.z );
			return true;
		}
		pointOnRay += ray->Direction;
	}
	return false;
}

bool IntersectionTest::HeightMapRay( const Volume* heightMapVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint )
{
	return RayHeightMap( rayVolume, heightMapVolume, outIntersectionPoint );
}