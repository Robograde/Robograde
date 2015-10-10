/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool PlaneOBB( const Volume* planeVolume, const Volume* obbVolume, glm::vec3* outIntersectionPoint );
	bool OBBPlane( const Volume* obbVolume, const Volume* planeVolume, glm::vec3* outIntersectionPoint );
};