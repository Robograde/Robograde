/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "GPUProfiler.h"
#include "Profiler.h"
#include <utility/Logger.h>

using namespace Profiler;

GPUProfiler& GPUProfiler::GetInstance()
{
	static GPUProfiler gp;
	return gp;
}

void GPUProfiler::AddEntry( const rString& name, const GPUProfileEntry& entry )
{
	auto mapElement = m_GPUProfileEntries.find( name );
	if ( mapElement == m_GPUProfileEntries.end() )
	{
		rVector<GPUProfileEntry> newVec;
		newVec.push_back( entry );
		m_GPUProfileEntries.emplace( name, newVec );
	}
	else
	{
		mapElement->second.push_back( entry );
	}
}

void GPUProfiler::ProcessGPUTimes( )
{
	for ( auto& name : m_GPUProfileEntries )
	{
		for ( auto& entry : name.second )
		{
			Uint64 elapsedTime = 0;
			switch ( entry.ProfilingType )
			{
				case GPUProfilingType::QueryCounter:
				{
					GLint stopTimerAvailable = 0;
					while (!stopTimerAvailable)
					{
						glGetQueryObjectiv(entry.StartQuery, GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
					}
					GLuint64 startTime, endTime;
					glGetQueryObjectui64v ( entry.StartQuery, GL_QUERY_RESULT, &startTime );
					glGetQueryObjectui64v ( entry.EndQuery,	GL_QUERY_RESULT, &endTime );
					glDeleteQueries(1, &entry.StartQuery );
					glDeleteQueries(1, &entry.EndQuery );
					elapsedTime = endTime - startTime;
				} break;
				case GPUProfilingType::BeginEnd:
				{
					GLint stopTimerAvailable = 0;
					while (!stopTimerAvailable)
					{
						glGetQueryObjectiv(entry.TimeQuery, GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
					}
					GLuint64 queryTime;
					glGetQueryObjectui64v ( entry.TimeQuery, GL_QUERY_RESULT, &queryTime );
					glDeleteQueries(1, &entry.TimeQuery );
					elapsedTime = queryTime;
				} break;
				case GPUProfilingType::Finish:
				{
					Logger::Log( "This should not happen", "GPUProfiler", LogSeverity::ERROR_MSG );
					assert( false );
				} break;
			}
			elapsedTime = g_Profiler.NanoToTicks( elapsedTime );
			if ( entry.Accumulation )
			{
				g_Profiler.IncrementAccumulationEntry( elapsedTime, name.first, PROFILER_CATEGORY_GPU );
			}
			else
			{
				g_Profiler.EndEntry( elapsedTime, name.first, PROFILER_CATEGORY_GPU );
			}
		}
		name.second.clear();
	}
}
