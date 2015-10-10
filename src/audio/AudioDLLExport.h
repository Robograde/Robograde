/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#ifdef _WIN32
    #ifdef AUDIO_DLL_EXPORT		 // This is the define you defines in the CMakeLists.txt file
        #define AUDIO_API __declspec(dllexport) // Exports symbols
    #else
        #define AUDIO_API __declspec(dllimport) // Imports symbols
    #endif
#else
    #define AUDIO_API // Unix is not annoying :D
#endif