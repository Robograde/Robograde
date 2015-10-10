/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once 

//Check which platform we are compiling on
#define PLATFORM_WINDOWS	1
#define PLATFORM_MAC		2
#define PLATFORM_LINUX		3

#if defined _WIN32
#define PLATFORM PLATFORM_WINDOWS
#elif defined __APPLE__
#define PLATFORM PLATFORM_MAC
#elif defined __linux
#define PLATFORM PLATFORM_LINUX
#endif

#if PLATFORM == PLATFORM_WINDOWS
	#ifdef _DEBUG
	#define WINDOWS_DEBUG
	#endif
#endif