/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "ProfileEntry.h"
#include <algorithm>

using namespace Profiler;

#pragma push_macro("min")
#undef min
#pragma push_macro("max")
#undef max

ProfileEntry::ProfileEntry()
{
	m_Samples.resize ( m_MaxNrOfSamples );
}

void ProfileEntry::AddSample ( uint64_t val )
{
	m_MaxValue = std::max ( val, m_MaxValue );
	m_Samples.at ( m_SampleToReplace ) = val;
	m_SampleToReplace = ( m_SampleToReplace + 1 ) % m_MaxNrOfSamples;
	m_CurrentNrOfSamples = std::min ( m_MaxNrOfSamples, m_CurrentNrOfSamples + 1 );
}

float ProfileEntry::GetAverage() const
{
	uint64_t total = 0;
	for ( unsigned int i = 0; i < m_CurrentNrOfSamples; ++i )
	{
		total += m_Samples.at ( i );
	}
	return total / static_cast<float> ( m_CurrentNrOfSamples );
}

uint64_t ProfileEntry::GetMax() const
{
	return m_MaxValue;
}

void ProfileEntry::ResetMax()
{
	m_MaxValue = 0;
}

#pragma pop_macro("min")
#pragma pop_macro("max")
