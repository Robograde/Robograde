/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../volume/Volume.h"

typedef bool (*IntersectionTestFunction)( const Volume* aVolume, const Volume* bVolume, glm::vec3* outIntersectionPoint );

class IntersectionTestLookupTable
{
public:
									IntersectionTestLookupTable();

	IntersectionTestFunction		Fetch( VOLUME_TYPE a, VOLUME_TYPE b );

private:
	static bool						NotImplemented( const Volume* aVolume, const Volume* bVolume, glm::vec3* outIntersectionPoint );

	IntersectionTestFunction		m_IntersectionTestFunctions[VOLUME_TYPE_SIZE][VOLUME_TYPE_SIZE];
};