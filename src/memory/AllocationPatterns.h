/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include <map>
#include <list>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

#include "Allocator.h"
#include "STLAllocator.h"
#include "FrameAllocator.h"

#ifndef DISABLE_ALLOCATOR
#ifdef DEBUG_MEMORY

///
///		LONG-LIVED ("PERMANENT") ALLOCATIONS
///
#define pAlloc( Type, count )		MemoryAllocator::Allocate<Type>( (count), true, __FILE__, __LINE__ )
#define pNew( Type, ... )			new (pAlloc(Type,1)) Type( __VA_ARGS__ )
#define pNewArray( Type, count )	MemoryAllocator::Construct<Type>( (count), true, __FILE__, __LINE__ )
#define pMalloc( count )			pAlloc( void, (count) )
#define pFree( ptr )				MemoryAllocator::Deallocate( (ptr) )
#define pDelete( ptr )				MemoryAllocator::Destroy( (ptr) )
#define pDeleteArray( ptr )			pDelete( (ptr) )

///
///		SHORT-LIVED ("TEMPORARY") ALLOCATIONS
///
#define tAlloc( Type, count )		MemoryAllocator::Allocate<Type>( (count), false, __FILE__, __LINE__ )
#define tNew( Type, ... )			new (tAlloc(Type,1)) Type( __VA_ARGS__ )
#define tNewArray( Type, count )	MemoryAllocator::Construct<Type>( (count), false, __FILE__, __LINE__ )
#define tMalloc( count )			tAlloc( void, (count) )
#define tFree( ptr )				MemoryAllocator::Deallocate( (ptr) )
#define tDelete( ptr )				MemoryAllocator::Destroy( (ptr) )
#define tDeleteArray( ptr )			tDelete( (ptr) )

#else

///
///		WITHOUT DEBUG DATA
///
#define pAlloc( Type, count )		MemoryAllocator::Allocate<Type>( (count), true )
#define pNew( Type, ... )			new (pAlloc(Type,1)) Type( __VA_ARGS__ )
#define pNewArray( Type, count )	MemoryAllocator::Construct<Type>( (count), true )
#define pMalloc( count )			pAlloc( void, (count) )
#define pFree( ptr )				MemoryAllocator::Deallocate( (ptr) )
#define pDelete( ptr )				MemoryAllocator::Destroy( (ptr) )
#define pDeleteArray( ptr )			pDelete( (ptr) )

#define tAlloc( Type, count )		MemoryAllocator::Allocate<Type>( (count), false )
#define tNew( Type, ... )			new (tAlloc(Type,1)) Type( __VA_ARGS__ )
#define tNewArray( Type, count )	MemoryAllocator::Construct<Type>( (count), false )
#define tMalloc( count )			tAlloc( void, (count) )
#define tFree( ptr )				MemoryAllocator::Deallocate( (ptr) )
#define tDelete( ptr )				MemoryAllocator::Destroy( (ptr) )
#define tDeleteArray( ptr )			tDelete( (ptr) )

#endif

#else

///
///		FALLBACKS
///
#define pAlloc( Type, count )		rAlloc( Type, (count) )
#define pNew( Type, ... )			rNew( Type, __VA_ARGS__ )
#define pNewArray( Type, count )	rNewArray( Type, (count) )
#define pMalloc( count )			rMalloc( (count) )
#define pFree( ptr )				rFree( (ptr) )
#define pDelete( ptr )				rDelete( (ptr) )
#define pDeleteArray( ptr )			rDeleteArray( (ptr) )

#define tAlloc( Type, count )		rAlloc( Type, (count) )
#define tNew( Type, ... )			rNew( Type, __VA_ARGS__ )
#define tNewArray( Type, count )	rNewArray( Type, (count) )
#define tMalloc( count )			rMalloc( (count) )
#define tFree( ptr )				rFree( (ptr) )
#define tDelete( ptr )				rDelete( (ptr) )
#define tDeleteArray( ptr )			rDeleteArray( (ptr) )

#endif

