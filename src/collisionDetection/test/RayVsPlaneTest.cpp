/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include <gtest/gtest.h>

#include "../volume/Ray.h"
#include "../volume/Plane.h"
#include "../detection/IntersectionTestLookupTable.h"

using glm::vec3;

TEST( CollisionTests, RayVsPlane_OutsideTowards )
{
	Ray ray;
	ray.Position	= vec3( 1.0f, 7.0f, 2.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( 30.0f, 0.0f, -50.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, 1.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
	EXPECT_EQ( vec3( 1.0f, 0.0f, 2.0f ), intersectionPoint );
}

TEST( CollisionTests, RayVsPlane_OutsideAway )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 7.0f, 0.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 1.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( 30.0f, 0.0f, -50.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, 1.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_OutsideParallel )
{
	Ray ray;
	ray.Position	= vec3( 3.0f, -4.0f, 8.0f );
	ray.Direction	= glm::normalize( vec3( -1.0f, 0.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( 75.0f, 5.0f, -23.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_InsideDiving )
{
	Ray ray;
	ray.Position	= vec3( 27.0f, 7.0f, -5.0f );
	ray.Direction	= glm::normalize( vec3( -1.0f, 1.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( 30.0f, 56.0f, -53.0f );
	plane.Normal	= glm::normalize( vec3( 1.0f, 0.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_InsideSurfacing )
{
	Ray ray;
	ray.Position	= vec3( 27.0f, 7.0f, -5.0f );
	ray.Direction	= glm::normalize( vec3( 1.0f, 1.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( 30.0f, 56.0f, -53.0f );
	plane.Normal	= glm::normalize( vec3( 1.0f, 0.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_InsideParallel )
{
	Ray ray;
	ray.Position	= vec3( 27.0f, 7.0f, -5.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	Plane plane;
	plane.Position	= vec3( 30.0f, 56.0f, -53.0f );
	plane.Normal	= glm::normalize( vec3( 1.0f, 0.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_SurfaceDiving )
{
	Ray ray;
	ray.Position	= vec3( 31.0f, -5.0f, -3.0f );
	ray.Direction	= glm::normalize( vec3( -1.0f, 0.0f, 1.0f ) );

	Plane plane;
	plane.Position	= vec3( -13.0f, 7.0f, -3.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
	EXPECT_EQ( ray.Position, intersectionPoint );
}

TEST( CollisionTests, RayVsPlane_SurfaceLeaving )
{
	Ray ray;
	ray.Position	= vec3( 31.0f, -5.0f, -3.0f );
	ray.Direction	= glm::normalize( vec3( -1.0f, 0.0f, -1.0f ) );

	Plane plane;
	plane.Position	= vec3( -13.0f, 7.0f, -3.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_SurfaceParallel )
{
	Ray ray;
	ray.Position	= vec3( 31.0f, -5.0f, -3.0f );
	ray.Direction	= glm::normalize( vec3( 3.0f, -5.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( -13.0f, 7.0f, -3.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
}

TEST( CollisionTests, RayVsPlane_OutsideRayDiagonal )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 4.0f, 0.0f );
	ray.Direction	= glm::normalize( vec3( 1, -1.0f, -1.0f ) );

	Plane plane;
	plane.Position	= vec3( -17.0f, 2.0f, 11.0f );
	plane.Normal	= glm::normalize( vec3( 0.0f, 1.0f, 0.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
	EXPECT_EQ( vec3( 2.0f, 2.0f, -2.0f ), intersectionPoint );
}

TEST( CollisionTests, RayVsPlane_OutsidePlaneDiagonal )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 4.0f, 0.0f );
	ray.Direction	= glm::normalize( vec3( 0, -1.0f, 0.0f ) );

	Plane plane;
	plane.Position	= vec3( 0.0f, 2.0f, 0.0f );
	plane.Normal	= glm::normalize( vec3( -1.0f, 1.0f, 2.0f ) );

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), plane.GetVolumeType() );

	vec3 intersectionPoint;

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &plane, &intersectionPoint ) );
	EXPECT_EQ( vec3( 0.0f, 2.0f, 0.0f ), intersectionPoint );
}