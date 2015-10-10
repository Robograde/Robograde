/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../AudioEngine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <memory/Alloc.h>
#include "audio/AudioDLLExport.h"
namespace Audio
{

	//+===+================================================
	//----| AudioEngineSDL |
	//+===+================================================
	class AudioEngineSDL :
		public AudioEngine
	{
		//Public variables
	public:
		//General
		AUDIO_API								AudioEngineSDL(void);
		AUDIO_API								~AudioEngineSDL(void);

		AUDIO_API const bool					Initialize(const int maxChannels = 100, const int listeners = 1) override;
		AUDIO_API void							Cleanup() override;

		AUDIO_API const bool					Update(const float deltaTime) override;

		AUDIO_API class Sound*					CreateSound(const rString &path, const bool asStream = false) override;

		AUDIO_API virtual void					InitializeChannels(const int num) override;

		//Getters
		AUDIO_API Channel*						GetFreeChannel(void) override;
		AUDIO_API rVector<class Channel*>*		GetActiveChannels(void);
		AUDIO_API const SDL_AudioSpec*			GetSystemSpec(void) const;
		AUDIO_API Uint8*						GetBufferPlaybackLeft(void) { return m_BufferPlaybackLeft; };
		AUDIO_API Uint8*						GetBufferPlaybackRight(void) { return m_BufferPlaybackRight; };

		//Setters
		AUDIO_API const bool					SetListernerOrientation(ListenerOrientation data) override;
		//Private Functions
	private:
												AudioEngineSDL(AudioEngineSDL &_ref);
		void									UpdateChannel3d(ListenerOrientation input);
		static void								AudioCallback(void *userdata, Uint8 *stream, int len);
		const bool								MixStream(Uint8* stream, Uint8* sample, const int length, const float volume);
		const bool								MixStereoStream(Uint8* stream, Uint8* sample, const int length, const float volumeLeft, const float volumeRight);
		static void								RepitchSample(Uint8* &inOutSample, const SDL_AudioSpec* &sampleSpec, const SDL_AudioSpec* &sysSpec, const float factor, const int sampleSize);
		//Private variables
	private:
		SDL_AudioSpec							m_SystemSpec;
		Uint8*									m_BufferPlaybackLeft;
		Uint8*									m_BufferPlaybackRight;
	};
}