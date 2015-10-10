/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#define DEFAULT_GPU_PROFILING_TYPE Profiler::GPUProfilingType::QueryCounter

#ifdef ROBOGRADE_CPU_PROFILING
	#define PROFILE(x) x;
#else
	#define PROFILE(x)
#endif

#ifdef ROBOGRADE_GPU_PROFILING
	#define GPU_PROFILE(x) x;
#else
	#define GPU_PROFILE(x)
#endif

#ifdef _WIN32
	#ifdef PROFILER_DLL_EXPORT
		#define PROFILER_API __declspec(dllexport)
	#else
		#define PROFILER_API __declspec(dllimport)
	#endif
#else
	#define PROFILER_API
#endif
