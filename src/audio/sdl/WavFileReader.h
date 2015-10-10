/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "AudioFileReader.h"

#include <stdio.h>
#include <memory/Alloc.h>

#include "../Sound.h"
#include "AudioBuffer.h"
#include "audio/AudioDLLExport.h"

namespace Audio
{
	//+===+================================================
	//----| WavFileReader	|
	//+===+================================================
	class WavFileReader :
		public AudioFileReader
	{
	//Public functions
	public:
	//General
		AUDIO_API		WavFileReader(void);
		AUDIO_API		~WavFileReader(void);

		AUDIO_API bool	Initialize(void) override;
		AUDIO_API void	Cleanup(void) override; 
		AUDIO_API bool	LoadSample(const rString &path, AudioBuffer* &buffer) override;
		AUDIO_API bool	OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer) override;
		AUDIO_API bool	ReadStream(AudioBuffer* &streamBuffer, const unsigned int &numBytes) override;
		AUDIO_API void	RestartStream(AudioBuffer* &streamBuffer) override;
		AUDIO_API bool	CloseStream(AudioBuffer* &streamBuffer) override;

	//Private functions
	private:
						WavFileReader(WavFileReader &_ref);
		bool			ParseWavHeader(FILE* &audioFile, SDL_AudioSpec* inOutspec);

	//Private variables
	private:
	FILE*				m_AudioFile;
	Uint32				m_DataOriginPos;	///Used by streaming to know where the audio data is located in the file
	};
}