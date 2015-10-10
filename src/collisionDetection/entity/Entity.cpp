/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "Entity.h"

#include <cassert>
#include <glm/gtx/quaternion.hpp>	// For glm::toMat4( quat );

Entity::~Entity()
{
	for ( auto& storageVolume : m_StorageVolumes )
	{
		pDelete( storageVolume );
	}
	for ( auto& worldVolume : m_WorldVolumes )
	{
		pDelete( worldVolume );
	}
}

void Entity::AddStorageVolume( const Volume* newVolume )
{
	m_StorageVolumes.push_back( newVolume );
	m_WorldVolumes.push_back( newVolume->CalculateWorld( m_WorldMatrix, m_Position, m_Orientation, m_Scale ) );
}

void Entity::CalculateWorldVolumes()
{
	assert( m_StorageVolumes.size() == m_WorldVolumes.size() );

	this->CalculateWorldMatrix();

	for ( int i = 0; i < static_cast<int>(m_StorageVolumes.size()); ++i )
	{
		m_StorageVolumes[i]->CalculateWorld( m_WorldMatrix, m_Position, m_Orientation, m_Scale, m_WorldVolumes[i] );
	}
}
															  
const glm::vec3& Entity::GetPosition() const
{
	return m_Position;
}

const glm::quat& Entity::GetOrientation() const
{
	return m_Orientation;
}

float Entity::GetScale() const
{
	return m_Scale;
}

int Entity::GetGroupID() const
{
	return m_GroupID;
}

const unsigned int& Entity::GetUserData() const
{
	return m_UserData;
}

const rVector< Volume* >& Entity::GetWorldVolumes() const
{
	return m_WorldVolumes;
}
															  
void Entity::SetPosition( const glm::vec3& newPosition )
{
	m_Position = newPosition;
}

void Entity::SetOrientation( const glm::quat& newOrientation )
{
	m_Orientation = newOrientation;
}

void Entity::SetScale( float newScale )
{
	m_Scale = newScale;
}

void Entity::SetGroupID( int newGroupID )
{
	m_GroupID = newGroupID;
}

void Entity::SetUserData( unsigned int newUserData )
{
	m_UserData = newUserData;
}

void Entity::CalculateWorldMatrix()
{
	glm::mat4 rotation = glm::toMat4( m_Orientation );

	glm::vec4 newAxisX = rotation * glm::vec4( m_Scale,	0.0f,		0.0f,		0.0f );
	glm::vec4 newAxisY = rotation * glm::vec4( 0.0f,	m_Scale,	0.0f,		0.0f );
	glm::vec4 newAxisZ = rotation * glm::vec4( 0.0f,	0.0f,		m_Scale,	0.0f );

	m_WorldMatrix[0][0] = newAxisX.x;		m_WorldMatrix[0][1] = newAxisX.y;		m_WorldMatrix[0][2] = newAxisX.z;		m_WorldMatrix[0][3] = 0.0f;
	m_WorldMatrix[1][0] = newAxisY.x;		m_WorldMatrix[1][1] = newAxisY.y;		m_WorldMatrix[1][2] = newAxisY.z;		m_WorldMatrix[1][3] = 0.0f;
	m_WorldMatrix[2][0] = newAxisZ.x;		m_WorldMatrix[2][1] = newAxisZ.y;		m_WorldMatrix[2][2] = newAxisZ.z;		m_WorldMatrix[2][3] = 0.0f;
	m_WorldMatrix[3][0] = m_Position.x;		m_WorldMatrix[3][1] = m_Position.y;		m_WorldMatrix[3][2] = m_Position.z;		m_WorldMatrix[3][3] = 1.0f;
}