/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool RayOBB( const Volume* rayVolume, const Volume* obbVolume, glm::vec3* outIntersectionPoint );
	bool OBBRay( const Volume* obbVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint );
};