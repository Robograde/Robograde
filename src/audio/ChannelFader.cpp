/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "ChannelFader.h"

//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
ChannelFader::ChannelFader()
{
	Reset();
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
ChannelFader::~ChannelFader()
{

}
//+----------------------------------------------------------------------------+
//|void Update(const float deltaTime)
//\----------------------------------------------------------------------------+
void ChannelFader::Update(const float deltaTime)
{
	if(m_FadeType == FadeType::NONE || m_Finished)
		return;

	m_Timer += deltaTime;

	if(m_Timer >= m_Duration)
	{
		m_Finished = true;
	}

	float progress = m_Timer / m_Duration;

	if(m_FadeType == FadeType::FADE_IN)
	{
		m_Volume = progress;
	}
	else if(m_FadeType == FadeType::FADE_OUT)
	{
		m_Volume = 1.0f - progress;
	}

	if(m_Volume < 0.0f)
		m_Volume = 0.0f;
	else if(m_Volume > 1.0f)
		m_Volume = 1.0f;
}
//+----------------------------------------------------------------------------+
//|void Reset(void)
//\----------------------------------------------------------------------------+
void ChannelFader::Reset(void)
{
	m_FadeType = FadeType::NONE;
	m_FadeEvent = FadeEvent::NONE;
	m_Duration = 0.0f;
	m_Timer = 0.0f;
	m_Volume = 0.0f;
	m_Finished = false;
	m_Active = false;
}
//+---------------------------------------------------+
//|void Start(const FadeType type, const FadeEvent event, const float currentVolume, const float duration)
//|Check if the fading may be started. Only fading between the same "types" are allowed to be combined and interupt current fading
//|PAUSE/UNPAUSE, MUTE/UNMUTE etc may be combined, while PAUSE/MUTE may not
//\---------------------------------------------------+
void ChannelFader::Start(const FadeType type, const FadeEvent event, const float currentVolume, const float duration)
{
	///Add - music being added could not have been paused / muted etc before
	///Stop - Music being stopped should override all fading
	if(event == FadeEvent::ADD || event == FadeEvent::STOP)
	{
		ActivateFading(type, event, currentVolume, duration);
	}
	///Not active - if no fading is currently active, don't care about input type
	else if(m_FadeEvent == FadeEvent::NONE)
	{
		ActivateFading(type, event, currentVolume, duration);
	}
	///Mute
	else if(m_FadeEvent == FadeEvent::MUTE)
	{
		if(event == FadeEvent::MUTE || event == FadeEvent::UNMUTE)
			ActivateFading(type, event, currentVolume, duration);
	}
	///Pause
	else if(m_FadeEvent == FadeEvent::PAUSE)
	{
		if(event == FadeEvent::PAUSE || event == FadeEvent::PAUSE)
			ActivateFading(type, event, currentVolume, duration);
	}
}
//+----------------------------------------------------------------------------+
//|const float GetVolume(void)
//\----------------------------------------------------------------------------+
const float ChannelFader::GetVolume(void)
{
	return m_Volume;
}
//+----------------------------------------------------------------------------+
//|const bool GetFinished(void)
//\----------------------------------------------------------------------------+
const bool ChannelFader::GetFinished(void)
{
	return m_Finished;
}
//+----------------------------------------------------------------------------+
//|const bool GetActive(void)
//\----------------------------------------------------------------------------+
const bool ChannelFader::GetActive(void)
{
	return m_Active;
}
//+----------------------------------------------------------------------------+
//|const FadeEvent GetEvent(void)
//\----------------------------------------------------------------------------+
const FadeEvent ChannelFader::GetEvent(void)
{
	return m_FadeEvent;
}
//+---------------------------------------------------+
//|void ActivateFading(const FadeType type, const FadeEvent event, const float currentVolume, const float duration)
//|Prepare and activate fading, set the starting volume etc 
//\---------------------------------------------------+
void ChannelFader::ActivateFading(const FadeType type, const FadeEvent event, const float currentVolume, const float duration)
{
	Reset();
	m_FadeType = type;
	m_FadeEvent = event;
	m_Duration = duration;

	if(m_FadeType == FadeType::FADE_IN)
	{
		m_Volume = currentVolume;
	}
	else if(m_FadeType == FadeType::FADE_OUT)
	{
		m_Volume = 1.0f - currentVolume;
	}

	m_Timer = m_Duration * m_Volume;

	m_Active = true;
}