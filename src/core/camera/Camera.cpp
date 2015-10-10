/**************************************************
2015 Ola Enberg
***************************************************/

#include "Camera.h"

#include <glm/gtx/transform2.hpp>	// lookAt, and perspectiveFov

void Camera::CalculateViewProjection()
{
	m_Orientation	= glm::normalize( m_Orientation );
	m_View			= glm::lookAt( m_Position, m_Position + this->GetForward(), this->GetUp() );
	m_Projection	= glm::perspective( m_Lens.VerticalFOV, m_Lens.WindowWidth / static_cast<float>(m_Lens.WindowHeight), m_Lens.Near, m_Lens.Far );
}

void Camera::MoveWorld( const glm::vec3& distanceToMove )
{
	m_Position += distanceToMove;
}

void Camera::MoveRelative( const glm::vec3& distanceToMove )
{
	m_Position += distanceToMove.x * (m_Orientation * m_Right	);
	m_Position += distanceToMove.y * (m_Orientation * m_Up		);
	m_Position -= distanceToMove.z * (m_Orientation * m_Forward	);
}

void Camera::YawWorld( const float radians )
{
	RotateAroundNormalizedAxis( m_Up, radians );
}

void Camera::YawRelative( const float radians )
{
	RotateAroundNormalizedAxis( m_Orientation * m_Up, radians );
}

void Camera::PitchWorld( const float radians )
{
	RotateAroundNormalizedAxis( m_Right, radians );
}

void Camera::PitchRelative( const float radians )
{
	RotateAroundNormalizedAxis( m_Orientation * m_Right, radians );
}

void Camera::RollWorld( const float radians )
{
	RotateAroundNormalizedAxis( m_Forward, radians );
}

void Camera::RollRelative( const float radians )
{
	RotateAroundNormalizedAxis( m_Orientation * m_Forward, radians );
}

void Camera::RotateAroundNormalizedAxis( const glm::vec3& normalizedRotationAxis, const float radians )
{
	float rotationAmount = radians * 0.5f;
	glm::quat rotation( glm::cos(rotationAmount), normalizedRotationAxis * glm::sin(rotationAmount) );
	m_Orientation = glm::normalize( rotation * m_Orientation );
}

void Camera::RotateWithQuaternion( const glm::quat& rotation )
{
	m_Orientation = glm::normalize( rotation * m_Orientation );
}

const glm::vec3 Camera::GetForward() const
{
	return m_Orientation * m_Forward;
}

const glm::vec3 Camera::GetUp() const
{
	return m_Orientation * m_Up;
}

const glm::vec3 Camera::GetRight() const
{
	return m_Orientation * m_Right;
}

const glm::vec3& Camera::GetPosition() const
{
	return m_Position;
}

const glm::quat& Camera::GetOrientation() const
{
	return m_Orientation;
}

const glm::mat4& Camera::GetView() const
{
	return m_View;
}

const glm::mat4& Camera::GetProjection() const
{
	return m_Projection;
}

const glm::mat4 Camera::GetViewProjection() const
{
	return m_Projection * m_View;
}

const CameraLens& Camera::GetLens() const
{
	return m_Lens;
}

glm::vec3& Camera::GetEditablePosition()
{
	return m_Position;
}

CameraLens& Camera::GetEditableLens()
{
	return m_Lens;
}

void Camera::SetPosition( const glm::vec3& newPosition )
{
	m_Position = newPosition;
}

void Camera::SetOrientation( const glm::quat& newOrientation )
{
	m_Orientation = newOrientation;
}

void Camera::SetLens( const CameraLens& newCameraLens )
{
	m_Lens = newCameraLens;
}

void Camera::SetMoveSpeed( const float newMoveSpeed )
{
	// Just here to be overriden by its derived classes. But I don't want to force them to have an own implementation.
}