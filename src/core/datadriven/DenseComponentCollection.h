/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#if defined(_DEBUG) || defined(DEBUG)
#define CATCH_WILD_COMPONENTS
#endif

#include <cassert>
#include <memory/Alloc.h>

#ifdef CATCH_WILD_COMPONENTS
#include <utility/Logger.h>
#endif

#include "BaseCollection.h"

template <typename Component>
class DenseComponentCollection : public BaseCollection
{
public:
	static DenseComponentCollection<Component>& GetInstance()
	{
		static DenseComponentCollection<Component> instance;
		return instance;
	}

	DenseComponentCollection()
	{
#ifdef CATCH_WILD_COMPONENTS
		memset( m_WasConstructed, 0, COMPONENTS_MAX_DENSE / 8 );
#endif
		m_Components.resize(COMPONENTS_MAX_DENSE);
	}

	// This function will create a new component with standard values defined in the component struct
	// NOTE: This will recreate at a slot defined with Entity id, make sure this slot is unused.
	void ConstructComponent( Entity entity ) override
	{
		assert( entity < COMPONENTS_MAX_DENSE );
#ifdef CATCH_WILD_COMPONENTS
		m_WasConstructed[entity / 64] |= (1ULL << (entity % 64));
#endif
		Component* component = &m_Components.at( entity );
		// Initialize with standard values defined in the struct
		*component = Component();
	}

	void DestructComponent( Entity entity ) override
	{
		assert( entity < COMPONENTS_MAX_DENSE );
#ifdef CATCH_WILD_COMPONENTS
		m_WasConstructed[entity / 64] &= ~(1ULL << (entity % 64));
#endif
	}

	Component* GetComponent( Entity id )
	{
		assert( id < COMPONENTS_MAX_DENSE );
		return &m_Components.at( id );
	}

	rVector<Component>* GetComponents()
	{
		return &m_Components;
	}

#ifdef CATCH_WILD_COMPONENTS
	bool WasConstructed( Entity id )
	{
		return 0ULL < (m_WasConstructed[id / 64] & (1ULL << (id % 64)));
	}
#endif

private:
#ifdef CATCH_WILD_COMPONENTS
	uint64_t			m_WasConstructed[COMPONENTS_MAX_DENSE / 64];
#endif
	rVector<Component>	m_Components;
	ComponentTypeIndex	m_ComponentTypeIndex;
};

/////////////////////////////
/// Conveniency functions ///
/////////////////////////////

template <typename Component>
static Component* GetDenseComponent( Entity entity )
{
#ifdef CATCH_WILD_COMPONENTS
	if ( !DenseComponentCollection<Component>::GetInstance( ).WasConstructed( entity ) )
	{
		Logger::Log( "Entity " + rToString( entity ) + " does not have " + typeid(Component).name( ) + ".", "ComponentCollection", LogSeverity::WARNING_MSG );
		assert( false );
	}
#endif
	return DenseComponentCollection<Component>::GetInstance().GetComponent( entity );
}

template <typename Component>
static rVector<Component>* GetDenseComponentArray( )
{
	return DenseComponentCollection<Component>::GetInstance().GetComponents();
}

template <typename Component>
static EntityMask GetDenseComponentFlag( )
{
	return DenseComponentCollection<Component>::GetInstance( ).GetComponentTypeFlag( );
}

template <typename Component>
static ComponentTypeIndex GetDenseComponentTypeIndex( )
{
	return DenseComponentCollection<Component>::GetInstance( ).GetComponentTypeIndex( );
}
