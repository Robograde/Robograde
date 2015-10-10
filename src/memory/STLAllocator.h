/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include "Allocator.h"
#include "FrameAllocator.h"
#ifdef DEBUG_MEMORY
#include <typeinfo>
#endif

namespace MemoryAllocator
{
	template<class T> class STLAllocator;

	template <>
	class STLAllocator<void>
	{
	public:
		typedef void*       pointer;
		typedef const void* const_pointer;
		typedef void		value_type;

		template <class U>
		struct rebind
		{
			typedef STLAllocator<U> other;
		};
	};

	template<class T>
	class STLAllocator
	{
	public:
		typedef T			value_type;
		typedef T*			pointer;
		typedef T&			reference;
		typedef const T*	const_pointer;
		typedef const T&	const_reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;

		template <class U>
		struct rebind
		{
			typedef STLAllocator<U> other;
		};

		STLAllocator( ) { }
		~STLAllocator( ) { }

		template<class U>
		STLAllocator( const STLAllocator<U>& ) { }
		STLAllocator( const STLAllocator<T>& ) { }

		// Address
		pointer address( reference x ) const
		{
			return &x;
		}

		const_pointer address( const_reference x ) const
		{
			return &x;
		}

		// Allocate
		pointer allocate( size_type n, STLAllocator<void>::const_pointer hint = 0 )
		{
#ifdef DEBUG_MEMORY
			pointer ptr = rAlloc( T, n );
#if !defined(DISABLE_ALLOCATOR)
			Header* header = ((Header*) ptr) - 1;
			strcpy( header->File, "[C=RED](STL) [C=BLUE]");
			strcpy( header->File+strlen(header->File), typeid(pointer).name( ));
			header->Line = ((~0U) >> 1);
#endif
			return ptr;
#else
			return rAlloc( T, n );
#endif
		}

		// Deallocate
		void deallocate( pointer p, size_type n )
		{
			rFree( p );
		}

		// Max size
		size_type max_size( ) const
		{
			return static_cast<size_type>(0xFFFFFFFFFFFFFFFFULL);
		}

		// Construct
		void construct( pointer p, const_reference val )
		{
			new (p) value_type( val );
		}

		// Destroy
		void destroy( pointer p )
		{
			p->~value_type( );
		}
	};

	// Comparison operators
	template<typename T, typename U>
	bool operator==(const STLAllocator<T>& a, const STLAllocator<U>& b) { return true; }

	template<typename T, typename U>
	bool operator!=(const STLAllocator<T>& a, const STLAllocator<U>& b) { return false; }
}


namespace FrameAllocator
{
	template<class T> class STLAllocator;

	template <>
	class STLAllocator<void>
	{
	public:
		typedef void*       pointer;
		typedef const void* const_pointer;
		typedef void		value_type;

		template <class U>
		struct rebind
		{
			typedef STLAllocator<U> other;
		};
	};

	template<class T>
	class STLAllocator
	{
	public:
		typedef T			value_type;
		typedef T*			pointer;
		typedef T&			reference;
		typedef const T*	const_pointer;
		typedef const T&	const_reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;

		template <class U>
		struct rebind
		{
			typedef STLAllocator<U> other;
		};

		STLAllocator( ) { }
		~STLAllocator( ) { }

		template<class U>
		STLAllocator( const STLAllocator<U>& ) { }
		STLAllocator( const STLAllocator<T>& ) { }

		// Address
		pointer address( reference x ) const
		{
			return &x;
		}

		const_pointer address( const_reference x ) const
		{
			return &x;
		}

		// Allocate
		pointer allocate( size_type n, STLAllocator<void>::const_pointer hint = 0 )
		{
			return fAlloc( T, n );
		}

		// Deallocate
		void deallocate( pointer p, size_type n )
		{
			fFree( p );
		}

		// Max size
		size_type max_size( ) const
		{
			return static_cast<size_type>(0xFFFFFFFFFFFFFFFFULL);
		}

		// Construct
		void construct( pointer p, const_reference val )
		{
			new (p) value_type( val );
		}

		// Destroy
		void destroy( pointer p )
		{
			p->~value_type( );
		}
	};

	// Comparison operators
	template<typename T, typename U>
	bool operator==(const STLAllocator<T>& a, const STLAllocator<U>& b) { return true; }

	template<typename T, typename U>
	bool operator!=(const STLAllocator<T>& a, const STLAllocator<U>& b) { return false; }
}