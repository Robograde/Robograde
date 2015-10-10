/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool FrustumSphere( const Volume* frustumVolume, const Volume* sphereVolume, glm::vec3* outIntersectionPoint );
	bool SphereFrustum( const Volume* sphereVolume, const Volume* frustumVolume, glm::vec3* outIntersectionPoint );
};