/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool RayHeightMap( const Volume* rayVolume, const Volume* heightMapVolume, glm::vec3* outIntersectionPoint );
	bool HeightMapRay( const Volume* heightMapVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint );
};