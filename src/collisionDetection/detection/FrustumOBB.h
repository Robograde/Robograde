/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool FrustumOBB( const Volume* frustumVolume, const Volume* obbVolume, glm::vec3* outIntersectionPoint );
	bool OBBFrustum( const Volume* obbVolume, const Volume* frustumVolume, glm::vec3* outIntersectionPoint );
};