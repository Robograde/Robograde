/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "RayPlane.h"

#include "../volume/Ray.h"
#include "../volume/Plane.h"

bool IntersectionTest::RayPlane( const Volume* rayVolume, const Volume* planeVolume, glm::vec3* outIntersectionPoint )
{
	const Ray*		ray			= static_cast< const Ray*	>( rayVolume	);
	const Plane*	plane		= static_cast< const Plane*	>( planeVolume	);

	const float dirProjOnNorm	= glm::dot( ray->Direction, plane->Normal );

	// No intersection if ray is turned away from plane.
	if( dirProjOnNorm >= 0.0f )
	{
		return false;
	}

	const glm::vec3	distance	= plane->Position - ray->Position;
	const float		penetration	= glm::dot( distance, plane->Normal );

	// No intersection if ray starts out inside the plane.
	if ( penetration > 0.0f )
	{
		return false;
	}

	// Calculate intersection point.
	*outIntersectionPoint		= ray->Position + (penetration / dirProjOnNorm) * ray->Direction;
	return true;
}

bool IntersectionTest::PlaneRay( const Volume* planeVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint )
{
	return RayPlane( rayVolume, planeVolume, outIntersectionPoint );
}