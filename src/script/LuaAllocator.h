/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once
#include <memory/Alloc.h>

namespace MemoryAllocator
{
	void* LuaAllocator( void* userData, void* ptr, size_t osize, size_t nsize )
	{
		// http://pgl.yoyo.org/luai/i/lua_Alloc

		auto reAlloc = [] ( void* ptr, size_t size, size_t oldSize ) -> void* 
		{ 
			if ( ptr == nullptr )
				return tMalloc( size );
			else
			{
				if ( size > (reinterpret_cast<Header*>(ptr) -1)->Size )
				{
					void* tmp = ptr;
					ptr = tMalloc( size );
					memcpy( ptr, tmp, oldSize );
					tFree( tmp );
				}
				return ptr;
			}
		};

		if ( nsize == 0 )
		{
			tFree( ptr );
			return nullptr;
		}
		else
			return reAlloc( ptr, nsize, osize );

		return nullptr;
	}
}