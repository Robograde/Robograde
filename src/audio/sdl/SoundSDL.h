/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Sound.h"
#include <SDL2/SDL.h>
#include <memory/Alloc.h>

#include "AudioBuffer.h"
#include "AudioFileReader.h"
#include "audio/AudioDLLExport.h"
namespace Audio
{
	//+===+================================================
	//----| SoundSDL	|
	//+===+================================================
	class SoundSDL :
		public Sound
	{
	//Public functions
	public:
		//General
		AUDIO_API					SoundSDL(void);
		AUDIO_API					~SoundSDL(void);

		AUDIO_API bool				Initialize(class AudioEngine* engine, const rString &path, SoundType type) override;
		AUDIO_API void				Cleanup(void) override;

		AUDIO_API void				UpdateStream(void);

		AUDIO_API class Channel*	Play(class Channel* dedicatedChannel = 0, int flags = 0) override;
		AUDIO_API void				Stop(class Channel* channel) override;

		AUDIO_API AudioBuffer*		GetBufferData(void);
		AUDIO_API const bool		GetStreamingActive(void);
	//Private functions
	private:
									SoundSDL(SoundSDL &_ref);

		bool						CreateSample(const rString &path, AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec);
		bool						CreateStream(const rString &path, AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec);

		int							RebuildAudioCVT(AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec);
		bool						ConvertBufferData(AudioBuffer* &buffer, SDL_AudioCVT* &audioCVT);
	//Private variables
	private:
		AudioBuffer*				m_BufferData;
		SDL_AudioCVT*				m_AudioCVT;
		AudioFileReader*			m_AudioFile;
		bool						m_StreamingActive;
		Uint32						m_ChunkSize;
	};
}