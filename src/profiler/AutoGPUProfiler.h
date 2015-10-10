/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <GL/glew.h>
#include <SDL2/SDL_timer.h>
#include <memory/Alloc.h>
#include "GPUProfiler.h"
#include "ProfileMacro.h"

class AutoGPUProfiler
{
public:
	PROFILER_API				AutoGPUProfiler ( const rString& name, bool accumulation = false,
			Profiler::GPUProfilingType profilingType = DEFAULT_GPU_PROFILING_TYPE  );
	PROFILER_API				AutoGPUProfiler ( AutoGPUProfiler& other );
	PROFILER_API				~AutoGPUProfiler();
	PROFILER_API void			Stop();

private:
	rString		m_Name			= "SOMETHING_WENT_WONKY";
	union
	{
		GLuint		m_StartQuery;	// Used for glQueryCounter
		GLuint 		m_TimeQuery;	// Used for glBeginQuery/glEndQuery
		uint64_t 	m_StartTime;	// Used for glFinish
	};
	GLuint		m_EndQuery;		// Used for glQueryCounter
	bool		m_Stopped		= false;
	bool 		m_Accumulation	= false;
	Profiler::GPUProfilingType m_ProfilingType = DEFAULT_GPU_PROFILING_TYPE;
};
