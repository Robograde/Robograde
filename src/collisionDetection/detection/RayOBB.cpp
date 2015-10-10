/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "RayOBB.h"

#include <float.h>
#include "../volume/Ray.h"
#include "../volume/OBB.h"

#define EPSILON 1.0e-14f

bool IntersectionTest::RayOBB( const Volume* rayVolume, const Volume* obbVolume, glm::vec3* outIntersectionPoint )
{
	const Ray*	ray	= static_cast< const Ray*	>( rayVolume	);
	const OBB*	obb	= static_cast< const OBB*	>( obbVolume	);

	float t_min = -FLT_MAX;
	float t_max =  FLT_MAX;

	const glm::vec3 p = obb->Position - ray->Position;

	float e;
	float f;
	float fInv;
	float t_1;
	float t_2;

	for ( int i = 0; i < OBB_DIRECTIONS; ++i )
	{
		e	= glm::dot( obb->Directions[i], p );
		f	= glm::dot( obb->Directions[i], ray->Direction );

		if ( f > EPSILON || f < -EPSILON )
		{
			fInv	= 1.0f / f;
			t_1 = (e + obb->HalfSizes[i]) * fInv;
			t_2 = (e - obb->HalfSizes[i]) * fInv;

			if ( t_1	> t_2	)	{ const float temp = t_1; t_1 = t_2; t_2 = temp; }
			if ( t_1	> t_min )	{ t_min = t_1; }
			if ( t_2	< t_max )	{ t_max = t_2; }
			if ( t_min	> t_max )	{ return false; }
			if ( t_max	< 0.0f	)	{ return false; }
		}
		else if ( ( -e - obb->HalfSizes[i] > 0.0f) || ( -e + obb->HalfSizes[i] < 0.0f ) ) { return false; }
	}

	float distance = t_min > 0.0f ? t_min : t_max;
	//*outIntersectionPoint = ray->Position + distance * ray->Direction;	// TODOOE: Make this work.
	return true;
}

bool IntersectionTest::OBBRay( const Volume* obbVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint )
{
	return RayOBB( rayVolume, obbVolume, outIntersectionPoint );
}