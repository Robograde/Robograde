/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <SDL2/SDL_timer.h>
#include <memory/Alloc.h>
#include "UtilityLibraryDefine.h"

#define FRAMECOUNTER_NR_OF_SAMPLES 100u
#define FRAMECOUNTER_STRING_FPS_PRECISION 0
#define FRAMECOUNTER_STRING_MS_PRECISION 2
#define FRAMECOUNTER_STRING_MAX_FRAMETIME_PRECISION 5

class FrameCounter {
public:
	UTILITY_API FrameCounter();

	UTILITY_API void TickFrame();

	UTILITY_API float GetFPS() const;
	UTILITY_API const rString GetFPSString() const;

	UTILITY_API float GetAverageFPS() const;
	UTILITY_API const rString GetAverageFPSString() const;

	UTILITY_API float GetMS() const;
	UTILITY_API const rString GetMSString() const;

	UTILITY_API float GetMaxFrameTime() const;
	UTILITY_API const rString GetMaxFrameTimeString() const;
	UTILITY_API void ResetMaxFrameTime();

	UTILITY_API float GetAverageMS() const;
	UTILITY_API const rString GetAverageMSString() const;

private:

	uint64_t		m_frameTimes[FRAMECOUNTER_NR_OF_SAMPLES];
	uint64_t 		m_maxFrameTime = 0;
	unsigned int 	m_nrOfFrames = 0;
	unsigned int	m_overwritePos = 0;
	uint64_t 		m_startTime = 0;
};
