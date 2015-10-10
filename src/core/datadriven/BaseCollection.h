/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "ComponentTypes.h"
#include <cassert>

class BaseCollection
{
public:
	// This will construct a component. It will not mark it as active for the entity manager.
	virtual void ConstructComponent(Entity entity) = 0;
	// This will destruct a component. It will not mark it as deactivated for the entity manager.
	virtual void DestructComponent(Entity entity) = 0;
	// Sets the component type index for this system. Don't call this!
	void SetComponentTypeIndex(ComponentTypeIndex componentTypeIndex)
	{
		m_ComponentTypeIndex = componentTypeIndex;
	}
	// Gets the unique index assigned to this collection
	ComponentTypeIndex GetComponentTypeIndex( ) const
	{
		assert( m_ComponentTypeIndex != -1 );
		return m_ComponentTypeIndex;
	}

	// Gets a flag marking this systems index, used for bitmasks.
	EntityMask GetComponentTypeFlag ( ) const
	{
		return (1ULL << GetComponentTypeIndex());
	}

private:
	ComponentTypeIndex m_ComponentTypeIndex = -1;
};
