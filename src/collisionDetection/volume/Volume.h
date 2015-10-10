/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

// Needs to be integers, used as indices for a 2d array with the collision detection algorithms.
enum VOLUME_TYPE
{
	VOLUME_TYPE_RAY,
	VOLUME_TYPE_OBB,
	VOLUME_TYPE_PLANE,
	VOLUME_TYPE_SPHERE,
	VOLUME_TYPE_FRUSTUM,
	VOLUME_TYPE_HEIGHT_MAP,
	VOLUME_TYPE_SIZE	// KEEP LAST!!! Value of size will then be automatically updated when more types are added.
};

class Volume
{
public:
	virtual						~Volume() {};	// Removes warning "delete called on 'Volume' that is abstract but has non-virtual destructor"

	virtual Volume*				CalculateWorld	( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale ) const = 0;
	virtual void				CalculateWorld	( const glm::mat4& world, const glm::vec3& position, const glm::quat& orientation, const float scale, Volume* outWorldVolume ) const = 0;

	virtual const VOLUME_TYPE	GetVolumeType	() const = 0;
};