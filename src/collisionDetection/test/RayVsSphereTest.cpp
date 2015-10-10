/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include <gtest/gtest.h>

#include "../volume/Ray.h"
#include "../volume/Sphere.h"
#include "../detection/IntersectionTestLookupTable.h"

using glm::vec3;

TEST( CollisionTests, RayVsSphere_OutsideTowards )
{
	Ray ray;
	ray.Position	= vec3( 10.0f, 0.5f, 0.5f );
	ray.Direction	= glm::normalize( vec3( -1.0f, 0.0f, 0.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 1.0f, 0.0f, 0.0f );
	sphere.Radius	= 1.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_OutsideMiss )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 4.0f, 10.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 1.0f, 0.0f, 0.0f );
	sphere.Radius	= 3.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_ParallelNearHit )
{
	Ray ray;
	ray.Position	= vec3( 3.999f, 0.0f, -2.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 0.0f, 1.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 0.0f, 0.0f, 0.0f );
	sphere.Radius	= 4.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_ParallelNearMiss )
{
	Ray ray;
	ray.Position	= vec3( 6.01f, 4.0f, 0.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 0.0f, 0.0f, 0.0f );
	sphere.Radius	= 6.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_OutsideAway )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 1.0f, 0.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 1.0f, 0.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 0.0f, -1.0f, 0.0f );
	sphere.Radius	= 1.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_FALSE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_WithinTowards )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 0.0f, 0.5f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 0.0f, 0.0f, 0.0f );
	sphere.Radius	= 3.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_WithinAway )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 0.0f, -0.5f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 0.0f, -1.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 0.0f, 0.0f, 0.0f );
	sphere.Radius	= 4.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );
}

TEST( CollisionTests, RayVsSphere_WithinCenter )
{
	Ray ray;
	ray.Position	= vec3( 0.0f, 0.0f, 0.0f );
	ray.Direction	= glm::normalize( vec3( 0.0f, 1.0f, 0.0f ) );

	Sphere sphere;
	sphere.Position	= vec3( 0.0f, 0.0f, 0.0f );
	sphere.Radius	= 0.0f;

	IntersectionTestLookupTable testLookup;
	IntersectionTestFunction intersectionTestFunction = testLookup.Fetch( ray.GetVolumeType(), sphere.GetVolumeType() );

	EXPECT_TRUE( (*intersectionTestFunction)( &ray, &sphere, nullptr ) );

}
