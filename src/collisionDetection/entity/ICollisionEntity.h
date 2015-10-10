/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

#ifdef _WIN32
	#ifdef COLLISION_DETECTION_DLL_EXPORT
		#define COLLISION_DETECTION_API __declspec(dllexport) // Exports symbols
	#else
		#define COLLISION_DETECTION_API __declspec(dllimport) // Imports symbols
	#endif
#else
	#define COLLISION_DETECTION_API // Unix is not annoying :D
#endif

class COLLISION_DETECTION_API ICollisionEntity
{
public:
	virtual const glm::vec3&		GetPosition				() const = 0;
	virtual const glm::quat&		GetOrientation			() const = 0;
	virtual float					GetScale				() const = 0;
	virtual int						GetGroupID				() const = 0;
	virtual const unsigned int&		GetUserData				() const = 0;
	
	virtual void					SetPosition				( const glm::vec3& newPosition		) = 0;
	virtual void					SetOrientation			( const glm::quat& newOrientation	) = 0;
	virtual void					SetScale				( float newScale					) = 0;
	virtual void					SetGroupID				( int newGroupID					) = 0;
	virtual void					SetUserData				( unsigned int newUserData			) = 0;

protected:
	virtual							~ICollisionEntity() {}
};