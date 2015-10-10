/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "RaySphere.h"

#include "../volume/Ray.h"
#include "../volume/Sphere.h"

bool IntersectionTest::RaySphere( const Volume* rayVolume, const Volume* sphereVolume, glm::vec3* outIntersectionPoint )
{
	const Ray*		ray				= static_cast< const Ray*	>( rayVolume	);
	const Sphere*	sphere			= static_cast< const Sphere*>( sphereVolume	);

	const glm::vec3	distance		= sphere->Position - ray->Position;
	const float		distanceSqrd	= glm::dot( distance, distance );
	const float		radiusSqrd		= sphere->Radius * sphere->Radius;

	// Checks if the rays position is within the spheres volume.
	if ( distanceSqrd <= radiusSqrd )
	{
		return true;
	}
	
	const float distProjOnDir	= glm::dot( ray->Direction, distance );

	// Checks if the sphere center is infront of the ray.
	if ( distProjOnDir < 0.0f )
	{
		return false;
	}

	const float	mSqrd	= distanceSqrd - distProjOnDir * distProjOnDir;
	const float	qSqrd	= radiusSqrd - mSqrd;

	// Checks if the shortest distance to the ray is smaller than the radius of the sphere.
	return qSqrd >= 0.0f;
}

bool IntersectionTest::SphereRay( const Volume* sphereVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint )
{
	return RaySphere( rayVolume, sphereVolume, outIntersectionPoint );
}