/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include "memory/Alloc.h"

template <typename Handle, typename Implementation, int MaxSize>
class HandleAllocator
{
private:
	int m_MaxAllocation = 0;
	rDeque<Handle> m_ToBeReAllocated;
	rVector<Implementation> m_Buffer;
	Implementation m_InvalidImplementation;
public:
	HandleAllocator( Implementation invalidImplementation )
		: m_InvalidImplementation( invalidImplementation )
	{
		m_Buffer.resize( MaxSize );
	}

	Implementation* GetEditableValue( Handle handle )
	{
		assert( static_cast<int>( handle ) < m_MaxAllocation );
		return &m_Buffer.at( static_cast<int>( handle ) );
	}

	Handle Allocate( const Implementation& implementation )
	{
		Handle handle;
		if ( m_ToBeReAllocated.size() > 0 )
		{
			handle = m_ToBeReAllocated.back( );
			m_ToBeReAllocated.pop_back( );
		}
		else
		{
			handle = static_cast<Handle>( m_MaxAllocation++ );
		}
		m_Buffer.at( static_cast<int>( handle ) ) = implementation;
		return handle;
	}

	void Remove( Handle handle )
	{
		m_Buffer.at( static_cast<int>( handle ) ) = m_InvalidImplementation;
		m_ToBeReAllocated.push_back( handle );
	}

	Implementation* Data( )
	{
		return m_Buffer.data();
	}

	int GetMaxAllocationNumber( ) const
	{
		return m_MaxAllocation;
	}

	int GetNumberOfUsedAllocations( ) const
	{
		return m_MaxAllocation - static_cast<int>( m_ToBeReAllocated.size( ) );
	}
};
