/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <cstdint>
#include <limits>

#define ENTITY_INVALID UINT_MAX

typedef unsigned int Entity;
typedef uint64_t EntityMask;
typedef short ComponentTypeIndex;

#define COMPONENTS_MAX_DENSE 8192 * 2
