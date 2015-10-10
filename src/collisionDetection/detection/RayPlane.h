/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

namespace IntersectionTest
{
	bool RayPlane( const Volume* rayVolume, const Volume* planeVolume, glm::vec3* outIntersectionPoint );
	bool PlaneRay( const Volume* planeVolume, const Volume* rayVolume, glm::vec3* outIntersectionPoint );
};