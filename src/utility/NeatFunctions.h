/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "memory/Alloc.h"

namespace NeatFunctions
{
	template<typename t>
	rVector<t> GetVectorDiff( const rVector<t>& lhs, const rVector<t>& rhs ) // TODODB: Make this commutative
	{
		rVector<t> diff;
		for ( int i = 0; i < lhs.size(); ++i )
		{
			bool isCopy = false;
			for ( int j = 0; j < rhs.size(); ++j )
			{
				if ( lhs[i] == rhs[j] )
				{
					isCopy = true;
					break;
				}
			}
			if ( !isCopy )
			{
				diff.push_back( lhs[i] );
			}
		}
		return diff;
	}

	template<typename t>
	rVector<t> GetVectorIntersection( const rVector<t>& lhs, const rVector<t>& rhs )
	{
		rVector<t> intersection;
		for ( int i = 0; i < lhs.size(); ++i )
		{
			for ( int j = 0; j < rhs.size(); ++j )
			{
				if ( lhs[i] == rhs[j] )
				{
					intersection.push_back( lhs[i] );
					break;
				}
			}
		}
		return intersection;
	}

	template<typename t>
	int GetIndexOfElement( const t& element, const rVector<t>& list )
	{
		for ( int i = 0; i < list.size(); ++i )
		{
			if ( list[i] == element )
				return i;
		}

		return -1;
	}
	template <typename M, typename V>
	void MapKeysToVector( const M& map, V& vector )
	{
		for ( typename M::const_iterator it = map.begin(); it != map.end(); ++it )
			vector.push_back( it->first );
	}

	template <typename M, typename V>
	void MapValuesToVector( const M& map, V& vector )
	{
		for ( typename M::const_iterator it = map.begin(); it != map.end(); ++it )
			vector.push_back( it->second );
	}

	template <typename M, typename V1, typename V2>
	void MapToVector( const M& map, V1& keysVector, V2& valuesVector )
	{
		for ( typename M::const_iterator it = map.begin(); it != map.end(); ++it )
		{
			keysVector.push_back( it->first );
			valuesVector.push_back( it->second );
		}
	}
}