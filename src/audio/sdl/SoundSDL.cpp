/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "SoundSDL.h"
#include "ChannelSDL.h"
#include "AudioEngineSDL.h"
#include "WavFileReader.h"
#include "OggFileReader.h"

namespace Audio
{

//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SoundSDL::SoundSDL(void)
	:	m_AudioCVT(nullptr)
	,	m_BufferData(nullptr)
	,	m_AudioFile(nullptr)
	,	m_StreamingActive(false)
	,	m_ChunkSize(0)
{
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SoundSDL::~SoundSDL(void)
{
	Cleanup();
}
//+----------------------------------------------------------------------------+
//|bool Initialize(AudioEngine* engine, const rString &path, SoundType type)
//\----------------------------------------------------------------------------+
bool SoundSDL::Initialize(AudioEngine* engine, const rString &path, SoundType type)
{
	AudioEngineSDL* audioEngine = reinterpret_cast<AudioEngineSDL*>(engine);
	const SDL_AudioSpec* sysSpec = audioEngine->GetSystemSpec();
	m_AudioEngine = audioEngine;
	m_Type = type;
	m_Name = path;
	m_ChunkSize = sysSpec->size;

	m_AudioCVT = pNew(SDL_AudioCVT);
	m_BufferData = tNew(AudioBuffer);
	m_AudioFile = nullptr;

	///Check which file format the requested audiofile are
	///Only wav and ogg are supported

	///Is wave file
	if(path.find(".wav", 0) != std::string::npos)
	{ 
		m_AudioFile = pNew(WavFileReader);
	}
	///Is ogg file
	else if(path.find(".ogg", 0) != std::string::npos)
	{ 
		m_AudioFile = pNew(OggFileReader);
	}
	///Not supported format
	else
	{
		fprintf(stderr, "SoundSDL. Trying to load unsupported file format %s \n", path.c_str());
		return false;
	}

	m_AudioFile->Initialize();

	///Read as sample, read everything into buffer
	if(type == SoundType::SAMPLE)
	{
		CreateSample(path, m_BufferData, sysSpec);
	}
	///Read as stream, prepare for buffer streaming
	else if(type == SoundType::STREAM)
	{
		CreateStream(path, m_BufferData, sysSpec);
	}
	else
	{
		///This should not happend..
		fprintf(stderr, "SoundSDL. Unexpected error loading %s while setting sound type \n", path.c_str());
		return false;
	}

	return true;
}
//+----------------------------------------------------------------------------+
//|void Cleanup(void)
//\----------------------------------------------------------------------------+
void SoundSDL::Cleanup(void)
{
	if(m_AudioCVT)
	{
		pDelete(m_AudioCVT);
		m_AudioCVT = nullptr;
	}

	if(m_BufferData)
	{
		tDelete(m_BufferData);
		m_BufferData = nullptr;
	}

	if(m_AudioFile)
	{
		pDelete(m_AudioFile);
		m_AudioFile = nullptr;
	}
	
	m_AudioEngine = nullptr;
	m_Name = "";
};
//+----------------------------------------------------------------------------+
//|void UpdateStream(void)
//\----------------------------------------------------------------------------+
void SoundSDL::UpdateStream(void)
{
	///Dont update inactive streams
	if(!m_StreamingActive)
	{
		return;
	}

	//Check if stream should reset
	if(m_BufferData->readBytesTotal + m_ChunkSize > m_BufferData->spec.size)
	{
		m_AudioFile->RestartStream(m_BufferData);
	}

	if(!m_AudioFile->ReadStream(m_BufferData, m_ChunkSize))
	{
		fprintf(stderr, "Failed to read stream data %s\n", m_Name.c_str());
		return;
	}

	if(!ConvertBufferData(m_BufferData, m_AudioCVT))
	{
		fprintf(stderr, "Failed to convert audio: %s\n", GetName().c_str());
		return;
	}
};
//+----------------------------------------------------------------------------+
//|Channel* Play(Channel* dedicatedChannel, int flags)
//|Used to create a channel, an "instance of the sound". This is returned for detailed control while sound is playing.
//|The sound may be played using a specific dedicated channel
//\----------------------------------------------------------------------------+
Channel* SoundSDL::Play(Channel* dedicatedChannel, int flags)
{
	ChannelSDL* channel = nullptr;

	///Streaming audio may only have one channel dedicated
	if(m_Type == Audio::SoundType::STREAM)
	{
		if(m_NumInstaces > 0)
			return nullptr;
		
		UpdateStream();

	}

	///Dedicated
	if(dedicatedChannel)
	{
		channel = reinterpret_cast<ChannelSDL*>(dedicatedChannel);
	}
	///Get new free channel
	else
	{
		channel = reinterpret_cast<ChannelSDL*>(m_AudioEngine->GetFreeChannel());
	}

	channel->Reset();

	if((flags & IS_3D) == IS_3D)
		channel->SetIs3D(true);

	if((flags & LOOPING) == LOOPING)
		channel->SetLooping(true);

	channel->Lock();
	channel->SetSound(this);
	channel->Play();

	if((flags & PAUSED) == PAUSED)
		channel->Pause();

	m_NumInstaces++;

	///Streaming audio needs to be activated to update
	if(m_Type == Audio::SoundType::STREAM)
		m_StreamingActive = true;

	return channel;
};
//+----------------------------------------------------------------------------+
//|void Stop(Channel* channel)
//\----------------------------------------------------------------------------+
void SoundSDL::Stop(Channel* channel)
{
	SDL_LockAudio();
	if(channel)
	{
		if(m_Type == Audio::SoundType::STREAM)
		{
			channel->Kill();
			m_NumInstaces--;

			if(m_StreamingActive)
			{
				m_AudioFile->RestartStream(m_BufferData);
				m_StreamingActive = false;
			}

		}
		else
		{
			channel->Kill();
			m_NumInstaces--;
		}
	}
	SDL_UnlockAudio();
};
//+----------------------------------------------------------------------------+
//|AudioBuffer* GetBufferData(void)
//\----------------------------------------------------------------------------+
AudioBuffer* SoundSDL::GetBufferData(void)
{
	return m_BufferData;
};
//+----------------------------------------------------------------------------+
//|const bool GetBufferData(void)
//\----------------------------------------------------------------------------+
const bool SoundSDL::GetStreamingActive(void)
{
	return m_StreamingActive;
};
//+---------------------------------------------------+
//|bool CreateSample(const rString &path, AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec)
//\---------------------------------------------------+
bool SoundSDL::CreateSample(const rString &path, AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec)
{
	///Load the wav and recieve the buffer containing the audio data
	if(!m_AudioFile->LoadSample(path.c_str(), m_BufferData))
	{
		fprintf(stderr, "Failed to load sample %s\n", path.c_str());
		return false;
	}

	buffer->chunkSize = buffer->spec.size;

	///Create audio converter using the data from the wav header
	int ret = RebuildAudioCVT(m_BufferData, sysSpec);
	if(ret < 0)
	{
		fprintf(stderr, "Failed to build converter: %s\n", path.c_str());
		return false;
	}

	//Convert the loaded data into matching engine format
	//If ret == 1 the loaded audio needs not to be converted
	if(ret == 1)
	{
		if(!ConvertBufferData(buffer, m_AudioCVT))
		{
			fprintf(stderr, "Failed to convert audio: %s\n", path.c_str());
			return false;
		}
	}

	return true;
}
//+---------------------------------------------------+
//|bool CreateStream(const rString &path, AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec)
//\---------------------------------------------------+
bool SoundSDL::CreateStream(const rString &path, AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec)
{
	if(!m_AudioFile->OpenStream(path.c_str(), m_ChunkSize, buffer))
	{
		fprintf(stderr, "Failed to open wav stream %s\n", path.c_str());
		return false;
	}

	///Load the first chunk into memory
	if(!m_AudioFile->ReadStream(m_BufferData, m_ChunkSize))
	{
		fprintf(stderr, "Failed to stream data %s\n", m_Name.c_str());
		return false;
	}

	///Create audio converter using the data from the wav header
	int ret = RebuildAudioCVT(m_BufferData, sysSpec);
	if(ret < 0)
	{
		fprintf(stderr, "Failed to build converter: %s\n", path.c_str());
		return false;
	}

	//Convert the loaded data into matching engine format
	//If ret == 0 the loaded audio needs not to be converted
	if(ret == 1)
	{
		if(!ConvertBufferData(buffer, m_AudioCVT))
		{
			fprintf(stderr, "Failed to convert audio: %s\n", path.c_str());
			return false;
		}
	}

	return true;
}
//+----------------------------------------------------------------------------+
//|int RebuildAudioCVT(AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec)
//\----------------------------------------------------------------------------+
int SoundSDL::RebuildAudioCVT(AudioBuffer* &buffer, const SDL_AudioSpec* sysSpec)
{
	return SDL_BuildAudioCVT(m_AudioCVT, buffer->spec.format, buffer->spec.channels, buffer->spec.freq, sysSpec->format, sysSpec->channels, sysSpec->freq);
}
//+----------------------------------------------------------------------------+
//|void ConvertBufferData(AudioBuffer* &buffer, SDL_AudioCVT* &audioCVT)
//|Converts the input buffer into the audio format used by the engine
//\----------------------------------------------------------------------------+
bool SoundSDL::ConvertBufferData(AudioBuffer* &buffer, SDL_AudioCVT* &audioCVT)
{
	///Start conversion and delete the loaded buffer
	const Uint32 convertedBufferSize = buffer->chunkSize * audioCVT->len_mult;
	audioCVT->buf = tNewArray(Uint8, convertedBufferSize);
	audioCVT->len = buffer->chunkSize;
	SDL_memcpy(audioCVT->buf, buffer->chunk, buffer->chunkSize);

	///Delete old buffer data since its not useful anymore
	tDeleteArray(buffer->chunk);

	///Convert the audio stored in the CVT into engine format
	if(SDL_ConvertAudio(audioCVT) < 0)
	{
		return false;
	}

	///Copy the converted data back into the original, resized buffer
	buffer->chunk = tNewArray(Uint8, convertedBufferSize);
	SDL_memcpy(buffer->chunk, audioCVT->buf, convertedBufferSize);

	///Delete converted data since its stored in the original buffer again
	tDeleteArray(audioCVT->buf);

	return true;
}

}
