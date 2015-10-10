/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once
/*
==============================================================================

   Classic RTS camera.
   Author: Ola Enberg

==============================================================================
*/
#include "Camera.h"
#include <float.h>

enum CAMERA_PAN_DIRECTION
{
	CAMERA_PAN_DIRECTION_UP,
	CAMERA_PAN_DIRECTION_DOWN,
	CAMERA_PAN_DIRECTION_LEFT,
	CAMERA_PAN_DIRECTION_RIGHT,
	CAMERA_PAN_DIRECTION_UP_LEFT,
	CAMERA_PAN_DIRECTION_UP_RIGHT,
	CAMERA_PAN_DIRECTION_DOWN_LEFT,
	CAMERA_PAN_DIRECTION_DOWN_RIGHT,
	CAMERA_PAN_DIRECTION_SIZE,
	CAMERA_PAN_DIRECTION_NONE
};

#define CAMERA_RTS_MOVEMENT_SPEED	70.0f	// Given in meters per second.
#define CAMERA_RTS_ZOOM_SPEED		4.0f	// Given in meters per scroll tick.
#define CAMERA_RTS_INITIAL_PITCH	1.1345f	// Angle camera looks down. 0*pi is into the horizon. 0.5*pi is full topdown view.
#define CAMERA_RTS_MOUSE_ZONE		5		// How many pixels from border mouse cursor needs to be in order to control camera movement.

#define CAMERA_RTS_PAN_SPEED		1.0f	// Given in meters per pixel.
#define CAMERA_RTS_PAN_MINIMUM		10.0f	// Mininum distance in pixels to start panning.
#define CAMERA_RTS_PAN_MAXIMUM		600.0f	// Maximum distance in pixels to maximize panning speed.

#define CAMERA_RTS_KEY_UP			SDL_SCANCODE_UP
#define CAMERA_RTS_KEY_DOWN			SDL_SCANCODE_DOWN
#define CAMERA_RTS_KEY_LEFT			SDL_SCANCODE_LEFT
#define CAMERA_RTS_KEY_RIGHT		SDL_SCANCODE_RIGHT

class CameraRTS : public Camera
{
public:
							CameraRTS			();

	void					Update				( float deltaTime ) override;
	
	void					MoveDown			( const float distance );
	void					MoveRight			( const float distance );
	void					Zoom				( const float distance );

	void					LookAtPosition		( const glm::vec3& targetPosition );

	CAMERA_PAN_DIRECTION	GetPanDirection		() const;

	void					SetMoveSpeed		( const float newMoveSpeed ) override;
	void					SetBoundsActive		( const bool newBoundsActive );
	void					SetMovementBounds	( const glm::vec3& min, const glm::vec3& max );

private:
	void					MoveWithMouse		( const float deltaTime );
	void					MoveWithKeyboard	( const float deltaTime );
	void					KeepWithinBounds	();

	float					m_MoveSpeed			= CAMERA_RTS_MOVEMENT_SPEED;
	float					m_ScrollSpeed		= CAMERA_RTS_ZOOM_SPEED;
	glm::ivec2				m_PanningStartPixel	= glm::ivec2( 0 );
	CAMERA_PAN_DIRECTION	m_PanDirection		= CAMERA_PAN_DIRECTION_NONE;

	bool					m_BoundsActive		= true;
	glm::vec3				m_BoundMin			= glm::vec3( -FLT_MAX );
	glm::vec3				m_BoundMax			= glm::vec3( FLT_MAX );
};
