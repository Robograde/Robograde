/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include <gtest/gtest.h>

#include "../volume/Ray.h"
#include "../volume/Plane.h"
#include "../volume/Sphere.h"
#include "../entity/Entity.h"

using glm::vec3;
using glm::quat;

struct TestEntity
{
	vec3	Position		= vec3( 0.0f );
	quat	Orientation		= quat( 1.0f, 0.0f, 0.0f, 0.0f );
	float	Scale			= 1.0f;

	void	InitStandardEntity( Entity& entity ) const
	{
		entity.SetPosition		( this->Position	);
		entity.SetOrientation	( this->Orientation	);
		entity.SetScale			( this->Scale		);
	}
};

struct TestVolume
{
	vec3	Position;

	virtual Volume*		CreateStandardVolume() const = 0;
	virtual void		Equal( const Volume* volume ) = 0;
};

struct TestRay : public TestVolume
{
	vec3	Direction;

	Volume* CreateStandardVolume() const override
	{
		Ray* volume			= pNew( Ray );
		volume->Position	= this->Position;
		volume->Direction	= this->Direction;
		return volume;
	}

	void Equal( const Volume* volume ) override
	{
		const Ray* ray		= static_cast<const Ray*>(volume);
		this->Position		= ray->Position;
		this->Direction		= ray->Direction;
	}
};

struct TestPlane : public TestVolume
{
	vec3	Normal;

	Volume* CreateStandardVolume() const override
	{
		Plane* volume		= pNew( Plane );
		volume->Position	= this->Position;
		volume->Normal		= this->Normal;
		return volume;
	}

	void Equal( const Volume* volume ) override
	{
		const Plane* plane	= static_cast<const Plane*>(volume);
		this->Position		= plane->Position;
		this->Normal		= plane->Normal;
	}
};

struct TestSphere : public TestVolume
{
	float	Radius;

	Volume* CreateStandardVolume() const override
	{
		Sphere* volume		= pNew( Sphere );
		volume->Position	= this->Position;
		volume->Radius		= this->Radius;
		return volume;
	}

	void Equal( const Volume* volume ) override
	{
		const Sphere* sphere	= static_cast<const Sphere*>(volume);
		this->Position			= sphere->Position;
		this->Radius			= sphere->Radius;
	}
};

void CalculateWorld( const TestEntity& testEntity, const TestVolume* testVolume, TestVolume* worldTestVolume )
{
	Entity entity;
	testEntity.InitStandardEntity( entity );

	Volume* volume = testVolume->CreateStandardVolume();

	entity.AddStorageVolume( volume );
	entity.CalculateWorldVolumes();

	worldTestVolume->Equal( entity.GetWorldVolumes()[0] );
}

TEST( CollisionPrimitives, RayToWorld_Identity )
{
	TestEntity entity;

	TestRay volume, worldVolume;
	volume.Position		= vec3( 1.0f, 0.0f, -3.0f );
	volume.Direction	= glm::normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( volume.Position, worldVolume.Position );
	EXPECT_EQ( volume.Direction, worldVolume.Direction );
}

