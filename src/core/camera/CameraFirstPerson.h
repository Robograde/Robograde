/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once
/*
==============================================================================

   First person camera used for debugging. Not made for actual gameplay.
   Author: Ola Enberg

==============================================================================
*/
#include "Camera.h"

#define CAMERA_FIRST_PERSON_RUN_SPEED	150.0f	// Given in meters per second.
#define CAMERA_FIRST_PERSON_WALK_SPEED	3.0f	// Given in meters per second.
#define CAMERA_FIRST_PERSON_TURN_SPEED	0.0033f	// Given in radians per pixel.

class CameraFirstPerson : public Camera
{
public:
	void			Update					( float deltaTime ) override;

	void			MoveForward				( const float distance );
	void			MoveRight				( const float distance );
	void			MoveUp					( const float distance );

	void			LookUp					( const float radians );
	void			LookRight				( const float radians );

	void			SetMoveSpeed			( const float newMoveSpeed ) override;
	void			SetAlternativeMoveSpeed	( const float newMoveSpeed );

private:
	float			m_MainSpeed				= CAMERA_FIRST_PERSON_RUN_SPEED;
	float			m_AlternativeSpeed		= CAMERA_FIRST_PERSON_WALK_SPEED;
	float			m_MoveSpeed				= m_MainSpeed;
};
