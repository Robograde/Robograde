/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "audio/AudioDLLExport.h"
//----+-----------------+------------------------------
//----| FadeType		|
//----+-----------------+------------------------------
//----| Decribes how a channel will fade 
//----+-----------------+------------------------------
enum class FadeType
{
	NONE,
	FADE_IN,
	FADE_OUT
};

//----+-----------------+------------------------------
//----| FadeEvent		|
//----+-----------------+------------------------------
//----| Decribes what will happend when a channel has faded
//----+-----------------+------------------------------
enum class FadeEvent
{
	NONE,
	ADD,
	MUTE,
	UNMUTE,
	PAUSE,
	UNPAUSE,
	STOP
};

//+===+=================+==============================
//----| ChannelFader	|
//----+-----------------+------------------------------
//----| Utility class used to manage the fading volume of channels
//+===+================================================
class ChannelFader
{
//Public Functions
public:
	AUDIO_API					ChannelFader();
	AUDIO_API					~ChannelFader();

	AUDIO_API void				Update(const float deltaTime);
	AUDIO_API void				Reset(void);

	///Setters
	AUDIO_API void				Start(const FadeType type, const FadeEvent effect, const float currentVolume, const float duration);

	///Getters
	AUDIO_API const float		GetVolume(void);
	AUDIO_API const bool		GetFinished(void);
	AUDIO_API const bool		GetActive(void);
	AUDIO_API const FadeEvent	GetEvent(void);
//Private Functions
private:
	AUDIO_API void				ActivateFading(const FadeType type, const FadeEvent effect, const float currentVolume, const float duration);

//Private Variables
private:
	FadeType		m_FadeType;

	FadeEvent		m_FadeEvent;

	float			m_Duration,
					m_Timer,
					m_Volume;

	bool			m_Finished,
					m_Active;
};