/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "memory/Alloc.h"

namespace StringUtility
{
	bool IsDigitsOnly( const rString& str )
	{
		if ( str != "" )
			return std::all_of( str.begin(), str.end(), ::isdigit );
		else
			return false;
	}
}