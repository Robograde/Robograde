/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#ifdef _WIN32
	#ifdef MEMORY_DLL_EXPORT
		#define MEMORY_API __declspec(dllexport)
	#else
		#define MEMORY_API __declspec(dllimport)
	#endif
#else
	#define MEMORY_API
#endif
