/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "WavFileReader.h"
#include "../Wav_data.h"

namespace Audio
{

//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
WavFileReader::WavFileReader(void)
	:	m_AudioFile(nullptr)
	,	m_DataOriginPos(0)
{
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
WavFileReader::~WavFileReader(void)
{
	Cleanup();
}
//+----------------------------------------------------------------------------+
//|bool Initialize(void)
//\----------------------------------------------------------------------------+
bool WavFileReader::Initialize(void)
{
	return true;
}
//+----------------------------------------------------------------------------+
//|void Cleanup(void)
//\----------------------------------------------------------------------------+
void WavFileReader::Cleanup(void)
{

}
//+---------------------------------------------------+
//|bool LoadSample(const rString &path, AudioBuffer* &buffer)
//|Completely read the input audiofile into buffer memory
//\---------------------------------------------------+
bool WavFileReader::LoadSample(const rString &path, AudioBuffer* &buffer)
{
	///Try to open the file
    m_AudioFile = fopen(path.c_str(), "rb");

    if(!m_AudioFile)
  	{
		fprintf(stderr,"LoadWavSample, Failed trying to open: %s", path.c_str());

		if(m_AudioFile != NULL)
			fclose(m_AudioFile);

		return false;
	}

	///Read the wav-header and get the specifications for the sound
	if(!ParseWavHeader(m_AudioFile, &buffer->spec))
	{
		fprintf(stderr,"LoadWavSample, Failed trying to read header of: %s", path.c_str());

		if(m_AudioFile != NULL)
			fclose(m_AudioFile);

		return false;
	}
	
    //Allocate buffer memory for pcm-data
    buffer->chunk = tNewArray(Uint8, buffer->spec.size);
 
    //Read all the sound data into the buffer
    if(!fread(buffer->chunk, buffer->spec.size, 1, m_AudioFile))
	{
		fprintf(stderr, "LoadWav, error loading WAVE data into buffer from: %s", path.c_str());
		tDeleteArray(buffer->chunk);
		buffer->chunk = nullptr;
		return false;
	}

	buffer->readBytes = buffer->spec.size;
	buffer->readBytesTotal = buffer->spec.size;

    //Completed reading file, sound buffer created
    fclose(m_AudioFile);

    return true;
}
//+---------------------------------------------------+
//|bool OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer)
//|Open and prepare the audiofile for later streaming, parsing the wav-header
//\---------------------------------------------------+
bool WavFileReader::OpenStream(const rString &path, const unsigned int &chunkSize, AudioBuffer* &streamBuffer)
{
	///Try to open the file
    m_AudioFile = fopen(path.c_str(), "rb");

    if(!m_AudioFile)
  	{
		fprintf(stderr,"OpenWavStream, Failed trying to open: %s", path.c_str());

		//clean up memory if wave loading fails
		if(m_AudioFile != NULL)
			fclose(m_AudioFile);

		//return false to indicate the failure to load wave
		return false;
	}

	///Read the wav-header and get the specifications for the sound
	if(!ParseWavHeader(m_AudioFile, &streamBuffer->spec))
	{
		fprintf(stderr,"LoadWavSample, Failed trying to read header of: %s", path.c_str());

		if(m_AudioFile != NULL)
			fclose(m_AudioFile);

		return false;
	}

	//Allocate buffer memory for wav-data
	streamBuffer->chunk = tNewArray(Uint8, chunkSize);
	streamBuffer->chunkSize = chunkSize;
	streamBuffer->readBytes = 0;
	m_DataOriginPos = ftell(m_AudioFile);

    return true;
}
//+---------------------------------------------------+
//|bool ReadStream(AudioBuffer* &streamBuffer, const Uint32 &numBytes)
//\---------------------------------------------------+
bool WavFileReader::ReadStream(AudioBuffer* &streamBuffer, const Uint32 &numBytes)
{
	Uint8* tempBuffer = tNewArray(Uint8, numBytes);

	if(fread(tempBuffer, numBytes, 1, m_AudioFile) == 0)
	{
		fprintf(stderr, "ReadWavStream, error streaming WAVE data into buffer\n");
		return false;
	}

	///Append the read bytes to the chunk
	memcpy(streamBuffer->chunk + streamBuffer->readBytes, tempBuffer, numBytes);
	
	streamBuffer->readBytes += numBytes;
	streamBuffer->readBytesTotal += numBytes;

	tDeleteArray(tempBuffer);

    return true;
}
//+----------------------------------------------------------------------------+
//|void RestartStream(AudioBuffer* &streamBuffer)
//\----------------------------------------------------------------------------+
void WavFileReader::RestartStream(AudioBuffer* &streamBuffer)
{
	fseek(m_AudioFile, m_DataOriginPos, SEEK_SET);
	streamBuffer->Rewind();
};
//+---------------------------------------------------+
//|bool CloseStream(AudioBuffer* &streamBuffer)
//\---------------------------------------------------+
bool WavFileReader::CloseStream(AudioBuffer* &streamBuffer)
{
	fclose(m_AudioFile);
	tDeleteArray(streamBuffer->chunk);
	streamBuffer->chunk = nullptr;
    return true;
}	
//+---------------------------------------------------+
//|bool ParseWavHeader(FILE* &audioFile, SDL_AudioSpec* inOutspec)
//|Tries to parse the input file. Reading the header and stop at the data segment for future data reading
//\---------------------------------------------------+
bool WavFileReader::ParseWavHeader(FILE* &audioFile, SDL_AudioSpec* inOutspec)
{
	WAVE_Format wave_format;
	RIFF_Header riff_header;
	WAVE_Data wave_data;
 
    // Read in the first chunk into the struct
    fread(&riff_header, sizeof(RIFF_Header), 1, audioFile);
 
    //check for RIFF and WAVE tag in memeory
    if ((riff_header.chunkID[0] != 'R' ||
         riff_header.chunkID[1] != 'I' ||
         riff_header.chunkID[2] != 'F' ||
         riff_header.chunkID[3] != 'F') ||
        (riff_header.format[0] != 'W' ||
         riff_header.format[1] != 'A' ||
         riff_header.format[2] != 'V' ||
         riff_header.format[3] != 'E'))
	{
		fprintf(stderr, "ParseWavHeader, Invalid RIFF or WAVE Header\n");

		if(audioFile != NULL)
			fclose(audioFile);

		return false;
	}
 
    //Read in the 2nd chunk for the wave info
    fread(&wave_format, sizeof(WAVE_Format), 1, audioFile);

    //check for fmt tag in memory
    if (wave_format.subChunkID[0] != 'f' ||
        wave_format.subChunkID[1] != 'm' ||
        wave_format.subChunkID[2] != 't' ||
        wave_format.subChunkID[3] != ' ')
	{
		fprintf(stderr, "ParseWavHeader, Invalid Wave Format\n");

		if(audioFile != NULL)
			fclose(audioFile);

		return false;
	}
 
    //check for extra parameters;
    if(wave_format.subChunkSize > 16)
        fseek(audioFile, sizeof(Uint16), SEEK_CUR);
 
    //Read in the the last byte of data before the sound file
    fread(&wave_data, sizeof(WAVE_Data), 1, audioFile);
    //check for data tag in memory
    if (wave_data.subChunkID[0] != 'd' ||
        wave_data.subChunkID[1] != 'a' ||
        wave_data.subChunkID[2] != 't' ||
        wave_data.subChunkID[3] != 'a')
	{
		fprintf(stderr, "ParseWavHeader, Invalid data header\n");

		if(audioFile != NULL)
			fclose(audioFile);

		return false;
	}

    //The format is worked out by looking at the number of
    //channels and the bits per sample.
    if(wave_format.numChannels == 1)
	{
        if(wave_format.bitsPerSample == 8 )
			inOutspec->format = AUDIO_S8;
       else if(wave_format.bitsPerSample == 16)
			inOutspec->format = AUDIO_S16;
    }
	else if(wave_format.numChannels == 2)
	{
        if(wave_format.bitsPerSample == 8)
			inOutspec->format = AUDIO_S16LSB;
       else if(wave_format.bitsPerSample == 16)
           inOutspec->format = AUDIO_S16LSB;
    }

	inOutspec->channels = static_cast<Uint8>(wave_format.numChannels);
	inOutspec->freq = wave_format.sampleRate;
	inOutspec->size = wave_data.subChunk2Size;
	inOutspec->callback = nullptr;
	inOutspec->userdata = nullptr;
	inOutspec->padding = 0;
	inOutspec->samples = 4096; ///Default buffer size
    return true;
}

}