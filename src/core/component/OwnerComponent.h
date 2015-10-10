/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#define NO_OWNER_ID			-1
#define INVALID_OWNER_ID	-2

struct OwnerComponent
{
	short OwnerID = NO_OWNER_ID;

	OwnerComponent( short owner = NO_OWNER_ID ) :OwnerID( owner )
	{ }
};