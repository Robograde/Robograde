/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "BeatManager.h"
#include <cmath>

#define DEFAULT_BPM		140

//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
BeatManager::BeatManager()
{
	SetBPM(DEFAULT_BPM);

	ResetBeatCount();
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
BeatManager::~BeatManager()
{

}
//+----------------------------------------------------------------------------+
//|void Update(const float deltaTime)
//\----------------------------------------------------------------------------+
void BeatManager::Update(const float deltaTime)
{
	///Handle gigantic lags by just not caring about them, h4xx
	if(deltaTime >= m_SecondsPerBeat)
		return;

	for(int i = 0; i < BEAT_STEPS; i++)
	{
		///Calculate time since last beat
		double time = deltaTime * std::pow(2.0, i) / 4.0;
		
		m_TimeTotal[i] += time;
		m_LastBeat[i] = m_Beat[i];

		//Get progress of the beat
		m_Beat[i] = fmod(m_TimeTotal[i] / m_SecondsPerBeat, 1.0);

		if(m_Beat[i] < 0.0)
			m_Beat[i] = 0.0;

		m_TimerBeat[i] += time;

		if(m_TimerBeat[i] >= m_SecondsPerBeat)
		{
			m_TimerBeat[i] = m_TimerBeat[i] - m_SecondsPerBeat;

			m_BeatTriggered[i] = true;
		}
		else
		{
			m_BeatTriggered[i] = false;	
		}
	}
}
//+----------------------------------------------------------------------------+
//|void ResetBeatCount(void)
//\----------------------------------------------------------------------------+
void BeatManager::ResetBeatCount(void)
{
	m_BPS = m_BPM / 60.0;
	m_SecondsPerBeat = 1.0 / m_BPS;

	for(int i = 0; i < 6; i++)
	{
		m_Beat[i] = 0.0;
		m_TimeTotal[i] = 0.0;
		m_TimerBeat[i] = 0.0;
		m_BeatTriggered[i] = false;
	}
}

//+---------------------------------------------------+
//|void SetBpm(const int bpm)
//\---------------------------------------------------+
void BeatManager::SetBPM(const int bpm)
{
	m_BPM = bpm;
	ResetBeatCount();
}
//+----------------------------------------------------------------------------+
//|const int GetBPM(void)
//\----------------------------------------------------------------------------+
const int BeatManager::GetBPM(void) const
{
	return m_BPM;
}
//+----------------------------------------------------------------------------+
//|const float GetBeatTime(void)
//|Returns the time in seconds on how long a beat is
//\----------------------------------------------------------------------------+
const float	BeatManager::GetBeatTime(void) const
{
	return static_cast<float>(m_SecondsPerBeat);
}
//+----------------------------------------------------------------------------+
//|const float GetBeatProgress(BeatType beat)
//|Get the progress of the input beat, ranged from 0.0f -> 1.0f
//\----------------------------------------------------------------------------+
const float BeatManager::GetBeatProgress(BeatType type) const
{
	if(type == BeatType::NONE)
		return -1.0f;

	return static_cast<float>(m_Beat[static_cast<int>(type)-1]);
}
//+----------------------------------------------------------------------------+
//|const bool GetAtBeat(BeatType type)
//\----------------------------------------------------------------------------+
const bool BeatManager::GetAtBeat(BeatType type) const
{
	if(type == BeatType::NONE)
		return false;

	return m_BeatTriggered[static_cast<int>(type)-1];
}