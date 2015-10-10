/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#include "CameraRTS.h"

#include <input/Input.h>

CameraRTS::CameraRTS()
{
	this->PitchRelative( -CAMERA_RTS_INITIAL_PITCH );
}

void CameraRTS::Update( const float deltaTime )
{
	if ( g_Input->GetMouseInsideWindow() )
	{
		this->MoveWithMouse( deltaTime );
	}

	this->MoveWithKeyboard( deltaTime );

	if ( m_BoundsActive )
	{
		this->KeepWithinBounds();
	}
}
	
void CameraRTS::MoveDown( const float distance )
{
	this->MoveWorld( glm::vec3( 0.0f, 0.0f, distance ) );
}

void CameraRTS::MoveRight( const float distance )
{
	this->MoveWorld( glm::vec3( distance, 0.0f, 0.0f ) );
}

void CameraRTS::Zoom( const float distance )
{
	this->MoveRelative( glm::vec3( 0.0f, 0.0f, -distance ) );
}

void CameraRTS::LookAtPosition( const glm::vec3& targetPosition )
{
	const glm::vec3	forward	= this->GetForward();
	const float		factor	= (this->GetPosition().y - targetPosition.y) / forward.y;
	this->SetPosition( targetPosition + factor * forward );
}

CAMERA_PAN_DIRECTION CameraRTS::GetPanDirection() const
{
	return m_PanDirection;
}

void CameraRTS::SetMoveSpeed( const float newMoveSpeed )
{
	m_MoveSpeed = newMoveSpeed;
}

void CameraRTS::SetBoundsActive( const bool newBoundsActive )
{
	m_BoundsActive = newBoundsActive;
}

void CameraRTS::SetMovementBounds( const glm::vec3& min, const glm::vec3& max )
{
	m_BoundMin = min;
	m_BoundMax = max;
}

void CameraRTS::MoveWithMouse( const float deltaTime )
{
	m_PanDirection = CAMERA_PAN_DIRECTION_NONE;

	const int mousePositionX	= g_Input->GetMousePosX();
	const int mousePositionY	= g_Input->GetMousePosY();
	const int windowWidth		= this->GetLens().WindowWidth;
	const int windowHeight		= this->GetLens().WindowHeight;

	if ( g_Input->MouseUpDown( MOUSE_BUTTON_MIDDLE ) )
	{
		m_PanningStartPixel.x = mousePositionX;
		m_PanningStartPixel.y = mousePositionY;
	}
	else if ( g_Input->MouseDown( MOUSE_BUTTON_MIDDLE ) )
	{
		glm::vec2	direction( mousePositionX - m_PanningStartPixel.x, mousePositionY - m_PanningStartPixel.y );
		float		length = glm::length( direction );

		if ( length >= CAMERA_RTS_PAN_MINIMUM )
		{
			direction /= length;
			float speed = CAMERA_RTS_PAN_SPEED * (glm::min( length, CAMERA_RTS_PAN_MAXIMUM ) - CAMERA_RTS_PAN_MINIMUM);

			if ( direction.x != 0.0f )
			{
				this->MoveRight( deltaTime * direction.x * speed );
			}
			if ( direction.y != 0.0f )
			{
				this->MoveDown( deltaTime * direction.y * speed );
			}

			float angle = glm::atan( direction.y, direction.x );
			if ( angle <= -2.61799387799f )
				m_PanDirection = CAMERA_PAN_DIRECTION_LEFT;
			else if ( angle <= -2.35619449019f )
				m_PanDirection = CAMERA_PAN_DIRECTION_UP_LEFT;
			else if ( angle <= -1.0471975512f )
				m_PanDirection = CAMERA_PAN_DIRECTION_UP;
			else if ( angle <= -0.52359877559f )
				m_PanDirection = CAMERA_PAN_DIRECTION_UP_RIGHT;
			else if ( angle <= 0.52359877559f )
				m_PanDirection = CAMERA_PAN_DIRECTION_RIGHT;
			else if ( angle <= 1.0471975512f )
				m_PanDirection = CAMERA_PAN_DIRECTION_DOWN_RIGHT;
			else if ( angle <= 2.35619449019f )
				m_PanDirection = CAMERA_PAN_DIRECTION_DOWN;
			else if ( angle <= 2.61799387799f )
				m_PanDirection = CAMERA_PAN_DIRECTION_DOWN_LEFT;
			else
				m_PanDirection = CAMERA_PAN_DIRECTION_LEFT;
		}
	}
	else
	{
		bool up(false), down(false), left(false), right(false);

		if ( mousePositionY >= 0 && mousePositionY < CAMERA_RTS_MOUSE_ZONE )
		{
			this->MoveDown( deltaTime * -m_MoveSpeed );
			up = true;
		}
		if ( mousePositionY >= windowHeight - CAMERA_RTS_MOUSE_ZONE && mousePositionY < windowHeight )
		{
			this->MoveDown( deltaTime * m_MoveSpeed );
			down = true;
		}
		if ( mousePositionX >= 0 && mousePositionX < CAMERA_RTS_MOUSE_ZONE )
		{
			this->MoveRight( deltaTime * -m_MoveSpeed );
			left = true;
		}
		if ( mousePositionX >= windowWidth - CAMERA_RTS_MOUSE_ZONE && mousePositionX < windowWidth )
		{
			this->MoveRight( deltaTime * m_MoveSpeed );
			right = true;
		}

		if ( up )
		{
			if ( left )
				m_PanDirection = CAMERA_PAN_DIRECTION_UP_LEFT;
			else if ( right )
				m_PanDirection = CAMERA_PAN_DIRECTION_UP_RIGHT;
			else
				m_PanDirection = CAMERA_PAN_DIRECTION_UP;
		}
		else if ( down )
		{
			if ( left )
				m_PanDirection = CAMERA_PAN_DIRECTION_DOWN_LEFT;
			else if ( right )
				m_PanDirection = CAMERA_PAN_DIRECTION_DOWN_RIGHT;
			else
				m_PanDirection = CAMERA_PAN_DIRECTION_DOWN;
		}
		else if ( left )
		{
			m_PanDirection = CAMERA_PAN_DIRECTION_LEFT;
		}
		else if ( right )
		{
			m_PanDirection = CAMERA_PAN_DIRECTION_RIGHT;
		}

		// Zoom
		const int scrollY = g_Input->GetScrollY();
		if ( !m_BoundsActive || (scrollY > 0 && this->GetPosition().y > m_BoundMin.y) )
		{
			this->Zoom( CAMERA_RTS_ZOOM_SPEED * scrollY );
		}
		if ( !m_BoundsActive || (scrollY < 0 && this->GetPosition().y < m_BoundMax.y)  )
		{
			this->Zoom( CAMERA_RTS_ZOOM_SPEED * scrollY );
		}
	}
}

