/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include <malloc.h>
#include <assert.h>
#include <cstring>
#include <string>
#include <mutex>
#include <new>

#include "MemoryLibraryDefine.h"

//#undef DEBUG_MEMORY

#ifdef DEBUG_MEMORY
	#define MEMORY_SIZE (384ULL * 1024ULL * 1024ULL)
#else
	#define MEMORY_SIZE (128ULL * 1024ULL * 1024ULL)
#endif

//#define DEBUG_MEMORY

//#define DISABLE_ALLOCATOR

#if !defined(DISABLE_ALLOCATOR)
#ifdef DEBUG_MEMORY
#define rAlloc( Type, count )		MemoryAllocator::Allocate<Type>( (count), true,__FILE__, __LINE__ )
#define rNew( Type, ... )			new (rAlloc(Type, 1)) Type( __VA_ARGS__ )
#define rNewArray( Type, count )	MemoryAllocator::Construct<Type>( (count), true, __FILE__, __LINE__ )
#else
#define rAlloc( Type, count )		MemoryAllocator::Allocate<Type>( (count) )
#define rNew( Type, ... )			new (rAlloc(Type, 1)) Type( __VA_ARGS__ )
#define rNewArray( Type, count )	MemoryAllocator::Construct<Type>( (count) )
#endif
#define rMalloc( count )			rAlloc( void, (count) )
#define rFree( ptr )				MemoryAllocator::Deallocate( (ptr) )
#define rDelete( ptr )				MemoryAllocator::Destroy( (ptr) )
#define rDeleteArray( ptr )			rDelete( ptr )
#else
#define rAlloc( Type, count )		(Type*) malloc( sizeof(Type) * (count) )
#define rNew( Type, ... )			new Type(__VA_ARGS__)
#define rNewArray( Type, count )	new Type[(count)]
#define rMalloc( count )			malloc( (count) )
#define rFree( ptr )				free( (void*) ptr )
#define rDelete( ptr )				delete ptr
#define rDeleteArray( ptr )			delete [] ptr
#endif

namespace MemoryAllocator
{
	typedef char Byte;
	static_assert(sizeof( Byte ) == 1, "Byte requires a size of 1.");

	struct Header
	{
		Header*	Prev;
		size_t	Size;
		bool	Free;

		size_t	Count;

#ifdef DEBUG_MEMORY
		char	File[512];
		int		Line;
#endif
		Header*	Next;
	};

	MEMORY_API void Initialize( );

	MEMORY_API void Finish( );

	MEMORY_API void PrintAllocations( );

	MEMORY_API void VerifyPointer( void* ptr );

	MEMORY_API size_t& NrAllocations( );

	MEMORY_API size_t& NrMisses( );

	MEMORY_API std::mutex& GetLock( );

	MEMORY_API Header* GetBottom( );

	MEMORY_API Header* GetTop( );

	MEMORY_API Header*& GetHigh( );

	MEMORY_API Header*& GetLow( );

	MEMORY_API void ResetPos( );

	MEMORY_API void MergeBlocks( Header* header );

	MEMORY_API Header* AllocateNew( Header* header, size_t size, bool high );

	MEMORY_API Header* AllocateOld( Header* header, size_t size, bool high );

	template<typename T> // TODODP: Reduce branching?
#ifndef DEBUG_MEMORY
	T* Allocate( size_t count = 1ULL, bool high = true )
#else
	T* Allocate( size_t count = 1ULL, bool high = true, const char* file = "_UnKnown_", int line = -1 )
#endif
	{
		GetLock( ).lock( );
		Header*	header	 = high ? GetHigh( ) : GetLow( );
		size_t	size	 = count * sizeof( T );
		size			+= (size & 0x7) ? 8 - (size & 0x7) : 0; // 8-byte align

		assert( size < MEMORY_SIZE );

#ifndef DEBUG_MEMORY
		while ( (!header->Free || header->Size < size) && header->Next )
			header = header->Next;
#else
		while ( (!header->Free || header->Size < size) && header->Next )
		{
			header = header->Next;
			++NrMisses( );
		}
		++NrAllocations( );
#endif
		// Get/Update allocation header.
		if ( header->Next == nullptr && (!header->Free || header->Size < size) )
			header = AllocateNew( header, size, high );
		else
			header = AllocateOld( header, size, high );

		// Update global allocation position
		if ( high && header < GetHigh( ) )
			GetHigh( ) = header;
		else if ( !high && header > GetLow( ) )
			GetLow( ) = header;

		assert( GetLow( ) < GetHigh( ) );

#ifdef DEBUG_MEMORY
		strcpy( header->File, file );
		header->Line = line;
#endif

		header->Count = count;
		GetLock( ).unlock( );
		return (T*) (header + 1);
	}

	template<typename T>
	void Deallocate( T*& ptr )
	{
		if ( ptr != nullptr )
		{
			GetLock( ).lock( );
			Header* header = ((Header*) ptr) - 1;
			header->Free = true;
			MergeBlocks( header );
			GetLock( ).unlock( );
		}
	}

	template<typename T>
	void Deallocate( T* const& ptr )
	{
		if ( ptr != nullptr )
		{
			GetLock( ).lock( );
			Header* header = ((Header*) ptr) - 1;
			header->Free = true;
			MergeBlocks( header );
			GetLock( ).unlock( );
		}
	}

	template<typename T>
#ifndef DEBUG_MEMORY
	T* Construct( size_t count = 1ULL, bool high = true )
	{
		T* ptr = Allocate<T>( count, high );
#else
	T* Construct( size_t count = 1ULL, bool high = true, const char* file = "_UnKnown_", int line = -1 )
	{
		T* ptr = Allocate<T>( count, high, file, line );
#endif
		for ( size_t i = 0; i < count; ++i )
			new (&ptr[i]) T( );
		return ptr;
	}

	template<typename T>
	void Destroy( T*& ptr )
	{
		if ( ptr != nullptr )
		{
			Header* header	= ((Header*) ptr) - 1;
			size_t	count	= header->Count;
			for ( size_t i = 0; i < count; ++i )
				ptr[i].~T( );
			Deallocate<T>( ptr );
		}
	}

	template<typename T>
	void Destroy( T* const& ptr )
	{
		if ( ptr != nullptr )
		{
			Header* header	= ((Header*) ptr) - 1;
			size_t	count	= header->Count;
			for ( size_t i = 0; i < count; ++i )
				ptr[i].~T( );
			Deallocate<T>( ptr );
		}
	}
}

namespace MemoryAllocator
{
#ifndef DEBUG_MEMORY
	template<> MEMORY_API void* Allocate<void>( size_t count, bool high );
#else
	template<> MEMORY_API void* Allocate<void>( size_t count, bool high, const char* file, int line );
#endif
}