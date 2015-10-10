/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "InputLibraryDefine.h"
#include <utility/Handle.h>

#define ACTION_IDENTIFIER_INVALID -1;

struct ActionIdentifier_Tag {};

typedef Handle<ActionIdentifier_Tag, int, -1> ActionIdentifier;

enum class INPUT_API KeyBindingType
{
	Primary,
	Secondary,
	Any
};