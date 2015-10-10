/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include <SDL2/SDL.h>
namespace Audio
{
	//+===+================================================
	//----| RIFF_Header |
	//+===+================================================
	struct RIFF_Header
	{
	  char	chunkID[4];
	  Uint32 chunkSize;//size not including chunkSize or chunkID
	  char	format[4];
	};

	//+===+================================================
	//----| WAVE_Format |
	//----| Struct to hold fmt subchunk data for WAVE files.
	//+===+================================================
	struct WAVE_Format
	{
		char	subChunkID[4];
		Uint32	subChunkSize;
		Uint16	audioFormat;
		Uint16	numChannels;
		Uint32	sampleRate;
		Uint32	byteRate;
		Uint16	blockAlign;
		Uint16	bitsPerSample;
	};

	//+===+================================================
	//----| WAVE_Data
	//----| Struct to hold the data of the wave file|
	//+===+================================================
	struct WAVE_Data
	{
	  char		subChunkID[4]; //should contain the word data
	  Uint32	subChunk2Size; //Stores the size of the data block
	};
}
