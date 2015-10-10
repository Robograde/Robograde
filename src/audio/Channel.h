/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "audio/AudioDLLExport.h"
namespace Audio
{

	//+===+================================================
	//----| ChannelFlags |
	//+===+================================================
	enum ChannelFlags
	{
		IS_3D = 0x0000001,
		LOOPING = 0x0000010,
		PAUSED = 0x0000100,
	};

	//+===+================================================
	//----| Vector3 |
	//+===+================================================
	struct Vector3
	{
		Vector3()
		{
			X = 0;
			Y = 0;
			Z = 0;
		}

		Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		Vector3(const Vector3& _ref)
		{
			X = _ref.X;
			Y = _ref.Y;
			Z = _ref.Z;
		}

		float	X,
				Y,
				Z;
	};

	//+===+================================================
	//----| Channel	|
	//+===+================================================
	class Channel
	{
		//Public functions
	public:
		//General
		AUDIO_API						Channel(void);
		AUDIO_API virtual				~Channel(void);
		
		AUDIO_API virtual const bool	Initialize(class AudioEngine* engine) = 0;
		AUDIO_API virtual void			Reset(void);
		
		
		AUDIO_API const bool			IsReleased(void) const;
		AUDIO_API void					Release(void);
		AUDIO_API void					Lock(void);
		
		AUDIO_API virtual void			Update(const float deltaTime) = 0;
		AUDIO_API virtual void			UpdateVolume(void) = 0;
		
		AUDIO_API virtual const bool	IsActive(void) const;
		
		AUDIO_API virtual const bool	IsPlaying(void) const = 0;
		
		AUDIO_API virtual const bool	IsPaused(void) const = 0;
		AUDIO_API virtual void			Pause(void) = 0;
		AUDIO_API virtual void			Unpause(void) = 0;
		
		AUDIO_API virtual const bool	IsMuted(void) const = 0;
		AUDIO_API virtual void			Mute(void) = 0;
		AUDIO_API virtual void			Unmute(void) = 0;
		
		
		AUDIO_API  virtual	void		Kill(void) = 0;
		
		
		AUDIO_API virtual void			SetMinMaxDistance(const float min, const float max) = 0;
		AUDIO_API virtual void			SetPosition(const Vector3& pos);
		AUDIO_API virtual void			SetVelocity(const Vector3& vel);
		AUDIO_API virtual void			SetVolume(const float vol) = 0;
		AUDIO_API virtual void			SetFrequency(const float freq) = 0;
		AUDIO_API virtual void			SetLooping(const bool loop) = 0;

		AUDIO_API virtual const float	GetVolume(void) const = 0;
		AUDIO_API virtual const float	GetFrequency(void) const = 0;
		AUDIO_API virtual const float	GetMinDistance(void) const = 0;
		AUDIO_API virtual const float	GetMaxDistance(void) const = 0;
		AUDIO_API virtual const Vector3	GetPosition(void) const = 0;
		AUDIO_API virtual const Vector3	GetVelocity(void) const = 0;
		AUDIO_API virtual const bool	GetLooping(void) const = 0;

		//Protected Functions
	protected:
								Channel(Channel &_ref);
		//Protected variables
	protected:
		Vector3				m_Position,
							m_Velocity;

		bool				m_Released,
							m_Active,
							m_Muted,
							m_Paused,
							m_Looping;

		float				m_Volume,
							m_Frequency,
							m_FrequencyFactor,
							m_DistanceMin,
							m_DistanceMax;
	};
}