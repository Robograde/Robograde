/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include "Allocator.h"

#include "utility/PlatformDefinitions.h"
#if PLATFORM == PLATFORM_WINDOWS
#include "utility/RetardedWindowsIncludes.h"
#endif

namespace MemoryAllocator
{
	void DumpMemory( bool high )
	{
#ifdef DEBUG_MEMORY
		struct alloc
		{
			bool		used;
			size_t		size;
			std::string	file;
			size_t		line;
		};

		std::string lowAllocations;
		std::string highAllocations;
		std::string stats;

		size_t freeMem		= 0ULL;
		size_t usedMem		= 0ULL;
		size_t countAlloc	= 0ULL;
		size_t activeAlloc	= 0ULL;

		alloc headers ={ true, sizeof( Header ), "__ALLOCATION_HEADER__", 0 };

		std::vector<alloc> allocations;

		// Collect data
		Header* header = high ? GetTop( ) : GetBottom( );
		while ( header )
		{
			alloc a;
			a.used = !header->Free;
			a.size = header->Size;
			a.file = header->File;
			a.line = header->Line;

			if ( a.used )
			{
				usedMem += a.size;
				++activeAlloc;
			}
			else
			{
				freeMem += a.size;
			}

			++countAlloc;

			allocations.push_back( headers );
			allocations.push_back( a );

			header = header->Next;
		}

		// Summarize data
		std::map<std::string, size_t> summaryFull, summaryActive;
		for ( auto& it : allocations )
		{
			if ( summaryFull.find( it.file ) != summaryFull.end( ) )
				summaryFull[it.file] += it.size;
			else
				summaryFull[it.file] = it.size;

			if ( it.used )
			{
				if ( summaryActive.find( it.file ) != summaryActive.end( ) )
					summaryActive[it.file] += it.size;
				else
					summaryActive[it.file] = it.size;
			}
		}

		auto flipper = [] ( const std::pair<std::string, size_t>& p ) -> std::pair < size_t, std::string > { return{ p.second, p.first }; };

		std::multimap<size_t, std::string> sortedFull, sortedActive;
		std::transform( summaryFull.begin( ), summaryFull.end( ), std::inserter( sortedFull, sortedFull.begin( ) ), flipper );
		std::transform( summaryActive.begin( ), summaryActive.end( ), std::inserter( sortedActive, sortedActive.begin( ) ), flipper );

		// Output data
		std::ofstream f;
		f.open( "MemoryDump.txt" );

		f << "\n#########################################################\n\n";
		f << "  " << "Used blocks\t" << (activeAlloc) << "\n";
		f << "  " << "Total blocks\t" << (countAlloc) << "\n";
		f << "  " << "Used size\t" << (usedMem) << "\n";
		f << "  " << "Free size\t" << (freeMem) << "\n";
		f << "\n#########################################################\n\n";
		f << "  " << "Active Allocations Summary" << "\n\n";
		for ( auto& it : sortedActive )
			f << "  " << it.first << "\t\t" << it.second << "\n";
		f << "\n#########################################################\n\n";
		f << "  " << "All Allocations Summary" << "\n\n";
		for ( auto& it : sortedFull )
			f << "  " << it.first << "\t\t" << it.second << "\n";
		f << "\n#########################################################\n\n";
		f << "  " << "All Allocations" << "\n\n";
		for ( auto& it : allocations )
			f << "  " << (it.used) << "\t" << (it.size) << "\t\t" << (it.file) << ":" << (it.line) << "\n";
		f << "\n#########################################################\n\n";

		f.close( );
#endif
 	}
}

namespace MemoryAllocator
{
	Byte*	Memory	= (Byte*) malloc( MEMORY_SIZE );
	Header*	HighPos = nullptr;
	Header*	LowPos	= nullptr;

	std::mutex MemoryLock;

	size_t MemoryAllocations	= 0ULL;
	size_t MemoryMisses			= 0ULL;

	void Initialize( )
	{
		memset( (void*) Memory, 0, MEMORY_SIZE );
		ResetPos( );
	}

	void Finish( )
	{
		free( (void*) Memory );
	}

	void PrintAllocations( )
	{
		GetLock( ).lock( );
#ifdef DEBUG_MEMORY
		Header* header = GetBottom( )->Next;

		std::string str = "\n";
		while ( header )
		{
			if ( !header->Free )
				str += "\tMEMORY LEAK\t\t" + std::string( header->File ) + ":" + std::to_string( header->Line ) + "\t\t" + std::to_string( header->Size ) + "\n";

			header = header->Next;
		}
		header = GetTop( )->Next;
		while ( header )
		{
			if ( !header->Free )
				str += "\tMEMORY LEAK\t\t" + std::string( header->File ) + ":" + std::to_string( header->Line ) + "\t\t" + std::to_string( header->Size ) + "\n";

			header = header->Next;
		}
		str += "\n";
#if PLATFORM == PLATFORM_WINDOWS
		OutputDebugStringA( str.c_str( ) );
#else
		printf( "%s", str.c_str( ) );
#endif
#endif
		GetLock( ).unlock( );
	}

