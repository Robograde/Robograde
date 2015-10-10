/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool PlaneSphere( const Volume* planeVolume, const Volume* sphereVolume, glm::vec3* outIntersectionPoint );
	bool SpherePlane( const Volume* sphereVolume, const Volume* planeVolume, glm::vec3* outIntersectionPoint );
};