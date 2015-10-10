/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include "DenseComponentCollection.h"
#include "EntityManager.h"
#include "../component/AgentComponent.h"
#include "../component/ResourceComponent.h"
#include "../component/HealthComponent.h"
#include "../component/DoodadComponent.h"

inline Agent* GetAgentPointer(Entity e)
{
	if (e != ENTITY_INVALID)
	{
		if (GetDenseComponentFlag<AgentComponent>() & EntityManager::GetInstance().GetEntityMask(e))
			return GetDenseComponent<AgentComponent>(e)->Agent;
	}
	return nullptr;
}

template <typename Component>
inline bool HasComponent(Entity e)
{
	if (e != ENTITY_INVALID)
	{
		if (GetDenseComponentFlag<Component>() & EntityManager::GetInstance().GetEntityMask(e))
			return true;
	}
	return false;
}