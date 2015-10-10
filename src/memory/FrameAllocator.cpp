/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#include "FrameAllocator.h"
#include "Allocator.h"

namespace FrameAllocator
{
	static Byte* Memory;
	static Byte* CurrentPos;

	Byte*& GetPos( )
	{
		return CurrentPos;
	}

	bool IsPosValid( )
	{
		return CurrentPos < (Memory + FRAME_BUFFER_MEMORY_SIZE);
	}

	void Initialize( )
	{
#ifdef DEBUG_MEMORY
		Memory = (Byte*) MemoryAllocator::Allocate<void>( FRAME_BUFFER_MEMORY_SIZE, false, "_FRAME_ALLOCATOR_" );
#else
		Memory = (Byte*) MemoryAllocator::Allocate<void>( FRAME_BUFFER_MEMORY_SIZE, false );
#endif
		Reset( );
	}

	void Finish( )
	{
		MemoryAllocator::Deallocate( Memory );
	}

	void Reset( )
	{
		CurrentPos = Memory;
	}
}

namespace FrameAllocator
{
	template<> void* Allocate<void>( size_t count )
	{
		Byte* ptr = Allocate<Byte>( count );
		(*(((size_t*) ptr) - 1)) = 0ULL;
		return ptr;
	}
}