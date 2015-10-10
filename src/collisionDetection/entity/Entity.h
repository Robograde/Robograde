/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "ICollisionEntity.h"

#include <glm/mat4x4.hpp>
#include <memory/Alloc.h>
#include "../volume/Volume.h"

class Entity : public ICollisionEntity
{
public:
									~Entity					();

	void							AddStorageVolume		( const Volume* newVolume );
	void							CalculateWorldVolumes	();
															  
	const glm::vec3&				GetPosition				() const override;
	const glm::quat&				GetOrientation			() const override;
	float							GetScale				() const override;
	int								GetGroupID				() const override;
	const unsigned int&				GetUserData				() const override;
	const rVector<Volume*>&			GetWorldVolumes			() const;
															  
	void							SetPosition				( const glm::vec3& newPosition		) override;
	void							SetOrientation			( const glm::quat& newOrientation	) override;
	void							SetScale				( float newScale					) override;
	void							SetGroupID				( int newGroupID					) override;
	void							SetUserData				( unsigned int newUserData			) override;
															  
private:													  
	void							CalculateWorldMatrix	();

	glm::vec3						m_Position				= glm::vec3( 0.0f );
	glm::quat						m_Orientation			= glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
	float							m_Scale					= 1.0f;
	int								m_GroupID				= -1;
	unsigned int					m_UserData				= -1;
	glm::mat4						m_WorldMatrix			= glm::mat4( 1.0f );
	rVector<const Volume*>			m_StorageVolumes;
	rVector<Volume*>				m_WorldVolumes;
};