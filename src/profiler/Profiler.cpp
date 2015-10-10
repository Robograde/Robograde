/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "Profiler.h"
#include "GPUProfiler.h"
#include <mutex>
#include <iostream>

using namespace Profiler;

ProfilerManager& ProfilerManager::GetInstance()
{
	static ProfilerManager pm;
	return pm;
}

void ProfilerManager::SetFrequency ( uint64_t freq )
{
	m_Frequency = freq;
	m_FrequencyInv 		= 1.0f / m_Frequency;
	m_FrequencyInvMs 	= static_cast<float> ( m_FrequencyInv * 1e3 );
	m_FrequencyInvMicro	= static_cast<float> ( m_FrequencyInv * 1e6 );
	m_FrequencyInvNano 	= static_cast<float> ( m_FrequencyInv * 1e9 );
}

uint64_t ProfilerManager::NanoToTicks( uint64_t nanosecs ) const
{
	uint64_t ticks = static_cast<uint64_t>(nanosecs * m_Frequency / 1.0e9);
	return ticks;
}

void ProfilerManager::StartEntry (const rString& name )
{
	if ( m_CurrentParentPath == "" )
	{
		m_CurrentParentPath = name.c_str( );
	}
	else
	{
		m_CurrentParentPath.append( "." );
		m_CurrentParentPath.append( name.c_str( ) );
	}
}

void ProfilerManager::EndEntry ( uint64_t val, const rString& name, PROFILER_CATEGORY category )
{
	m_EntriesLock.lock();
	rMap<rString, ProfileEntry>::iterator it;
	// Include scope
	if ( name == "" )
	{
		it = m_ProfileEntries[category].find ( m_CurrentParentPath.c_str( ) );
		if ( it == m_ProfileEntries[category].end() )
		{
			// Was not found
			ProfileEntry pe;
			pe.AddSample ( val );

			m_ProfileEntries[category].emplace ( m_CurrentParentPath.c_str( ), pe );
		}
		else
		{
			// Was found
			it->second.AddSample ( val );
		}
		PopScope();
	}
	// Exclude scope. Use only name. 
	else
	{
		it = m_ProfileEntries[category].find ( name );
		if ( it == m_ProfileEntries[category].end() )
		{
			// Was not found
			ProfileEntry pe;
			pe.AddSample ( val );

			m_ProfileEntries[category].emplace ( name, pe );
		}
		else
		{
			// Was found
			it->second.AddSample ( val );
		}
	}
	m_EntriesLock.unlock();
}

void ProfilerManager::PopScope() 
{
	size_t posOfDot = m_CurrentParentPath.find_last_of ( '.' );
	if ( posOfDot == rString::npos )
	{
		m_CurrentParentPath = "";
	}
	else
	{
		m_CurrentParentPath = m_CurrentParentPath.substr ( 0, posOfDot );
	}
}

void ProfilerManager::IncrementAccumulationEntry ( uint64_t val, const rString& name, PROFILER_CATEGORY category )
{
	m_AccumulationEntriesLock.lock();

	rMap<rString, AccumulationEntry>::iterator it = m_AccumulationProfileEntries.find ( name );
	if ( it == m_AccumulationProfileEntries.end() )
	{
		m_AccumulationProfileEntries.emplace ( name, AccumulationEntry{val, category} );
	}
	else
	{
		it->second.Time += val;
	}

	m_AccumulationEntriesLock.unlock();
}

void ProfilerManager::ResetFrame()
{
	m_AccumulationEntriesLock.lock();

	for ( auto& it : m_AccumulationProfileEntries )
	{
		EndEntry ( it.second.Time, it.first, it.second.Category );
	}
	m_AccumulationProfileEntries.clear();

	m_AccumulationEntriesLock.unlock();

	GPUProfiler::GetInstance().ProcessGPUTimes();
}

void ProfilerManager::ResetMaxEntries()
{
	m_EntriesLock.lock();
	for ( auto& categoryMap : m_ProfileEntries )
	{
		for ( auto& entry : categoryMap )
		{
			entry.second.ResetMax();
		}
	}
	m_EntriesLock.unlock();
}

rString ProfilerManager::GetFormattedAverages( float factor, const rString& suffix )
{
	rString toReturn;

	m_EntriesLock.lock( );
	toReturn += ("---------- Average times in " + suffix + " ----------\n");
	for ( auto& categoryMap : m_ProfileEntries )
	{
		for ( auto& entry : categoryMap )
		{
			toReturn += ("[C=GREEN]" + entry.first + ": [C=WHITE]" + rToString( entry.second.GetAverage( ) * factor ) + " " + suffix + '\n');
		}
	}
	toReturn += "--------------------------------------\n";
	m_EntriesLock.unlock( );

	return toReturn;
}

