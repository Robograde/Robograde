/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <memory/Alloc.h>
#include <SDL2/SDL_timer.h>
#include <GL/glew.h>
#include "ProfileMacro.h"

namespace Profiler
{
enum class GPUProfilingType
{
	QueryCounter, 	// Will use glQueryCounter, lowest cost. May not be correct.
	BeginEnd, 		// Will use glBeginQuery and glEndQuery, medium cost. May not be correct.
	Finish 			// Will use glFinish, highest cost. Shuld be correct.
};
struct GPUProfileEntry
{
	union
	{
		GLuint 	StartQuery;
		GLuint 	TimeQuery;
	};
	GLuint 	EndQuery;
	bool 	Accumulation;
	GPUProfilingType ProfilingType = DEFAULT_GPU_PROFILING_TYPE;
};
class GPUProfiler
{
public:
	static GPUProfiler& GetInstance();

	PROFILER_API void 	AddEntry( const rString& name, const GPUProfileEntry& entry );
	PROFILER_API void	ProcessGPUTimes( );

private:
	// No external instancing allowed
	GPUProfiler ( ) {}
	GPUProfiler ( const GPUProfiler & rhs );
	~GPUProfiler ( ) {}
	GPUProfiler& operator=(const GPUProfiler & rhs);

	rMap<rString, rVector<GPUProfileEntry>> m_GPUProfileEntries;
};
}
