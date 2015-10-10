/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "Channel.h"
namespace Audio
{
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
Channel::Channel(void)
	:	m_Released(true)
	,	m_Active(true)
	,	m_DistanceMin(0.0f)
	,	m_DistanceMax(0.0f)
	,	m_Volume(1.0f)
	,	m_Frequency(0.0f)
	,	m_FrequencyFactor(1.0f)
	,	m_Muted(false)
	,	m_Paused(false)
	,	m_Looping(false)
{
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
Channel::~Channel(void)
{

}
//+----------------------------------------------------------------------------+
//|void Reset(void)
//\----------------------------------------------------------------------------+
void Channel::Reset(void)
{
	m_Released = true;
	m_Active = false;
	m_DistanceMin = 0.0f;
	m_DistanceMax = 0.0f;
	m_Volume = 1.0f;
	m_Frequency = 0.0f;
	m_FrequencyFactor = 1.0f;
	m_Muted = false;
	m_Paused = false;
	m_Looping = false;
}
//+----------------------------------------------------------------------------+
//|const bool IsReleased(void)
//\----------------------------------------------------------------------------+
const bool Channel::IsReleased(void) const
{
	return m_Released;
}
//+----------------------------------------------------------------------------+
//|void Release(void)
//\----------------------------------------------------------------------------+
void Channel::Release(void)
{
	m_Released = true;
}
//+----------------------------------------------------------------------------+
//|void Lock(void)
//\----------------------------------------------------------------------------+
void Channel::Lock(void)
{
	m_Released = false;
}
//+----------------------------------------------------------------------------+
//|const bool IsActive(void)
//\----------------------------------------------------------------------------+
const bool Channel::IsActive(void) const
{
	return m_Active;
}
//+----------------------------------------------------------------------------+
//|void SetPosition(const Audio::Vector3& pos)
//\----------------------------------------------------------------------------+
void Channel::SetPosition(const Audio::Vector3& pos)
{
	m_Position = pos;
}
//+----------------------------------------------------------------------------+
//|void SetVelocity(const Audio::Vector3& vel)
//\----------------------------------------------------------------------------+
void Channel::SetVelocity(const Audio::Vector3& vel)
{
	m_Velocity = vel;
}
}