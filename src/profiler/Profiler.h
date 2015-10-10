/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <memory/Alloc.h>
#include "ProfileMacro.h"
#include "ProfileEntry.h"

#define g_Profiler Profiler::ProfilerManager::GetInstance()

namespace Profiler
{
enum PROFILER_CATEGORY
{
	PROFILER_CATEGORY_STANDARD,
	PROFILER_CATEGORY_MAJOR,
	PROFILER_CATEGORY_GPU,
	PROFILER_CATEGORY_SIZE,
};
class ProfilerManager
{
public:
	PROFILER_API static ProfilerManager& 	GetInstance();

	PROFILER_API void			SetFrequency( uint64_t freq );
	PROFILER_API uint64_t		NanoToTicks( uint64_t nanosecs ) const;
	PROFILER_API void			StartEntry( const rString& name );
	PROFILER_API void			EndEntry( uint64_t val, const rString& name = "", PROFILER_CATEGORY category = PROFILER_CATEGORY_STANDARD );
	PROFILER_API void 			PopScope();

	PROFILER_API void 			IncrementAccumulationEntry( uint64_t val, const rString& name, PROFILER_CATEGORY category = PROFILER_CATEGORY_STANDARD );
	// Adds accumulation entries to profiler times
	PROFILER_API void 			ResetFrame();

	PROFILER_API void			ResetMaxEntries();

	// Get formatted profiler strings for output
	PROFILER_API rString		GetFormattedAverages( float factor, const rString& suffix );
	PROFILER_API rString		GetFormattedMax( float factor, const rString& suffix );

	// Print profiler strings to cout
	PROFILER_API void			PrintAverages( float factor, const rString& suffix );
	PROFILER_API void			PrintMax( float factor, const rString& suffix );

	// Quick functions for averages
	PROFILER_API void			PrintAveragesTicks();
	PROFILER_API void			PrintAveragesSeconds();
	PROFILER_API void			PrintAveragesMilliSeconds();
	PROFILER_API void			PrintAveragesMicroSeconds();
	PROFILER_API void			PrintAveragesNanoSeconds();
	PROFILER_API rString 		GetFormattedAveragesTicks();
	PROFILER_API rString 		GetFormattedAveragesSeconds();
	PROFILER_API rString 		GetFormattedAveragesMilliSeconds();
	PROFILER_API rString 		GetFormattedAveragesMicroSeconds();
	PROFILER_API rString 		GetFormattedAveragesNanoSeconds();

	// Quick functions for max
	PROFILER_API void			PrintMaxTicks();
	PROFILER_API void			PrintMaxSeconds();
	PROFILER_API void			PrintMaxMilliSeconds();
	PROFILER_API void			PrintMaxMicroSeconds();
	PROFILER_API void			PrintMaxNanoSeconds();
	PROFILER_API rString		GetFormattedMaxTicks();
	PROFILER_API rString		GetFormattedMaxSeconds();
	PROFILER_API rString		GetFormattedMaxMilliSeconds();
	PROFILER_API rString		GetFormattedMaxMicroSeconds();
	PROFILER_API rString		GetFormattedMaxNanoSeconds();

	PROFILER_API float 			GetConversionFactorMilliSeconds() const;

	PROFILER_API const rString& GetCurrentParentPath() const;
	PROFILER_API const rMap<rString, ProfileEntry>* GetMapForCategory( PROFILER_CATEGORY category ) const;
	// Will loop through each category and return the first match to the name. In order of the enum,
	PROFILER_API const ProfileEntry* GetEntry( const rString& name ) const;

private:
	struct AccumulationEntry
	{
		uint64_t Time;
		PROFILER_CATEGORY Category;
	};
	// No external instancing allowed
	ProfilerManager ( ) {}
	ProfilerManager ( const ProfilerManager & rhs );
	~ProfilerManager ( ) {};
	ProfilerManager& operator=(const ProfilerManager & rhs);

	std::mutex							m_EntriesLock;
	rMap<rString, ProfileEntry>			m_ProfileEntries[PROFILER_CATEGORY_SIZE];
	std::mutex 							m_AccumulationEntriesLock;
	rMap<rString, AccumulationEntry> 	m_AccumulationProfileEntries;
	rString								m_CurrentParentPath	= "";
	uint64_t 							m_Frequency;
	float								m_FrequencyInv 		= 0.0f;
	float								m_FrequencyInvMs	= 0.0f;
	float								m_FrequencyInvMicro	= 0.0f;
	float								m_FrequencyInvNano	= 0.0f;
};
}