	void VerifyPointer( void* ptr )
	{
#ifdef DEBUG_MEMORY
		MemoryLock.lock( );
		Header* header = ((Header*) ptr) - 1;
		if ( header->Next )
		{
			assert( header == header->Next->Prev ); // H <-> N     NN
			if ( header->Next->Next )
				assert( header->Next == header->Next->Next->Prev ); // H     N <-> NN
		}
		if ( header->Prev )
		{
			assert( header == header->Prev->Next ); // PP     P <-> H
			if ( header->Prev->Prev )
				assert( header->Prev == header->Prev->Prev->Next ); // PP <-> P     H
		}
		MemoryLock.unlock( );
#endif
    }

	size_t& NrAllocations( )
	{
		return MemoryAllocations;
	}

	size_t& NrMisses( )
	{
		return MemoryMisses;
	}

	std::mutex& GetLock( )
	{
		return MemoryLock;
	}

	Header* GetBottom( )
	{
		return (Header*) Memory;
	}

	Header* GetTop( )
	{
		return (Header*) (Memory + MEMORY_SIZE - sizeof( Header ));
	}

	Header*& GetHigh( )
	{
		return HighPos;
	}

	Header*& GetLow( )
	{
		return LowPos;
	}

	void ResetPos( )
	{
		// Reset global allocation pointers to memory bounds
		MemoryLock.lock( );
		LowPos	= (Header*) Memory;
		HighPos	= (Header*) (Memory + MEMORY_SIZE - sizeof( Header ));
		MemoryLock.unlock( );
	}

	void MergeBlocks( Header* header )
	{
		// Merge with header->Next
		if ( header->Next && header->Next->Free )
		{
			if ( header < header->Next )
			{
				// Merge into header (low)
				header->Size += header->Next->Size + sizeof( Header );

				if ( LowPos == header->Next )
					LowPos = header;

				header->Next = header->Next->Next;
				if ( header->Next )
					header->Next->Prev = header;
			}
			else
			{
				// Merge into header->Next (high)
				header->Next->Size += header->Size + sizeof( Header );

				if ( HighPos == header )
					HighPos = header->Next;

				if ( header->Prev )
					header->Prev->Next = header->Next;
				header->Next->Prev = header->Prev;
				header = header->Next; // Set the new header as header (for the prev merge to work)
			}
		}

		// Merge with header->Prev
		if ( header->Prev && header->Prev->Free )
		{
			if ( header->Prev < header )
			{
				// Merge into header->Prev (low)
				header->Prev->Size += header->Size + sizeof( Header );

				if ( LowPos == header )
					LowPos = header->Prev;

				header->Prev->Next = header->Next;
				if ( header->Next )
					header->Next->Prev = header->Prev;
			}
			else
			{
				// Merge into header (high)
				header->Size += header->Prev->Size + sizeof( Header );

				if ( HighPos == header->Prev )
					HighPos = header;

				header->Prev = header->Prev->Prev;
				if ( header->Prev )
					header->Prev->Next = header;
			}
		}
	}

	Header* AllocateNew( Header* header, size_t size, bool high )
	{
		// Create a new header
		Header* newHeader = (Header*) (high
			? (((Byte*) header) - size - sizeof( Header ))
			: (((Byte*) header) + header->Size + sizeof( Header )));

#ifdef DEBUG_MEMORY
		if ( !(Memory <= (Byte*) newHeader && (Byte*) newHeader <= (Memory + MEMORY_SIZE)) )
		{
			DumpMemory( high );
			assert( false );
		}
#endif

		newHeader->Prev = header;
		newHeader->Next = header->Next;
		newHeader->Size = size;
		newHeader->Free = false;
		header->Next = newHeader;
		return newHeader;
	}

	Header* AllocateOld( Header* header, size_t size, bool high )
	{
		if ( header->Size > size + sizeof( Header ) )
		{
			Header*	nextHeader	= nullptr;
			size_t	oldSize		= header->Size;
			if ( high )
			{
				// Create new header and use it for the allocation (high)
				header->Size = oldSize - size - sizeof( Header );

				nextHeader = AllocateNew( header->Prev, size, true );

				if ( nextHeader->Next )
					nextHeader->Next->Prev = nextHeader;
				header = nextHeader;
			}
			else
			{
				// Create new header and use the previous for the allocation (low)
				header->Size = size;

				nextHeader = AllocateNew( header, oldSize - size - sizeof( Header ), false );
				nextHeader->Free = true;

				if ( nextHeader->Next )
					nextHeader->Next->Prev = nextHeader;
			}
		}

		header->Free = false;

		return header;
	}
}

namespace MemoryAllocator
{
#ifndef DEBUG_MEMORY
	template<> void* Allocate<void>( size_t count, bool high )
	{
		Byte*	ptr		= Allocate<Byte>( count, high );
#else
	template<> void* Allocate<void>( size_t count, bool high, const char* file, int line )
	{
		Byte*	ptr		= Allocate<Byte>( count, high, file, line );
#endif
		Header* header	= ((Header*) ptr) - 1;
		header->Count = 0;
		return (void*) ptr;
	}
}