///
///		HIGH-MEMORY ("PERMANENT") STL ALLOCATOR
///
namespace MemoryAllocator
{
	template<class T> class STLAllocatorHigh;

	template <>
	class STLAllocatorHigh<void>
	{
	public:
		typedef void*       pointer;
		typedef const void* const_pointer;
		typedef void		value_type;

		template <class U>
		struct rebind
		{
			typedef STLAllocatorHigh<U> other;
		};
	};

	template<class T>
	class STLAllocatorHigh
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
			typedef STLAllocatorHigh<U> other;
		};

		STLAllocatorHigh( ) { }
		~STLAllocatorHigh( ) { }

		template<class U>
		STLAllocatorHigh( const STLAllocatorHigh<U>& ) { }
		STLAllocatorHigh( const STLAllocatorHigh<T>& ) { }

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
		pointer allocate( size_type n, STLAllocatorHigh<void>::const_pointer hint = 0 )
		{
#ifdef DEBUG_MEMORY
			pointer ptr = pAlloc( T, n );
#if !defined(DISABLE_ALLOCATOR)
			Header* header = ((Header*) ptr) - 1;
			strcpy( header->File, "[C=RED](STL) [C=BLUE]" );
			strcpy( header->File + strlen( header->File ), typeid(pointer).name( ) );
			header->Line = ((~0U) >> 1);
#endif
			return ptr;
#else
			return pAlloc( T, n );
#endif
		}

		// Deallocate
		void deallocate( pointer p, size_type n )
		{
			pFree( p );
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
	bool operator==(const STLAllocatorHigh<T>& a, const STLAllocatorHigh<U>& b) { return true; }

	template<typename T, typename U>
	bool operator!=(const STLAllocatorHigh<T>& a, const STLAllocatorHigh<U>& b) { return false; }
}

///
///		LOW-MEMORY ("TEMPORARY") STL ALLOCATOR
///
namespace MemoryAllocator
{
	template<class T> class STLAllocatorLow;

	template <>
	class STLAllocatorLow<void>
	{
	public:
		typedef void*       pointer;
		typedef const void* const_pointer;
		typedef void		value_type;

		template <class U>
		struct rebind
		{
			typedef STLAllocatorLow<U> other;
		};
	};

	template<class T>
	class STLAllocatorLow
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
			typedef STLAllocatorLow<U> other;
		};

		STLAllocatorLow( ) { }
		~STLAllocatorLow( ) { }

		template<class U>
		STLAllocatorLow( const STLAllocatorLow<U>& ) { }
		STLAllocatorLow( const STLAllocatorLow<T>& ) { }

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
		pointer allocate( size_type n, STLAllocatorLow<void>::const_pointer hint = 0 )
		{
#ifdef DEBUG_MEMORY
			pointer ptr = tAlloc( T, n );
#if !defined(DISABLE_ALLOCATOR)
			Header* header = ((Header*) ptr) - 1;
			strcpy( header->File, "[C=RED](STL) [C=BLUE]" );
			strcpy( header->File + strlen( header->File ), typeid(pointer).name( ) );
			header->Line = ((~0U) >> 1);
#endif
			return ptr;
#else
			return tAlloc( T, n );
#endif
		}

		// Deallocate
		void deallocate( pointer p, size_type n )
		{
			tFree( p );
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
	bool operator==(const STLAllocatorLow<T>& a, const STLAllocatorLow<U>& b) { return true; }

	template<typename T, typename U>
	bool operator!=(const STLAllocatorLow<T>& a, const STLAllocatorLow<U>& b) { return false; }
}

#include "rString.h"

