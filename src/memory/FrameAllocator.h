/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once
#include "MemoryLibraryDefine.h"
#include "Allocator.h"

#define FRAME_BUFFER_MEMORY_SIZE (16ULL * 1024ULL * 1024ULL)

#ifndef DISABLE_ALLOCATOR

#define fAlloc( Type, count )		FrameAllocator::Allocate<Type>( (count) )
#define fNew( Type, ... )			new (fAlloc(Type, 1)) Type( __VA_ARGS__ )
#define fNewArray( Type, count )	FrameAllocator::Construct<Type>( (count) )
#define fMalloc( count )			fAlloc( void, (count) )
#define fFree( ptr )				
#define fDelete( ptr )				FrameAllocator::Destroy( (ptr) )
#define fDeleteArray( ptr )			fDelete( ptr )

#else

#define fAlloc( Type, count )		rAlloc( Type, (count) )
#define fNew( Type, ... )			rNew( Type, __VA_ARGS__ )
#define fNewArray( Type, count )	rNewArray( Type, (count) )
#define fMalloc( count )			rMalloc( (count) )
#define fFree( ptr )				rFree( (ptr) )
#define fDelete( ptr )				rDelete( (ptr) )
#define fDeleteArray( ptr )			rDeleteArray( (ptr) )

#endif

namespace FrameAllocator
{
	typedef char Byte;
	static_assert(sizeof( Byte ) == 1, "Byte requires a size of 1.");

	MEMORY_API Byte*& GetPos( );

	MEMORY_API bool IsPosValid( );

	MEMORY_API void Initialize( );

	MEMORY_API void Finish( );

	MEMORY_API void Reset( );

	template<typename T>
	T* Allocate( size_t count = 1ULL )
	{
		Byte*	ptr		= GetPos( );
		size_t	size	= count * sizeof( T );
		size			+= (size & 0x7) ? 8 - (size & 0x7) : 0; // 8-byte align
		GetPos( )		+= size + sizeof( size_t );

		memcpy( ptr, (void*) &count, sizeof( size_t ) );
		ptr += sizeof( size_t );

		assert( IsPosValid( ) );

		return (T*) ptr;
	}

	template<typename T>
	T* Construct( size_t count = 1ULL )
	{
		T* ptr = Allocate<T>( count );
		for ( size_t i = 0; i < count; ++i )
			new (&ptr[i]) T( );
		return ptr;
	}

	template<typename T>
	void Destroy( T*& ptr )
	{
		if ( ptr != nullptr )
		{
			size_t count = *(((size_t*) ptr) - 1);
			for ( size_t i = 0; i < count; ++i )
				ptr[i].~T( );
		}
	}
}

namespace FrameAllocator
{
	template<> MEMORY_API void* Allocate<void>( size_t count );
}