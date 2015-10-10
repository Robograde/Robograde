/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "OggFileReader.h"

namespace Audio
{

//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
OggFileReader::OggFileReader(void)
	: m_OggFile(nullptr)
{
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
OggFileReader::~OggFileReader(void)
{
	Cleanup();
}
//+----------------------------------------------------------------------------+
//|bool Initialize(void)
//\----------------------------------------------------------------------------+
bool OggFileReader::Initialize(void)
{
	m_OggFile = pNew(OggVorbis_File);

	return true;
}
//+----------------------------------------------------------------------------+
//|void Cleanup(void)
//\----------------------------------------------------------------------------+
void OggFileReader::Cleanup(void)
{
	if(m_OggFile)
		pDelete(m_OggFile);
}
//+---------------------------------------------------+
//|bool LoadSample(const rString &path, AudioBuffer* &buffer)
//|Completely read the input audiofile into buffer memory
//\---------------------------------------------------+
bool OggFileReader::LoadSample(const rString &path, AudioBuffer* &buffer)
{
	//Try to open for reading
	int result = ov_fopen(path.c_str(), m_OggFile);

	if(result < 0)
  	{
		fprintf(stderr,"LoadOggSample, Failed trying to open: %s", path.c_str());
		ov_clear(m_OggFile);

		return false;
	}

	///Read the ogg informattion to get the specifications for the sound
	if(!GetOggInfo(m_OggFile, &buffer->spec))
	{
		fprintf(stderr,"LoadOggSample, Failed trying to read info from: %s", path.c_str());
		ov_clear(m_OggFile);

		return false;
	}

	//Allocate buffer memory for pcm-data
	buffer->chunk = tNewArray(Uint8, buffer->spec.size);
	memset(buffer->chunk, 0, buffer->spec.size); 

	int read = 0;
	Uint32 to_read = buffer->spec.size;
	int bitStream = -1;
	Uint8* tempBuffer = tNewArray(Uint8, to_read);

	//Read all the sound data into the buffer one step at the time
	do
	{
		read = ov_read(m_OggFile, (char*)tempBuffer, to_read, 0, 2, 1, &bitStream);

		///Append the read bytes to the chunk
		memcpy(buffer->chunk + buffer->readBytes, tempBuffer, read);
	
		buffer->readBytes += read;
		buffer->readBytesTotal += read;

	} while (read > 0);

	tDeleteArray(tempBuffer);

	//Completed reading file, sound buffer created
	ov_clear(m_OggFile);

    return true;
}
//+---------------------------------------------------+
//|bool OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer)
//|Open and prepare the audiofile for later streaming
//\---------------------------------------------------+
bool OggFileReader::OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer)
{
	//Try to open for reading
	int result = ov_fopen(path.c_str(), m_OggFile);

	if(result < 0)
  	{
		fprintf(stderr,"OpenStreamOgg, Failed trying to open: %s", path.c_str());
		ov_clear(m_OggFile);

		return false;
	}

	///Read the ogg informattion to get the specifications for the sound
	if(!GetOggInfo(m_OggFile, &streamBuffer->spec))
	{
		fprintf(stderr,"LoadOggSample, Failed trying to read info from: %s", path.c_str());
		ov_clear(m_OggFile);

		return false;
	}

	//Allocate buffer memory for PCM-data
	streamBuffer->chunk = tNewArray(Uint8, chunkSize);
	streamBuffer->chunkSize = chunkSize;
	streamBuffer->readBytes = 0;
	m_DataOriginPos = ov_pcm_tell(m_OggFile);
    return true;
}
//+---------------------------------------------------+
//|bool ReadStream(AudioBuffer* &streamBuffer, const Uint32 &numBytes)
//\---------------------------------------------------+
bool OggFileReader::ReadStream(AudioBuffer* &streamBuffer, const Uint32 &numBytes)
{
	Uint8* tempBuffer = tNewArray(Uint8, numBytes);

	int read = 0;
	int to_read = numBytes;
	int bitStream = -1;

	do
	{
		read = ov_read(m_OggFile, (char*)tempBuffer, to_read, 0, 2, 1, &bitStream);

		if(read < 1)
			break;

		///Append the read bytes to the chunk
		memcpy(streamBuffer->chunk + streamBuffer->readBytes, tempBuffer, read);
	
		streamBuffer->readBytes += read;
		streamBuffer->readBytesTotal += read;
		to_read -= read;

	} while(to_read >= 0);

	tDeleteArray(tempBuffer);
    return true;
}
//+----------------------------------------------------------------------------+
//|void RestartStream(AudioBuffer* &streamBuffer)
//\----------------------------------------------------------------------------+
void OggFileReader::RestartStream(AudioBuffer* &streamBuffer)
{
	ov_pcm_seek(m_OggFile, m_DataOriginPos);
	streamBuffer->Rewind();
};
//+---------------------------------------------------+
//|bool CloseStream(AudioBuffer* &streamBuffer)
//\---------------------------------------------------+
bool OggFileReader::CloseStream(AudioBuffer* &streamBuffer)
{
	ov_clear(m_OggFile);
	tDeleteArray(streamBuffer->chunk);
	streamBuffer->chunk = nullptr;
    return true;
}
//+---------------------------------------------------+
//|bool GetOggInfo(OggVorbis_File* &oggfile, SDL_AudioSpec* inOutspec)
//|Retreives the information obout the ogg file, like number of channels etc that is used
//\---------------------------------------------------+
bool OggFileReader::GetOggInfo(OggVorbis_File* &oggFile, SDL_AudioSpec* inOutspec)
{
	///Retreive the info about the ogg file
	vorbis_info* pInfo;
	pInfo = ov_info(oggFile, -1);

	///Set channel format
	if(pInfo->channels == 1)
		inOutspec->format = AUDIO_S16;
	else if(pInfo->channels == 2)
		inOutspec->format = AUDIO_S16LSB;

	inOutspec->channels = static_cast<Uint8>(pInfo->channels);
	inOutspec->freq = pInfo->rate;
	inOutspec->callback = nullptr;
	inOutspec->userdata = nullptr;
	inOutspec->padding = 0;
	inOutspec->samples = 4096;

    ogg_int64_t audio_len = ov_pcm_total(m_OggFile, -1) * inOutspec->channels * 2;
	inOutspec->size = static_cast<Uint32>(audio_len);

    return true;
}
}