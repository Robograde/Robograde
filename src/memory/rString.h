/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include "STLAllocator.h"
#include "MemoryLibraryDefine.h"

#if !defined(DISABLE_ALLOCATOR)
typedef std::basic_string < char, std::char_traits<char>, MemoryAllocator::STLAllocator<char> > rString;
typedef std::basic_stringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocator<char> > rStringStream;
typedef std::basic_istringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocator<char> > rIStringStream;
typedef std::basic_ostringstream< char, std::char_traits<char>, MemoryAllocator::STLAllocator<char> > rOStringStream;
#else
typedef std::basic_string < char, std::char_traits<char>, std::allocator<char> > rString;
typedef std::basic_stringstream< char, std::char_traits<char>, std::allocator<char> > rStringStream;
typedef std::basic_istringstream< char, std::char_traits<char>, std::allocator<char> > rIStringStream;
typedef std::basic_ostringstream< char, std::char_traits<char>, std::allocator<char> > rOStringStream;
#endif

MEMORY_API rString rToString( int value );
MEMORY_API rString rToString( long value );
MEMORY_API rString rToString( long long value );
MEMORY_API rString rToString( unsigned value );
MEMORY_API rString rToString( unsigned long value );
MEMORY_API rString rToString( unsigned long long value );
MEMORY_API rString rToString( float value );
MEMORY_API rString rToString( double value );
MEMORY_API rString rToString( long double value );
