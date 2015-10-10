/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "AudioFileReader.h"
#include <vorbis/vorbisfile.h>
#include <memory/Alloc.h>

#include "../Sound.h"
#include "AudioBuffer.h"
#include "audio/AudioDLLExport.h"
namespace Audio
{
	//+===+================================================
	//----| OggFileReader	|
	//+===+================================================
	class OggFileReader :
		public AudioFileReader
	{
	//Public functions
	public:
	//General
		AUDIO_API			OggFileReader(void);
		AUDIO_API			~OggFileReader(void);

		AUDIO_API bool		Initialize(void) override;
		AUDIO_API void		Cleanup(void) override;

		AUDIO_API bool		LoadSample(const rString &path, AudioBuffer* &buffer) override;

		AUDIO_API bool		OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer) override;
		AUDIO_API bool		ReadStream(AudioBuffer* &streamBuffer, const unsigned int &numBytes) override;
		AUDIO_API void		RestartStream(AudioBuffer* &streamBuffer) override;
		AUDIO_API bool		CloseStream(AudioBuffer* &streamBuffer) override;

	//Private functions
	private:
							OggFileReader(OggFileReader &_ref);
		bool				GetOggInfo(OggVorbis_File* &oggFile, SDL_AudioSpec* inOutspec);

	//Private variables
	private:
		OggVorbis_File*		m_OggFile;
		ogg_int64_t			m_DataOriginPos;
	};
}