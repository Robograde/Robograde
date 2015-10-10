/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Channel.h"
#include <SDL2/SDL.h>
#include "audio/AudioDLLExport.h"
namespace Audio
{
	//+===+================================================
	//----| ChannelSDL	|
	//+===+================================================
	class ChannelSDL :
		public Channel
	{
		//Public functions
	public:
		//General
		AUDIO_API							ChannelSDL(void);
		AUDIO_API							~ChannelSDL(void);

		AUDIO_API const bool				Initialize(class AudioEngine* engine) override;
		AUDIO_API void						Update(const float deltaTime) override;
		AUDIO_API void						UpdateVolume(void) override;
		AUDIO_API void						Update3d(Audio::Vector3 listenerPos, Audio::Vector3 listenerForward, Audio::Vector3 listenerUp);

		AUDIO_API const bool				IsPlaying(void) const override;
		AUDIO_API void						Play(void);

		AUDIO_API const bool				IsPaused(void) const override;
		AUDIO_API void						Pause(void) override;
		AUDIO_API void						Unpause(void) override;

		AUDIO_API const bool				IsMuted(void) const override;
		AUDIO_API void						Mute(void) override;
		AUDIO_API void						Unmute(void) override;

		AUDIO_API void						Reset(void) override;
		AUDIO_API virtual void				Kill(void) override;

		//Setters
		AUDIO_API void						SetMinMaxDistance(const float min, const float max) override;
		AUDIO_API void						SetVolume(const float vol) override;
		AUDIO_API void						SetFrequency(const float freq) override;
		
		AUDIO_API void						SetIs3D(const bool is3D);
		AUDIO_API void						SetLooping(const bool loop) override;

		AUDIO_API void						SetSound(class SoundSDL* sound);
		AUDIO_API void						SetBufferPosition(const int pos);

		//Getters
		AUDIO_API const float				GetVolume(void) const override;
		AUDIO_API const float				GetFrequency(void) const override;
		AUDIO_API virtual const float		GetMinDistance(void) const override;
		AUDIO_API virtual const float		GetMaxDistance(void) const override;
		AUDIO_API virtual const Vector3		GetPosition(void) const override;
		AUDIO_API virtual const Vector3		GetVelocity(void) const override;

		AUDIO_API const bool				GetIs3d(void) const;
		AUDIO_API const bool				GetLooping(void) const override;

		AUDIO_API class SoundSDL*			GetSound(void) const;
		AUDIO_API const int					GetBufferPosition(void) const;
		AUDIO_API const float				GetVolumeLeft(void) const;
		AUDIO_API const float				GetVolumeRight(void) const;

		//Private Functions
	private:
											ChannelSDL(ChannelSDL &_ref);
		void								UpdateDistanceVolume(const Vector3 listenerPos);
		void								UpdateStereoPanning(const Vector3 listenerPos, const Vector3 listenerForward, const Vector3 listenerUp);
		//Private variables
	private:
		class SoundSDL*						m_Sound;
		int									m_BufferPosition;

		bool								m_Is3D;

		float								m_StereoPan,
											m_StereoLeft,
											m_StereoRight,
											m_DistanceFactor,
											m_VolumeLeft,
											m_VolumeRight;
	};
}