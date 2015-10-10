/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#ifdef _WIN32
	#ifdef GFX_DLL_EXPORT
		#define GFX_API __declspec(dllexport)
	#else
		#define GFX_API __declspec(dllimport)
	#endif
	#else
		#define GFX_API
#endif