TEST( CollisionPrimitives, PlaneToWorld_Identity )
{
	TestEntity entity;

	TestPlane volume, worldVolume;
	volume.Position		= vec3( 0.0f, -5.0f, 2.0f );
	volume.Normal		= glm::normalize( vec3( 0.0f, 0.0f, 1.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( volume.Position, worldVolume.Position );
	EXPECT_EQ( volume.Normal, worldVolume.Normal );
}

TEST( CollisionPrimitives, SphereToWorld_Identity )
{
	TestEntity entity;

	TestSphere volume, worldVolume;
	volume.Position		= vec3( 1.0f, -2.0f, 3.0f );
	volume.Radius		= 4.0f;

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( volume.Position, worldVolume.Position );
	EXPECT_EQ( volume.Radius, worldVolume.Radius );
}

TEST( CollisionPrimitives, RayToWorld_Translation )
{
	TestEntity entity;
	entity.Position		= vec3( -2.0f, 4.0f, -8.0f );

	TestRay volume, worldVolume;
	volume.Position		= vec3( 1.0f, 0.0f, -3.0f );
	volume.Direction	= glm::normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( volume.Position + entity.Position, worldVolume.Position );
	EXPECT_EQ( volume.Direction, worldVolume.Direction );
}

TEST( CollisionPrimitives, PlaneToWorld_Translation )
{
	TestEntity entity;
	entity.Position		= vec3( -5.0f, 7.0f, 3.0f );

	TestPlane volume, worldVolume;
	volume.Position		= vec3( 0.0f, -5.0f, 2.0f );
	volume.Normal		= glm::normalize( vec3( 0.0f, 0.0f, 1.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( volume.Position + entity.Position, worldVolume.Position );
	EXPECT_EQ( volume.Normal, worldVolume.Normal );
}

TEST( CollisionPrimitives, SphereToWorld_Translation )
{
	TestEntity entity;
	entity.Position		= vec3( 3.0f, 9.0f, -23.0f );

	TestSphere volume, worldVolume;
	volume.Position		= vec3( 1.0f, -2.0f, 3.0f );
	volume.Radius		= 4.0f;

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( volume.Position + entity.Position, worldVolume.Position );
	EXPECT_EQ( volume.Radius, worldVolume.Radius );
}

TEST( CollisionPrimitives, RayToWorld_Scale )
{
	TestEntity entity;
	entity.Position		= vec3( -2.0f, 4.0f, -8.0f );
	entity.Scale		= 5.0f;

	TestRay volume, worldVolume;
	volume.Position		= vec3( 1.0f, 0.0f, -3.0f );
	volume.Direction	= glm::normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( entity.Scale * volume.Position + entity.Position, worldVolume.Position );
	EXPECT_EQ( volume.Direction, worldVolume.Direction );
}

TEST( CollisionPrimitives, PlaneToWorld_Scale )
{
	TestEntity entity;
	entity.Position		= vec3( -5.0f, 7.0f, 3.0f );
	entity.Scale		= 7.0f;

	TestPlane volume, worldVolume;
	volume.Position		= vec3( 0.0f, -5.0f, 2.0f );
	volume.Normal		= glm::normalize( vec3( 0.0f, 0.0f, 1.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( entity.Scale * volume.Position + entity.Position, worldVolume.Position );
	EXPECT_EQ( volume.Normal, worldVolume.Normal );
}

TEST( CollisionPrimitives, SphereToWorld_Scale )
{
	TestEntity entity;
	entity.Position		= vec3( 3.0f, 9.0f, -23.0f );
	entity.Scale		= 2.0f;

	TestSphere volume, worldVolume;
	volume.Position		= vec3( 1.0f, -2.0f, 3.0f );
	volume.Radius		= 4.0f;

	CalculateWorld( entity, &volume, &worldVolume );

	EXPECT_EQ( entity.Scale * volume.Position + entity.Position, worldVolume.Position );
	EXPECT_EQ( entity.Scale * volume.Radius, worldVolume.Radius );
}

TEST( CollisionPrimitives, RayToWorld_HalfTurn )
{
	TestEntity entity;
	entity.Position		= vec3( -2.0f, 4.0f, -8.0f );
	entity.Orientation	= quat( 0.0f, 0.0f, 1.0f, 0.0f );	// 180 degree rotation around y.
	entity.Scale		= 7.0f;

	TestRay volume, worldVolume;
	volume.Position		= vec3( 1.0f, 0.0f, -3.0f );
	volume.Direction	= glm::normalize( vec3( -1.0f, 0.0f, 0.0f ) );

	CalculateWorld( entity, &volume, &worldVolume );

	vec3 volumePositionHalfTurned = vec3( -volume.Position.x, volume.Position.y, -volume.Position.z );

	EXPECT_EQ( entity.Scale * volumePositionHalfTurned + entity.Position, worldVolume.Position );
	EXPECT_EQ( -volume.Direction, worldVolume.Direction );
}

TEST( CollisionPrimitives, PlaneToWorld_HalfTurn )
{
	TestEntity entity;
	entity.Position		= vec3( -3.0f, 6.0f, -1.0f );
	entity.Orientation	= quat( 0.0f, 1.0f, 0.0f, 0.0f );	// 180 degree rotation around x.
	entity.Scale		= 2.0f;

	TestPlane volume, worldVolume;
	volume.Position		= vec3( -2.0f, 5.0f, -7.0f );
	volume.Normal		= vec3( 0.0f, 0.0f, 1.0f );

	CalculateWorld( entity, &volume, &worldVolume );

	vec3 volumePositionHalfTurned = vec3( volume.Position.x, -volume.Position.y, -volume.Position.z );

	EXPECT_EQ( entity.Scale * volumePositionHalfTurned + entity.Position, worldVolume.Position );
	EXPECT_EQ( -volume.Normal, worldVolume.Normal );
}

TEST( CollisionPrimitives, SphereToWorld_HalfTurn )
{
	TestEntity entity;
	entity.Position		= vec3( 6.0f, 5.0f, -3.0f );
	entity.Orientation	= quat( 0.0f, 0.0f, 0.0f, 1.0f );	// 180 degree rotation around z.
	entity.Scale		= 4.0f;

	TestSphere volume, worldVolume;
	volume.Position		= vec3( 2.0f, 8.0f, -7.0f );
	volume.Radius		= 5.0f;

	CalculateWorld( entity, &volume, &worldVolume );

	vec3 volumePositionHalfTurned = vec3( -volume.Position.x, -volume.Position.y, volume.Position.z );

	EXPECT_EQ( entity.Scale * volumePositionHalfTurned + entity.Position, worldVolume.Position );
	EXPECT_EQ( entity.Scale * volume.Radius, worldVolume.Radius );
}

TEST( CollisionPrimitives, MultipleVolumesAndFrames )
{
	Entity entity;
	entity.SetPosition( vec3( 7.0f, 5.0f, -3.0f ) );
	entity.SetOrientation( quat( 0.0f, 0.0f, 1.0f, 0.0f ) );	// 180 degree rotation around y.
	entity.SetScale( 3.0f );

	Ray* ray			= pNew( Ray );	// Will be deleted by entity.
	ray->Position		= vec3( 1.0f, 0.0f, -3.0f );
	ray->Direction		= glm::normalize( vec3( -1.0f, 0.0f, 0.0f ) );

	Plane* plane		= pNew( Plane );	// Will be deleted by entity.
	plane->Position		= vec3( -2.0f, 5.0f, -7.0f );
	plane->Normal		= vec3( 0.0f, 0.0f, 1.0f );

	Sphere* sphere		= pNew( Sphere );	// Will be deleted by entity.
	sphere->Position	= vec3( 2.0f, 8.0f, -7.0f );
	sphere->Radius		= 5.0f;

	entity.AddStorageVolume( ray );		// Index 0 hopefully.
	entity.AddStorageVolume( plane );	// Index 1 hopefully.
	entity.AddStorageVolume( sphere );	// Index 2 hopefully.

	// "Frame 0"
	entity.CalculateWorldVolumes();

	Ray* worldRay		= static_cast<Ray*>(entity.GetWorldVolumes()[0]);
	Plane* worldPlane	= static_cast<Plane*>(entity.GetWorldVolumes()[1]);
	Sphere* worldSphere	= static_cast<Sphere*>(entity.GetWorldVolumes()[2]);

	vec3 halfTurn					= vec3( -1.0f, 1.0f, -1.0f );
	vec3 rayPositionHalfTurned		= ray->Position * halfTurn;
	vec3 planePositionHalfTurned	= plane->Position * halfTurn;
	vec3 spherePositionHalfTurned	= sphere->Position * halfTurn;

	// Ray
	EXPECT_EQ( entity.GetScale() * rayPositionHalfTurned + entity.GetPosition(), worldRay->Position );
	EXPECT_EQ( -ray->Direction, worldRay->Direction );

	// Plane
	EXPECT_EQ( entity.GetScale() * planePositionHalfTurned + entity.GetPosition(), worldPlane->Position );
	EXPECT_EQ( -plane->Normal, worldPlane->Normal );

	// Sphere
	EXPECT_EQ( entity.GetScale() * spherePositionHalfTurned + entity.GetPosition(), worldSphere->Position );
	EXPECT_EQ( entity.GetScale() * sphere->Radius, worldSphere->Radius );

	// "Frame 1"
	entity.SetPosition( vec3( -9.0f, -4.0f, 13.0f ) );
	entity.SetOrientation( quat( 0.0f, 1.0f, 0.0f, 0.0f ) );	// 180 degree rotation around x.
	entity.SetScale( 5.0f );

	entity.CalculateWorldVolumes();

	worldRay		= static_cast<Ray*>(entity.GetWorldVolumes()[0]);
	worldPlane		= static_cast<Plane*>(entity.GetWorldVolumes()[1]);
	worldSphere		= static_cast<Sphere*>(entity.GetWorldVolumes()[2]);

	halfTurn					= vec3( 1.0f, -1.0f, -1.0f );
	rayPositionHalfTurned		= ray->Position * halfTurn;
	planePositionHalfTurned		= plane->Position * halfTurn;
	spherePositionHalfTurned	= sphere->Position * halfTurn;

	// Ray
	EXPECT_EQ( entity.GetScale() * rayPositionHalfTurned + entity.GetPosition(), worldRay->Position );
	EXPECT_EQ( ray->Direction, worldRay->Direction );

	// Plane
	EXPECT_EQ( entity.GetScale() * planePositionHalfTurned + entity.GetPosition(), worldPlane->Position );
	EXPECT_EQ( -plane->Normal, worldPlane->Normal );

	// Sphere
	EXPECT_EQ( entity.GetScale() * spherePositionHalfTurned + entity.GetPosition(), worldSphere->Position );
	EXPECT_EQ( entity.GetScale() * sphere->Radius, worldSphere->Radius );
}