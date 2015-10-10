/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

/**
 * \file 	AutoGPUProfiler.cpp
 * \author	toi
 * \brief	Automatically profiles a scope.
 * If you wish to stop the profiling before the scope ends, call the Stop()
 * function.
 */
#include "AutoGPUProfiler.h"
#include "Profiler.h"
#include <utility/GameTimer.h>

using namespace Profiler;

/**
 * \brief	Automatically profiles a scope, or you can call the Stop()
 * function.
 * \para	name Identifier for the profiling entry.
 * \para	accumulation Wether or not to accumulate times over a frame
 */
AutoGPUProfiler::AutoGPUProfiler( const rString& name, bool accumulation, GPUProfilingType profilingType )
	: m_Name( name ), m_Accumulation( accumulation ), m_ProfilingType( profilingType )
{
	switch ( profilingType )
	{
		case GPUProfilingType::QueryCounter:
		{
			glGenQueries(1, &m_StartQuery );
			glGenQueries(1, &m_EndQuery );
			glQueryCounter( m_StartQuery, GL_TIMESTAMP );
			if ( !accumulation )
			{
				Profiler::ProfilerManager::GetInstance().StartEntry( name );
			}
		} break;
		case GPUProfilingType::BeginEnd:
		{
			glGenQueries( 1, &m_TimeQuery );
			glBeginQuery( GL_TIME_ELAPSED, m_TimeQuery );
			if ( !accumulation )
			{
				Profiler::ProfilerManager::GetInstance().StartEntry( name );
			}
		} break;
		case GPUProfilingType::Finish:
		{
			glFinish();
			m_StartTime = SDL_GetPerformanceCounter();
		} break;
	}
}

AutoGPUProfiler::AutoGPUProfiler( AutoGPUProfiler& other )
	: m_Name( other.m_Name ), m_StartQuery( other.m_StartQuery ),
	  m_EndQuery( other.m_EndQuery ), m_Accumulation( other.m_Accumulation ) {}

AutoGPUProfiler::~AutoGPUProfiler()
{
	if ( !m_Stopped )
	{
		Stop();
	}
}

void AutoGPUProfiler::Stop()
{
	m_Stopped = true;
	switch ( m_ProfilingType )
	{
		case GPUProfilingType::QueryCounter:
		{
			glQueryCounter( m_EndQuery, GL_TIMESTAMP );
			GPUProfileEntry gpupe;
			gpupe.StartQuery = m_StartQuery;
			gpupe.EndQuery = m_EndQuery;
			gpupe.Accumulation = m_Accumulation;
			if ( !m_Accumulation )
			{
				GPUProfiler::GetInstance().AddEntry( g_Profiler.GetCurrentParentPath(), gpupe );
				g_Profiler.PopScope();
			}
			else
			{
				GPUProfiler::GetInstance().AddEntry( m_Name, gpupe );
			}
		} break;
		case GPUProfilingType::BeginEnd:
		{
			glEndQuery( GL_TIME_ELAPSED );
			GPUProfileEntry gpupe;
			gpupe.TimeQuery = m_TimeQuery;
			gpupe.Accumulation = m_Accumulation;
			if ( !m_Accumulation )
			{
				GPUProfiler::GetInstance().AddEntry( g_Profiler.GetCurrentParentPath(), gpupe );
				g_Profiler.PopScope();
			}
			else
			{
				GPUProfiler::GetInstance().AddEntry( m_Name, gpupe );
			}
		} break;
		case GPUProfilingType::Finish:
		{
			glFinish();
			uint64_t endTime = SDL_GetPerformanceCounter();
			if ( m_Accumulation )
				Profiler::ProfilerManager::GetInstance().IncrementAccumulationEntry( endTime - m_StartTime, m_Name, PROFILER_CATEGORY_GPU );
			else
				Profiler::ProfilerManager::GetInstance().EndEntry( endTime - m_StartTime, m_Name, PROFILER_CATEGORY_GPU );
		} break;
	}
}
