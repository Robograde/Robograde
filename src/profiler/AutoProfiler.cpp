/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

/**
 * \file 	AutoProfiler.cpp
 * \author	toi
 * \brief	Automatically profiles a scope.
 * If you wish to stop the profiling before the scope ends, call the Stop() function.
 */
#include "AutoProfiler.h"
#include "Profiler.h"

/**
 * \brief	Automatically profiles a scope, or you can call the Stop() function.
 * \para	name Identifier for the profiling entry.
 * \para	accumulation Wether or not to accumulate times over a frame
 * \para	mainThread (Optional) If this profiling is run on a separate thread set this to true.
 */
AutoProfiler::AutoProfiler (const rString& name, Profiler::PROFILER_CATEGORY category, bool accumulation, bool mainThread )
	: m_Name ( name ), m_MainThread ( mainThread ), m_Accumulation ( accumulation ), m_Category(category)
{
	if ( mainThread && !accumulation )
	{
		Profiler::ProfilerManager::GetInstance().StartEntry ( name );
	}
	m_StartTime = SDL_GetPerformanceCounter();
}

AutoProfiler::AutoProfiler ( AutoProfiler& other )
	: m_Name ( other.m_Name ), m_StartTime ( other.m_StartTime ),
	  m_Accumulation ( other.m_Accumulation ) { }

AutoProfiler::~AutoProfiler()
{
	if ( !m_Stopped )
	{
		Stop();
	}
}

void AutoProfiler::Stop()
{
	if ( m_Accumulation )
	{
		Profiler::ProfilerManager::GetInstance().IncrementAccumulationEntry( SDL_GetPerformanceCounter() - m_StartTime, m_Name, m_Category );
	}
	else
	{
		if ( m_MainThread )
		{
			Profiler::ProfilerManager::GetInstance().EndEntry ( SDL_GetPerformanceCounter() - m_StartTime, "", m_Category );
		}
		else
		{
			Profiler::ProfilerManager::GetInstance().EndEntry ( SDL_GetPerformanceCounter() - m_StartTime, m_Name, m_Category );
		}
	}
	m_Stopped = true;
}
