/**************************************************
2015 Johan Melin
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"

enum MODULE_SLOT
{
	MODULE_SLOT_TOP = 0,
	MODULE_SLOT_FRONT = 1,
	MODULE_SLOT_SIDES = 2,
	MODULE_SLOT_BACK = 3,
	MODULE_SLOT_SIZE = 4,
};

struct ParentComponent
{
	Entity Children[4];

	ParentComponent( )
	{
		for ( unsigned short i = 0; i < MODULE_SLOT_SIZE; ++i )
			Children[i] = ENTITY_INVALID;
	}
};