void CameraRTS::MoveWithKeyboard( const float deltaTime )
{
	if ( g_Input->KeyDown( CAMERA_RTS_KEY_UP ) )
	{
		this->MoveDown( deltaTime * -m_MoveSpeed );
	}
	if ( g_Input->KeyDown( CAMERA_RTS_KEY_DOWN ) )
	{
		this->MoveDown( deltaTime * m_MoveSpeed );
	}
	if ( g_Input->KeyDown( CAMERA_RTS_KEY_LEFT ) )
	{
		this->MoveRight( deltaTime * -m_MoveSpeed );
	}
	if ( g_Input->KeyDown( CAMERA_RTS_KEY_RIGHT ) )
	{
		this->MoveRight( deltaTime * m_MoveSpeed );
	}
}

void CameraRTS::KeepWithinBounds()
{
	// Keep camera within correct height. (commented out since it makes camera jump up or down the frame it hits a bound)
	//if ( this->GetPosition().y < m_BoundMin.y )
	//{
	//	this->GetEditablePosition().y = m_BoundMin.y;
	//}
	//else if ( this->GetPosition().y > m_BoundMax.y )
	//{
	//	this->GetEditablePosition().y = m_BoundMax.y;
	//}

	// Calculates point camera is looking at.
	const glm::vec3	forward				= this->GetForward();
	const float		forwardsToGround	= this->GetPosition().y / -forward.y;
	const glm::vec3	groundLookatPos		= this->GetPosition() + forwardsToGround * forward;

	// Moves camera further inside the bounds if it is currently looking outside of the bounds.
	if ( groundLookatPos.x < m_BoundMin.x )
	{
		this->GetEditablePosition().x -= groundLookatPos.x - m_BoundMin.x;
	}
	else if ( groundLookatPos.x > m_BoundMax.x )
	{
		this->GetEditablePosition().x -= groundLookatPos.x - m_BoundMax.x;
	}
	if ( groundLookatPos.z < m_BoundMin.z )
	{
		this->GetEditablePosition().z -= groundLookatPos.z - m_BoundMin.z;
	}
	else if ( groundLookatPos.z > m_BoundMax.z )
	{
		this->GetEditablePosition().z -= groundLookatPos.z - m_BoundMax.z;
	}
}