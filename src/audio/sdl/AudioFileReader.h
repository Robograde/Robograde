/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include <memory/Alloc.h>

#include "AudioBuffer.h"
#include "audio/AudioDLLExport.h"
namespace Audio
{
	//+===+================================================
	//----| AudioFileReader	|
	//+===+================================================
	class AudioFileReader
	{
	//Public functions
	public:
	//General
		AUDIO_API 				AudioFileReader(void){}
		AUDIO_API virtual		~AudioFileReader(void){}

		AUDIO_API virtual bool	Initialize(void) = 0;
		AUDIO_API virtual void	Cleanup(void) = 0;
	 
		AUDIO_API virtual bool	LoadSample(const rString &path, AudioBuffer* &buffer) = 0;
 
		AUDIO_API virtual bool	OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer) = 0;
		AUDIO_API virtual bool	ReadStream(AudioBuffer* &streamBuffer, const unsigned int &numBytes) = 0;
		AUDIO_API virtual void	RestartStream(AudioBuffer* &streamBuffer) = 0;
		AUDIO_API virtual bool	CloseStream(AudioBuffer* &streamBuffer) = 0;

	//Private functions
	private:
								AudioFileReader(AudioFileReader &_ref);

	//Protected variables
	protected:
	};
}