#if !defined(DISABLE_ALLOCATOR)
template<typename T> using rList = class std::list < T, MemoryAllocator::STLAllocator<T> >;
template<typename T> using rDeque = class std::deque < T, MemoryAllocator::STLAllocator<T> >;
template<typename T> using rVector = class std::vector < T, MemoryAllocator::STLAllocator<T> >;
template<typename T, typename U> using rMap = std::map < T, U, std::less<T>, MemoryAllocator::STLAllocator< std::pair< const T, U > > >;
template<typename T, typename U> using rUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, MemoryAllocator::STLAllocator<std::pair< const T, U > > >;
#else
template<typename T> using rList = class std::list < T, std::allocator<T> >;
template<typename T> using rDeque = class std::deque < T, std::allocator<T> >;
template<typename T> using rVector = class std::vector < T, std::allocator<T> >;
template<typename T, typename U> using rMap = std::map < T, U, std::less<T>, std::allocator< std::pair< const T, U > > >;
template<typename T, typename U> using rUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, std::allocator<std::pair< const T, U > > >;
#endif

#ifndef DISABLE_ALLOCATOR

///
///		STL CONTAINERS
///
// String
typedef std::basic_string < char, std::char_traits<char>, MemoryAllocator::STLAllocatorHigh<char> >			pString;
typedef std::basic_string < char, std::char_traits<char>, MemoryAllocator::STLAllocatorLow<char> >			tString;
typedef std::basic_string < char, std::char_traits<char>, FrameAllocator::STLAllocator<char> >				fString;

// StringStream
typedef std::basic_stringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocatorHigh<char> >	pStringStream;
typedef std::basic_stringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocatorLow<char> >		tStringStream;
typedef std::basic_stringstream< char, std::char_traits<char>, FrameAllocator::STLAllocator<char> >			fStringStream;

// IStringStream
typedef std::basic_istringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocatorHigh<char> >	pIStringStream;
typedef std::basic_istringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocatorLow<char> >	tIStringStream;
typedef std::basic_istringstream< char, std::char_traits<char>, FrameAllocator::STLAllocator<char> >		fIStringStream;

// OStringStream
typedef std::basic_ostringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocatorHigh<char> >	pOStringStream;
typedef std::basic_ostringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocatorLow<char> >	tOStringStream;
typedef std::basic_ostringstream< char, std::char_traits<char>, FrameAllocator::STLAllocator<char> >		fOStringStream;

// List
template<typename T> using pList = class std::list < T, MemoryAllocator::STLAllocatorHigh<T> >;
template<typename T> using tList = class std::list < T, MemoryAllocator::STLAllocatorLow<T> >;
template<typename T> using fList = class std::list < T, FrameAllocator::STLAllocator<T> >;

// Deque
template<typename T> using pDeque = class std::deque < T, MemoryAllocator::STLAllocatorHigh<T> >;
template<typename T> using tDeque = class std::deque < T, MemoryAllocator::STLAllocatorLow<T> >;
template<typename T> using fDeque = class std::deque < T, FrameAllocator::STLAllocator<T> >;

// Vector
template<typename T> using pVector = class std::vector < T, MemoryAllocator::STLAllocatorHigh<T> >;
template<typename T> using tVector = class std::vector < T, MemoryAllocator::STLAllocatorLow<T> >;
template<typename T> using fVector = class std::vector < T, FrameAllocator::STLAllocator<T> >;

// Map
template<typename T, typename U> using pMap = std::map < T, U, std::less<T>, MemoryAllocator::STLAllocatorHigh< std::pair< const T, U > > >;
template<typename T, typename U> using tMap = std::map < T, U, std::less<T>, MemoryAllocator::STLAllocatorLow< std::pair< const T, U > > >;
template<typename T, typename U> using fMap = std::map < T, U, std::less<T>, FrameAllocator::STLAllocator< std::pair< const T, U > > >;

// UnorderedMap
template<typename T, typename U> using pUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, MemoryAllocator::STLAllocatorHigh<std::pair< const T, U > > >;
template<typename T, typename U> using tUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, MemoryAllocator::STLAllocatorLow<std::pair< const T, U > > >;
template<typename T, typename U> using fUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, FrameAllocator::STLAllocator<std::pair< const T, U > > >;

#else

///
///		FALLBACKS
///
// String
typedef std::basic_string < char, std::char_traits<char>, std::allocator<char> >		pString;
typedef std::basic_string < char, std::char_traits<char>, std::allocator<char> >		tString;
typedef std::basic_string < char, std::char_traits<char>, std::allocator<char> >		fString;

