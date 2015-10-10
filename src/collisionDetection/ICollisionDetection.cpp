/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "ICollisionDetection.h"

#include "engine/CollisionDetection.h"

ICollisionDetection& ICollisionDetection::GetInstance()
{
	static CollisionDetection instance;
	return instance;
}