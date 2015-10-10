/**************************************************
Copyright 2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"
#include "../ai/stdafx.h"

struct ResourceComponent
{
	Entity	EntityID	= ENTITY_INVALID;
	Tile*	TilePointer	= nullptr;
	int		SpawnedBy	= -1;
};