rString ProfilerManager::GetFormattedMax( float factor, const rString& suffix )
{
	rString toReturn;

	m_EntriesLock.lock( );
	toReturn += ("----------Max times in " + suffix + "----------\n");
	for ( auto& categoryMap : m_ProfileEntries )
	{
		for ( auto& entry : categoryMap )
		{
			toReturn += ( "[C=GREEN]" + entry.first + ": [C=WHITE]" + rToString( entry.second.GetMax( ) * factor ) + " " + suffix + '\n');
		}
	}
	toReturn += "--------------------------------------\n";
	m_EntriesLock.unlock( );

	return toReturn;
}

void ProfilerManager::PrintAverages ( float factor, const rString& suffix )
{
	std::cout << GetFormattedAverages( factor, suffix );
}

void ProfilerManager::PrintMax( float factor, const rString& suffix )
{
	std::cout << GetFormattedMax( factor, suffix );
}

void ProfilerManager::PrintAveragesTicks()
{
	PrintAverages ( 1.0f, "ticks" );
}

void ProfilerManager::PrintAveragesSeconds()
{
	PrintAverages ( m_FrequencyInv, "sec" );
}

void ProfilerManager::PrintAveragesMilliSeconds()
{
	PrintAverages ( m_FrequencyInvMs, "ms" );
}

void ProfilerManager::PrintAveragesMicroSeconds()
{
	PrintAverages ( m_FrequencyInvMicro, "us" );
}

void ProfilerManager::PrintAveragesNanoSeconds()
{
	PrintAverages ( m_FrequencyInvNano, "ns" );
}

rString ProfilerManager::GetFormattedAveragesTicks( )
{
	return GetFormattedAverages( 1.0f, "ticks" );
}

rString ProfilerManager::GetFormattedAveragesSeconds( )
{
	return GetFormattedAverages( m_FrequencyInv, "sec" );
}

rString ProfilerManager::GetFormattedAveragesMilliSeconds( )
{
	return GetFormattedAverages( m_FrequencyInvMs, "ms" );
}

rString ProfilerManager::GetFormattedAveragesMicroSeconds( )
{
	return GetFormattedAverages( m_FrequencyInvMicro, "us" );
}

rString ProfilerManager::GetFormattedAveragesNanoSeconds( )
{
	return GetFormattedAverages( m_FrequencyInvNano, "ns" );
}

void ProfilerManager::PrintMaxTicks()
{
	PrintMax ( 1.0f, "ticks" );
}

void ProfilerManager::PrintMaxSeconds()
{
	PrintMax ( m_FrequencyInv, "sec" );
}

void ProfilerManager::PrintMaxMilliSeconds()
{
	PrintMax ( m_FrequencyInvMs, "ms" );
}

void ProfilerManager::PrintMaxMicroSeconds()
{
	PrintMax ( m_FrequencyInvMicro, "us" );
}

void ProfilerManager::PrintMaxNanoSeconds()
{
	PrintMax ( m_FrequencyInvNano, "ns" );
}

rString ProfilerManager::GetFormattedMaxTicks( )
{
	return GetFormattedMax( 1.0f, "ticks" );
}

rString ProfilerManager::GetFormattedMaxSeconds( )
{
	return GetFormattedMax( m_FrequencyInv, "sec" );
}

rString ProfilerManager::GetFormattedMaxMilliSeconds( )
{
	return GetFormattedMax( m_FrequencyInvMs, "ms" );
}

rString ProfilerManager::GetFormattedMaxMicroSeconds( )
{
	return GetFormattedMax( m_FrequencyInvMicro, "us");
}

rString ProfilerManager::GetFormattedMaxNanoSeconds( )
{
	return GetFormattedMax( m_FrequencyInvNano, "ns" );
}

float ProfilerManager::GetConversionFactorMilliSeconds( ) const
{
	return m_FrequencyInvMs;
}

const rString& ProfilerManager::GetCurrentParentPath( ) const
{
	return m_CurrentParentPath;
}

const rMap<rString, ProfileEntry>* ProfilerManager::GetMapForCategory( PROFILER_CATEGORY category ) const
{
	assert( category < PROFILER_CATEGORY_SIZE );
	return &m_ProfileEntries[category];
}

const ProfileEntry* ProfilerManager::GetEntry( const rString& name ) const
{
	for ( auto& category : m_ProfileEntries )
	{
		auto entry = category.find( name );
		if ( entry != category.end() )
			return &entry->second;
	}
	return nullptr;
}
