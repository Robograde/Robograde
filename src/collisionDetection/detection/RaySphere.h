/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool RaySphere( const Volume* rayVolume, const Volume* sphereVolume, glm::vec3* outIntersectionPoint );
	bool SphereRay( const Volume* sphereVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint );
};