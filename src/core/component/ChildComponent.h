/**************************************************
2015 Johan Melin
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../datadriven/ComponentTypes.h"

struct ChildComponent
{
	Entity Parent = ENTITY_INVALID;
	glm::vec3 Offset = glm::vec3(1, 1, 1);
	glm::quat Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};
