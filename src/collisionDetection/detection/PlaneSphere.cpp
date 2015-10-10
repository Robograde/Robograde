/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "PlaneSphere.h"

#include "../volume/Plane.h"
#include "../volume/Sphere.h"

bool IntersectionTest::PlaneSphere( const Volume* planeVolume, const Volume* sphereVolume, glm::vec3* outIntersectionPoint )
{
	const Plane*	plane		= static_cast< const Plane*	>( planeVolume	);
	const Sphere*	sphere		= static_cast< const Sphere*>( sphereVolume	);

	const float		distance	= glm::dot( plane->Normal, sphere->Position - plane->Position );

	return distance <= sphere->Radius;
}

bool IntersectionTest::SpherePlane( const Volume* sphereVolume, const Volume* planeVolume, glm::vec3* outIntersectionPoint )
{
	return PlaneSphere( planeVolume, sphereVolume, outIntersectionPoint );
}