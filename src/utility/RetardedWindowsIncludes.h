/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "PlatformDefinitions.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <WinSock2.h>				// MUST BE INCLUDED BEFORE WINDOWS.H OR THE GATES TO COMPILER HELL WIlL SPRING WIDE OPEN! (Actually safe because of lean and mean define but anyways)
#define WINDOWS_MEAN_AND_LEAN
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#endif