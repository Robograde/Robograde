/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "PlaneOBB.h"

#include "../volume/Plane.h"
#include "../volume/OBB.h"

bool IntersectionTest::PlaneOBB( const Volume* planeVolume, const Volume* obbVolume, glm::vec3* outIntersectionPoint )
{
	const Plane*	plane	= static_cast< const Plane*	>( planeVolume	);
	const OBB*		obb		= static_cast< const OBB*>( obbVolume	);
	
	glm::vec3		cornerPosition;
	int				dir[3];

	for ( dir[0] = -1; dir[0] <= 1; dir[0] += 2 )
	{
		for ( dir[1] = -1; dir[1] <= 1; dir[1] += 2 )
		{
			for ( dir[2] = -1; dir[2] <= 1; dir[2] += 2 )
			{
				cornerPosition = obb->Position;
				for ( int i = 0; i < 3; ++i )
				{
					cornerPosition += (dir[i] * obb->HalfSizes[i]) * obb->Directions[i];
				}

				if ( glm::dot( plane->Normal, cornerPosition - plane->Position ) <= 0.0f )
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool IntersectionTest::OBBPlane( const Volume* obbVolume, const Volume* planeVolume, glm::vec3* outIntersectionPoint )
{
	return PlaneOBB( planeVolume, obbVolume, outIntersectionPoint );
}