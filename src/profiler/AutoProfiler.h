/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <SDL2/SDL_timer.h>
#include <memory/Alloc.h>
#include "Profiler.h"
#include "ProfileMacro.h"

class AutoProfiler
{
public:
	PROFILER_API		AutoProfiler ( const rString& name, Profiler::PROFILER_CATEGORY category = Profiler::PROFILER_CATEGORY_STANDARD,
			bool accumulation = false, bool mainThread = true );
	PROFILER_API		AutoProfiler ( AutoProfiler& other );
	PROFILER_API		~AutoProfiler();
	PROFILER_API void	Stop();

private:
	rString		m_Name			= "SOMETHING_WENT_WONKY";
	Uint64		m_StartTime;
	bool		m_Stopped		= false;
	bool		m_MainThread	= true;
	bool 		m_Accumulation	= false;
	Profiler::PROFILER_CATEGORY m_Category = Profiler::PROFILER_CATEGORY_STANDARD;
};
