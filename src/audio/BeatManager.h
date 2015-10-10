/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "audio/AudioDLLExport.h"
//----+-----------------+------------------------------
//----| BeatType	|
//----+-----------------+------------------------------
enum class BeatType
{
	NONE,		//Dont use at beat-timing
	BEAT_25,	//25%
	BEAT_50,	//50%
	BEAT_100,	//100%
	BEAT_200,	//200%
	BEAT_400,	//400%
	BEAT_800,	//800%
};

//+===+=================+==============================
//----| BeatManager	|
//----+-----------------+------------------------------
//----|The manager of the beat counting used in the game
//----|Used to make things move to a certain beat etc, trigger an event a given beat
//+===+================================================
class BeatManager
{
#define BEAT_STEPS 6
	//Public Functions
public:
	AUDIO_API				BeatManager();
	AUDIO_API				~BeatManager();

	AUDIO_API void			Update(const float deltaTime);
	AUDIO_API void			ResetBeatCount(void);

	///Setters
	AUDIO_API void			SetBPM(const int bpm);

	///Getters
	AUDIO_API const int		GetBPM(void) const;
	AUDIO_API const float	GetBeatTime(void) const;
	AUDIO_API const float	GetBeatProgress(BeatType type = BeatType::BEAT_100) const;
	AUDIO_API const bool	GetAtBeat(BeatType type = BeatType::BEAT_100) const;
	//Private Functions
private:

	//Private Variables
private:
	int		m_BPM;
	double	m_BPS;
	double	m_SecondsPerBeat;

	double	m_Beat[BEAT_STEPS],
			m_LastBeat[BEAT_STEPS],
			m_TimeTotal[BEAT_STEPS],
			m_TimerBeat[BEAT_STEPS];

	bool	m_BeatTriggered[BEAT_STEPS];
};