// StringStream
typedef std::basic_stringstream< char, std::char_traits<char>, std::allocator<char> >	pStringStream;
typedef std::basic_stringstream< char, std::char_traits<char>, std::allocator<char> >	tStringStream;
typedef std::basic_stringstream< char, std::char_traits<char>, std::allocator<char> >	fStringStream;

// IStringStream
typedef std::basic_istringstream< char, std::char_traits<char>, std::allocator<char> >	pIStringStream;
typedef std::basic_istringstream< char, std::char_traits<char>, std::allocator<char> >	tIStringStream;
typedef std::basic_istringstream< char, std::char_traits<char>, std::allocator<char> >	fIStringStream;

// OStringStream
typedef std::basic_ostringstream< char, std::char_traits<char>, std::allocator<char> >	pOStringStream;
typedef std::basic_ostringstream< char, std::char_traits<char>, std::allocator<char> >	tOStringStream;
typedef std::basic_ostringstream< char, std::char_traits<char>, std::allocator<char> >	fOStringStream;

// List
template<typename T> using pList = class std::list < T, std::allocator<T> >;
template<typename T> using tList = class std::list < T, std::allocator<T> >;
template<typename T> using fList = class std::list < T, std::allocator<T> >;

// Deque
template<typename T> using pDeque = class std::deque < T, std::allocator<T> >;
template<typename T> using tDeque = class std::deque < T, std::allocator<T> >;
template<typename T> using fDeque = class std::deque < T, std::allocator<T> >;

// Vector
template<typename T> using pVector = class std::vector < T, std::allocator<T> >;
template<typename T> using tVector = class std::vector < T, std::allocator<T> >;
template<typename T> using fVector = class std::vector < T, std::allocator<T> >;

// Map
template<typename T, typename U> using pMap = std::map < T, U, std::less<T>, std::allocator< std::pair< const T, U > > >;
template<typename T, typename U> using tMap = std::map < T, U, std::less<T>, std::allocator< std::pair< const T, U > > >;
template<typename T, typename U> using fMap = std::map < T, U, std::less<T>, std::allocator< std::pair< const T, U > > >;

// UnorderedMap
template<typename T, typename U> using pUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, std::allocator<std::pair< const T, U > > >;
template<typename T, typename U> using tUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, std::allocator<std::pair< const T, U > > >;
template<typename T, typename U> using fUnorderedMap = std::unordered_map < T, U, std::hash<T>, std::equal_to<T>, std::allocator<std::pair< const T, U > > >;

#endif

///
///		CONVERSION METHOD FOR STL CONTAINERS
///
template<typename outType, typename inType>
outType rTo( const inType& container )
{
	return outType{ container.begin( ), container.end( ) };
}

///
///		TO STRING METHODS
///
MEMORY_API pString pToString( int value );
MEMORY_API pString pToString( long value );
MEMORY_API pString pToString( long long value );
MEMORY_API pString pToString( unsigned value );
MEMORY_API pString pToString( unsigned long value );
MEMORY_API pString pToString( unsigned long long value );
MEMORY_API pString pToString( float value );
MEMORY_API pString pToString( double value );
MEMORY_API pString pToString( long double value );

MEMORY_API tString tToString( int value );
MEMORY_API tString tToString( long value );
MEMORY_API tString tToString( long long value );
MEMORY_API tString tToString( unsigned value );
MEMORY_API tString tToString( unsigned long value );
MEMORY_API tString tToString( unsigned long long value );
MEMORY_API tString tToString( float value );
MEMORY_API tString tToString( double value );
MEMORY_API tString tToString( long double value );

MEMORY_API fString fToString( int value );
MEMORY_API fString fToString( long value );
MEMORY_API fString fToString( long long value );
MEMORY_API fString fToString( unsigned value );
MEMORY_API fString fToString( unsigned long value );
MEMORY_API fString fToString( unsigned long long value );
MEMORY_API fString fToString( float value );
MEMORY_API fString fToString( double value );
MEMORY_API fString fToString( long double value );
