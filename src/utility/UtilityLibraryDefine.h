/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#ifdef _WIN32
	#ifdef UTILITY_DLL_EXPORT
		#define UTILITY_API __declspec(dllexport)
	#else
		#define UTILITY_API __declspec(dllimport)
	#endif
#else
	#define UTILITY_API
#endif
