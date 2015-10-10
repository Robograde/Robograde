/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "ProfileMacro.h"

/**
 * \brief	Handles tracking stats of a profiling entry.
 *
 * Tracks a set amount (10) of profiling times and the max value.
 */
namespace Profiler
{
class ProfileEntry
{
public:
	PROFILER_API ProfileEntry();

	PROFILER_API void 		AddSample	( uint64_t val );
	PROFILER_API float 		GetAverage	( ) const;
	PROFILER_API uint64_t 	GetMax		( ) const;
	PROFILER_API void 		ResetMax	( );

private:
	unsigned int 			m_MaxNrOfSamples 		= 10;
	unsigned int			m_SampleToReplace 		= 0;
	unsigned int			m_CurrentNrOfSamples	= 0;
	uint64_t				m_MaxValue 				= 0;
	rVector<uint64_t> 		m_Samples;
};
}
