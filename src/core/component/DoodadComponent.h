/**************************************************
Copyright 2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"

struct DoodadComponent
{
	bool IsTree = false;
	bool BlockPath = true;

	float SwaySpeedDivisor = 0;
	glm::vec3 AngleVector = glm::vec3(0.1f, 1.0f, 0.1f);

};
