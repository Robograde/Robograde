/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "ChannelSDL.h"
#include "SoundSDL.h"
#include "AudioEngineSDL.h"
#include <math.h>

namespace Audio
{
#define SQRT_TWO				1.41421354f
#define	PI_HALF					3.141592653589793238f / 2

#define DEFAULT_DIST_MIN		1
#define	DEFAULT_DIST_MAX		100

#define	CHANNEL_NOT_INITIALIZED	-1

#define CONSTANT_POWER_INFLUENCE 1.0f

//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
ChannelSDL::ChannelSDL(void)
	: 	m_Sound(nullptr)
	,	m_BufferPosition(0)
{
	Reset();
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
ChannelSDL::~ChannelSDL(void)
{
	Reset();
}
//+----------------------------------------------------------------------------+
//|const bool Initialize(class AudioEngine* engine)
//\----------------------------------------------------------------------------+
const bool ChannelSDL::Initialize(class AudioEngine* engine)
{
	return true;
}
//+----------------------------------------------------------------------------+
//|void Update(const float deltaTime)
//\----------------------------------------------------------------------------+
void ChannelSDL::Update(const float deltaTime)
{
	if(IsReleased())
		return;

	if(!IsPlaying())
	{
		m_Sound->Stop(this);
		return;
	}

	UpdateVolume();
}
//+----------------------------------------------------------------------------+
//|void UpdateVolume(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::UpdateVolume(void)
{
	//Muted
	if(m_Muted)
	{
		m_VolumeLeft = 0.0f;
		m_VolumeRight = 0.0f;
	}
	//Not Muted
	else
	{
		m_VolumeLeft = m_VolumeRight = m_Volume;

		///Only set panning if the channel is 3D
		//3d
		if(GetIs3d())
		{
			m_VolumeLeft *= m_VolumeLeft * m_DistanceFactor * m_StereoLeft;
			m_VolumeRight *= m_VolumeRight * m_DistanceFactor * m_StereoRight;
		}
	}

}
//+----------------------------------------------------------------------------+
//|void Update3d(Vector3 listenerPos, Vector3 listenerForward, Vector3 listenerUp)
//\----------------------------------------------------------------------------+
void ChannelSDL::Update3d(const Vector3 listenerPos, const Vector3 listenerForward, const Vector3 listenerUp)
{
	if(IsReleased())
		return;

	if(!m_Is3D)
		return;

	UpdateDistanceVolume(listenerPos);

	UpdateStereoPanning(listenerPos, listenerForward, listenerUp);
}
//+----------------------------------------------------------------------------+
//|const bool IsPlaying(void)
//\----------------------------------------------------------------------------+
const bool ChannelSDL::IsPlaying(void) const
{
	return	!IsReleased();
}
//+----------------------------------------------------------------------------+
//|void Play(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Play(void)
{
}
//+----------------------------------------------------------------------------+
//|const bool IsPaused(void)
//\----------------------------------------------------------------------------+
const bool ChannelSDL::IsPaused(void) const
{
	return m_Paused;
}
//+----------------------------------------------------------------------------+
//|void Pause(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Pause(void)
{
	m_Paused = true;
}
//+----------------------------------------------------------------------------+
//|void Unpause(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Unpause(void)
{
	m_Paused = false;
}
//+----------------------------------------------------------------------------+
//|const bool IsMuted(void)
//\----------------------------------------------------------------------------+
const bool ChannelSDL::IsMuted(void) const
{
	return m_Muted;
}
//+----------------------------------------------------------------------------+
//|void Mute(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Mute(void)
{
	m_Muted = true;
}
//+----------------------------------------------------------------------------+
//|void Unmute(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Unmute(void)
{
	m_Muted = false;
}
//+----------------------------------------------------------------------------+
//|void Reset(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Reset(void)
{
	Channel::Reset();

	m_StereoPan = 0.0f;
	m_StereoLeft = 0.0f;
	m_StereoRight = 0.0f;
	m_DistanceFactor = 0.0f;
	m_VolumeLeft = 0.0f;
	m_VolumeRight = 0.0f;
	m_Is3D = false;

	SetMinMaxDistance(DEFAULT_DIST_MIN, DEFAULT_DIST_MAX);

	m_Sound = nullptr;
	m_BufferPosition = 0;
} 
//+----------------------------------------------------------------------------+
//|void Kill(void)
//\----------------------------------------------------------------------------+
void ChannelSDL::Kill(void)
{
	Reset();
}
//+----------------------------------------------------------------------------+
//|void SetMinMaxDistance(float min, float max)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetMinMaxDistance(float min, float max)
{
	m_DistanceMin = min;
	m_DistanceMax = max;
}
//+----------------------------------------------------------------------------+
//|void SetVolume(float vol)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetVolume(float vol)
{
	m_Volume = vol;
}
//+----------------------------------------------------------------------------+
//|void SetFrequency(float freq)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetFrequency(float freq)
{
	m_Frequency = freq;
}
//+----------------------------------------------------------------------------+
//|bool SetIs3D(bool is3D)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetIs3D(bool is3D)
{
	m_Is3D = is3D;
}
//+----------------------------------------------------------------------------+
//|void SetLooping(bool loop)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetLooping(bool loop)
{
	m_Looping = loop;
}
//+----------------------------------------------------------------------------+
//|void SetSound(class SoundSDL* sound)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetSound(class SoundSDL* sound)
{
	m_Sound = sound;
}
//+----------------------------------------------------------------------------+
//|void SetBufferPosition(const int pos)
//\----------------------------------------------------------------------------+
void ChannelSDL::SetBufferPosition(const int pos)
{
	m_BufferPosition = pos;
}
//+----------------------------------------------------------------------------+
//|const float GetVolume(void)
//\----------------------------------------------------------------------------+
const float ChannelSDL::GetVolume(void) const
{
	return m_Volume;
}
//+----------------------------------------------------------------------------+
//|const float GetFrequency(void)
//\----------------------------------------------------------------------------+
const float ChannelSDL::GetFrequency(void) const
{
	return m_Frequency;
}
//+----------------------------------------------------------------------------+
//|const float GetMinDistance(void)
//\----------------------------------------------------------------------------+
const float ChannelSDL::GetMinDistance(void) const
{
	return m_DistanceMin;
}
//+----------------------------------------------------------------------------+
//|const float GetMaxDistance(void)
//\----------------------------------------------------------------------------+
const float ChannelSDL::GetMaxDistance(void) const
{
	return m_DistanceMax;
}
//+----------------------------------------------------------------------------+
//|const Vector3 GetPosition(void)
//\----------------------------------------------------------------------------+
const Vector3 ChannelSDL::GetPosition(void) const
{
	return m_Position;
}
//+----------------------------------------------------------------------------+
//|const Vector3 GetVelocity(void)
//\----------------------------------------------------------------------------+
const Vector3 ChannelSDL::GetVelocity(void) const
{
	return m_Velocity;
}
//+----------------------------------------------------------------------------+
//|const bool GetIs3d(void)
//\----------------------------------------------------------------------------+
const bool ChannelSDL::GetIs3d(void) const
{
	return m_Is3D;
}
//+----------------------------------------------------------------------------+
//|const bool GetLooping(void)
//\----------------------------------------------------------------------------+
const bool ChannelSDL::GetLooping(void) const
{
	return m_Looping;
}
//+----------------------------------------------------------------------------+
//|SoundSDL* GetSound(void) const
//\----------------------------------------------------------------------------+
SoundSDL* ChannelSDL::GetSound(void) const
{
	return m_Sound;
}
//+----------------------------------------------------------------------------+
//|const int GetBufferPosition(void) const
//\----------------------------------------------------------------------------+
const int ChannelSDL::GetBufferPosition(void) const
{
	return m_BufferPosition;
}
//+----------------------------------------------------------------------------+
//|const float GetVolumeLeft(void) const
//\----------------------------------------------------------------------------+
const float ChannelSDL::GetVolumeLeft(void) const
{
	return m_VolumeLeft;
}
//+----------------------------------------------------------------------------+
//|const int GetVolumeRight(void) const
//\----------------------------------------------------------------------------+
const float ChannelSDL::GetVolumeRight(void) const
{
	return m_VolumeRight;
}
//+----------------------------------------------------------------------------+
//|void UpdateDistanceVolume(Vector3 listenerPos)
//\----------------------------------------------------------------------------+
void ChannelSDL::UpdateDistanceVolume(const Vector3 listenerPos)
{
	const float minDistSq = m_DistanceMin;
	const float maxDistSq = m_DistanceMax;
	const float minMaxDist = maxDistSq - minDistSq;

	Vector3 difference;
	difference.X = listenerPos.X - m_Position.X;
	difference.Y = listenerPos.Y - m_Position.Y;
	difference.Z = listenerPos.Z - m_Position.Z;
	const float distanceSq = sqrtf(difference.X * difference.X + difference.Y * difference.Y + difference.Z * difference.Z);

	if(distanceSq > maxDistSq)
	{
		m_DistanceFactor = 0.0f;
		return;
	}
	else if(distanceSq < minDistSq)
	{
		m_DistanceFactor = 1.0f;
		return;
	}

	const float factor = distanceSq / minMaxDist;
	m_DistanceFactor = 1.0f - factor;
}
//+----------------------------------------------------------------------------+
//|void UpdateStereoPanning(Vector3 listenerPos, Vector3 listenerForward, Vector3 listenerUp)
//\----------------------------------------------------------------------------+
void ChannelSDL::UpdateStereoPanning(const Vector3 listenerPos, const Vector3 listenerForward, const Vector3 listenerUp)
{
	///Get direction from listener to channel
	Vector3 direction;
	direction.X = listenerPos.X - m_Position.X;
	direction.Y = listenerPos.Y - m_Position.Y;
	direction.Z = listenerPos.Z - m_Position.Z;

	const float distance = sqrtf(direction.X * direction.X + direction.Y * direction.Y + direction.Z * direction.Z);
	direction.X = direction.X / distance;
	direction.Y = direction.Y / distance;
	direction.Z = direction.Z / distance;

	Vector3 right;

	///Cross
	right.X = listenerForward.Y * direction.Z - direction.Y * listenerForward.Z;
	right.Y = listenerForward.Z * direction.X - direction.Z * listenerForward.X;
	right.Z = listenerForward.X * direction.Y - direction.X * listenerForward.Y;

	const float dot = listenerUp.X * right.X + listenerUp.Y * right.Y + listenerUp.Z * right.Z;

	const float angle = acos(dot) - PI_HALF;
	m_StereoPan = sinf(angle);

	///Constant power
	///Only make the constant power equation decide partially how much the volume should be decreased
	m_StereoLeft = (SQRT_TWO / 2 * (cosf(m_StereoPan) + sinf(m_StereoPan)));
	m_StereoRight = (SQRT_TWO / 2 * (cosf(m_StereoPan) - sinf(m_StereoPan)));

	if(m_StereoRight < 0)
		m_StereoRight = 0;
}
}