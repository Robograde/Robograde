/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include <SDL2/SDL.h>
#include <memory/Alloc.h>
#include "audio/AudioDLLExport.h"
#include "../Sound.h"

namespace Audio
{
	//+===+================================================
	//----| AudioBuffer	|
	//----+-------------+---------------------------------
	//----|Contains all the data about the audio buffers the system uses
	//+===+================================================
	class AudioBuffer
	{
	public:
		AUDIO_API AudioBuffer()
			:	chunk(nullptr)
			,	chunkSize(0)
			,	readBytes(0)
			,	readBytesTotal(0)
		{}

		AUDIO_API ~AudioBuffer()
		{
			if(chunk)
				tDelete(chunk);
		}

		AUDIO_API void	Rewind()
		{
			readBytes = 0;
			readBytesTotal = 0;
			memset(chunk, 0, chunkSize);
		}

		AUDIO_API void	ResetChunkReading()
		{
			readBytes = 0;
		}

		Uint8*			chunk;			///Bufferdata
		Uint32			chunkSize;		///Size only of the smaller chunk. If SoundType == sample, bufferSize == chunkSize
		SDL_AudioSpec	spec;

		Uint32			readBytes;		///Used by streaming to keep track of how much is left to read while the chunk is loading
		Uint32			readBytesTotal;	///Used by streaming to keep track of how much is left to read
	};
}