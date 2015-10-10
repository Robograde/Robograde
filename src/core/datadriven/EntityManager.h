/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include <memory/Alloc.h>
#include <array>

#include "BaseCollection.h"
#include "ComponentTypes.h"

#define g_EntityManager EntityManager::GetInstance()

#define ENTITY_MANAGER_NR_OF_BITS_TO_PRINT 24

class EntityManager
{
public:
	static EntityManager& GetInstance();
	EntityManager();
	~EntityManager();

	void	Reset();

	void 	SetCollectionVector ( rVector<BaseCollection*>* collections );

	void 	RemoveComponent		( Entity entity, ComponentTypeIndex componentTypeIndex );
	void 	AddComponent		( Entity entity, ComponentTypeIndex componentTypeIndex );
	Entity 	CreateEntity		( );
	void 	RemoveEntity 		( Entity entity );

	void 	PrintMasks			( );

	const rVector<EntityMask>& GetEntityMasks() const;
	EntityMask GetEntityMask( Entity entity ) const;

private:
	rVector<BaseCollection*>*	m_Collections;
	rVector<EntityMask> 		m_EntityMasks;
	rVector<Entity> 			m_EntitySlotsFree;
	Entity		 				m_MaxEntityValue = 0;
};