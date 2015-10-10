/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include <gtest/gtest.h>

#include <collisionDetection/ICollisionDetection.h>

using glm::vec3;
using glm::quat;

TEST( CollisionLibrary, EntitySetGet )
{
	ICollisionEntity* entity	= g_CollisionDetection.CreateEntity();

	const vec3	setPosition		= vec3( -3.0f, -11.0f, 7.0f );
	const quat	setOrientation	= quat( 0.0f, 0.0f, 1.0f, 0.0f );
	const float	setScale		= 2.0f;

	entity->SetPosition			( setPosition		);
	entity->SetOrientation		( setOrientation	);
	entity->SetScale			( setScale			);

	EXPECT_EQ( setPosition,		entity->GetPosition()		);
	EXPECT_EQ( setOrientation,	entity->GetOrientation()	);
	EXPECT_EQ( setScale,		entity->GetScale()			);

	g_CollisionDetection.DestroyEntity( entity );
	g_CollisionDetection.Shutdown();
}