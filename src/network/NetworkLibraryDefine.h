/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/PlatformDefinitions.h>

#if PLATFORM == PLATFORM_WINDOWS
#ifdef NETWORK_DLL_EXPORT
#define NETWORK_API __declspec(dllexport)
#else
#define NETWORK_API __declspec(dllimport)
#endif
#else
#define NETWORK_API
#endif