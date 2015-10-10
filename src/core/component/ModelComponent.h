/**************************************************
2015 Henrik Johansson
***************************************************/

#pragma once

#include <glm/glm.hpp>

struct ModelComponent
{
	int			ModelHandle = -1;
	glm::vec3	Min	= glm::vec3(0.0f);
	glm::vec3	Max	= glm::vec3(0.0f);
	bool		IsVisible = true;
};