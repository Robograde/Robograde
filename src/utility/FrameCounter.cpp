/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "FrameCounter.h"
#include <algorithm>
#include <sstream>

FrameCounter::FrameCounter()
{
	m_startTime = SDL_GetPerformanceCounter();
}

void FrameCounter::TickFrame()
{
	m_frameTimes[m_overwritePos] = SDL_GetPerformanceCounter() - m_startTime;
	m_maxFrameTime = std::max( m_frameTimes[m_overwritePos], m_maxFrameTime );
	m_nrOfFrames++;
	m_nrOfFrames = std::min( m_nrOfFrames, FRAMECOUNTER_NR_OF_SAMPLES );
	m_overwritePos = ( m_overwritePos + 1 ) % FRAMECOUNTER_NR_OF_SAMPLES;
	m_startTime = SDL_GetPerformanceCounter();
}

float FrameCounter::GetFPS() const
{
	return SDL_GetPerformanceFrequency() / static_cast<float>( m_frameTimes[ ( m_overwritePos - 1 + FRAMECOUNTER_NR_OF_SAMPLES ) %
			FRAMECOUNTER_NR_OF_SAMPLES] );
}

const rString FrameCounter::GetFPSString() const
{
	rOStringStream oss;
	oss.precision( FRAMECOUNTER_STRING_FPS_PRECISION );
	oss << std::fixed << GetFPS();
	return rString( oss.str().c_str() );
}

float FrameCounter::GetAverageFPS() const
{
	uint64_t total = 0;
	for ( unsigned int i = 0; i < m_nrOfFrames; ++i )
	{
		total += m_frameTimes[i];
	}
	return SDL_GetPerformanceFrequency() * m_nrOfFrames / ( total * 1.0f );
}

const rString FrameCounter::GetAverageFPSString() const
{
	rOStringStream oss;
	oss.precision( FRAMECOUNTER_STRING_FPS_PRECISION );
	oss << std::fixed << GetAverageFPS();
	return rString( oss.str().c_str() );
}

float FrameCounter::GetMS() const
{
	return static_cast<float>( m_frameTimes[ ( m_overwritePos - 1 + FRAMECOUNTER_NR_OF_SAMPLES ) % FRAMECOUNTER_NR_OF_SAMPLES] ) /
		   ( SDL_GetPerformanceFrequency() * 0.001f );
}

const rString FrameCounter::GetMSString() const
{
	rOStringStream oss;
	oss.precision( FRAMECOUNTER_STRING_MS_PRECISION );
	oss << std::fixed << GetMS();
	return rString( oss.str().c_str() );
}

float FrameCounter::GetMaxFrameTime() const
{
	return static_cast<float>( m_maxFrameTime / ( SDL_GetPerformanceFrequency() * 0.001f ) );
}

const rString FrameCounter::GetMaxFrameTimeString() const
{
	rOStringStream oss;
	oss.precision( FRAMECOUNTER_STRING_MAX_FRAMETIME_PRECISION );
	oss << std::fixed << GetMaxFrameTime();
	return rString( oss.str().c_str() );
}

void FrameCounter::ResetMaxFrameTime()
{
	m_maxFrameTime = 0;
}

float FrameCounter::GetAverageMS() const
{
	uint64_t total = 0;
	for ( unsigned int i = 0; i < m_nrOfFrames; ++i )
	{
		total += m_frameTimes[i];
	}
	return total / ( SDL_GetPerformanceFrequency() * 0.001f * m_nrOfFrames );
}

const rString FrameCounter::GetAverageMSString() const
{
	rOStringStream oss;
	oss.precision( FRAMECOUNTER_STRING_MS_PRECISION );
	oss << std::fixed << GetAverageMS();
	return rString( oss.str().c_str() );
}