/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/PlatformDefinitions.h>

#if PLATFORM == PLATFORM_WINDOWS
	#ifdef MESSAGING_DLL_EXPORT
		#define MESSAGING_API __declspec(dllexport)
	#else
		#define MESSAGING_API __declspec(dllimport)
	#endif
#else
	#define MESSAGING_API
#endif
