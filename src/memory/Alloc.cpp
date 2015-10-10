/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#include "Alloc.h"

using namespace MemoryAllocator;

MemorySystem::MemorySystem( )
{
	Initialize( );
	FrameAllocator::Initialize( );
}

MemorySystem& MemorySystem::GetInstance( )
{
	static MemorySystem instance;
	return instance;
}

MemorySystem::~MemorySystem( )
{
	FrameAllocator::Finish( );
	PrintAllocations( );
	Finish( );
}