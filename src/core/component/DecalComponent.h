/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <gfx/DecalManager.h>

struct DecalComponent
{
	gfx::Decal Decal;
	float Scale = 1.0f;
	bool AlwaysDraw = false;
	bool EnvironMentDecal = false;

	~DecalComponent( )
	{
		Decal = gfx::Decal( );
	}
};
