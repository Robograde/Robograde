/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "CollisionDetection.h"

#include <float.h>				// For FLT_MAX on linux
#include "../volume/Ray.h"
#include "../volume/Frustum.h"

void CollisionDetection::PickingWithRay( glm::vec3& rayPosition, glm::vec3& rayDirection, const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities, glm::vec3* outPosition )
{
	Ray ray;
	ray.Position	= rayPosition;
	ray.Direction	= rayDirection;
	
	this->PickingWithVolume( &ray, pickAgainstGroupsIDs, outEntities, outPosition, true );
}

void CollisionDetection::PickingWithFrustum( glm::vec3 (&planePositions)[6], glm::vec3 (&planeNormals)[6], const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities )
{
	Frustum frustum;
	for ( int i = 0; i < 6; ++i )
	{
		frustum.Planes[i].Position	= planePositions[i];
		frustum.Planes[i].Normal	= planeNormals[i];
	}

	this->PickingWithVolume( &frustum, pickAgainstGroupsIDs, outEntities, nullptr, false );
}

void CollisionDetection::PickingWithVolume( Volume* pickingVolume, const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities, glm::vec3* outPosition, bool pickClosest )
{
	// Used if pickClosest is true.
	float				distanceToClosestPoint	= FLT_MAX;
	const unsigned int*	closestEntity			= nullptr;

	for ( auto& entity : m_Entities )
	{
		// Check if entity is of a group that intersection should be tested against.
		bool pickAgainstEntity = false;
		for ( auto& groupID : pickAgainstGroupsIDs )
		{
			if ( entity->GetGroupID() == groupID )
			{
				pickAgainstEntity = true;
				break;
			}
		}

		// Check if unit is affected by and inside the fog of war.
		if ( pickAgainstEntity )
		{
			for ( auto& fogGroup : m_GroupsAffectedByFog )
			{
				if ( entity->GetGroupID() == fogGroup && !CalcVisibilityForPosition( entity->GetPosition() ) )
				{
					pickAgainstEntity = false;
				}
			}
		}
		
		if ( pickAgainstEntity )
		{
			// Check for intersections against the entities volumes.
			for ( auto& volume : entity->GetWorldVolumes() )
			{
				IntersectionTestFunction intersectionTestFunction = m_TestLookup.Fetch( pickingVolume->GetVolumeType(), volume->GetVolumeType() );

				if ( pickClosest )
				{
					glm::vec3 intersectionPoint;
					if ( (*intersectionTestFunction)( pickingVolume, volume, &intersectionPoint ) )
					{
						float distanceToIntersectionPoint = glm::length( intersectionPoint );
						if ( distanceToIntersectionPoint < distanceToClosestPoint )
						{
							distanceToClosestPoint	= distanceToIntersectionPoint;
							closestEntity			= &entity->GetUserData();
							*outPosition			= intersectionPoint;
						}
					}
				}
				else
				{
					if ( (*intersectionTestFunction)( pickingVolume, volume, outPosition ) )
					{
						outEntities.push_back( entity->GetUserData() );
						break;
					}
				}
			}
		}
	}

	if ( closestEntity )
	{
		outEntities.push_back( *closestEntity );
	}
}