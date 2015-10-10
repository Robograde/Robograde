/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once
#include "MemoryLibraryDefine.h"
#include "Allocator.h"
#include "STLAllocator.h"
#include "FrameAllocator.h"

#include <map>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

#include "AllocationPatterns.h"

#pragma deprecated( "rNew"				)
#pragma deprecated( "rNewArray"			)
#pragma deprecated( "rAlloc"			)
#pragma deprecated( "rMalloc"			)
#pragma deprecated( "rFree"				)
#pragma deprecated( "rDelete"			)
#pragma deprecated( "rDeleteArray"		)
//#pragma deprecated( "rList"				)
//#pragma deprecated( "rDeque"			)
//#pragma deprecated( "rVector"			) // TODODP
////#pragma deprecated( "rMap"				) // TODODP
//#pragma deprecated( "rUnorderedMap"		)
//#pragma deprecated( "rString"			) // TODODP
//#pragma deprecated( "rStringStream"		) // TODODP
//#pragma deprecated( "rIStringStream"	) // TODODP
//#pragma deprecated( "rOStringStream"	) // TODPDP

#define InitMem( ) MemoryAllocator::MemorySystem::GetInstance( )

namespace MemoryAllocator
{
	class MemorySystem
	{
	public:
		MEMORY_API static MemorySystem& GetInstance( );
		MEMORY_API ~MemorySystem( );

	private:
		MemorySystem( );
		MemorySystem( const MemorySystem& );
		MemorySystem& operator=(const MemorySystem&);
	};
}

