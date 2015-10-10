/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include "AudioEngineSDL.h"
#include "../ListenerOrientation.h"
#include "SoundSDL.h"
#include "ChannelSDL.h"

#define AUDIO_SAMPLE_RATE	44100
#define AUDIO_FORMAT		AUDIO_S16LSB
#define AUDIO_CHANNELS		2
#define AUDIO_BUFFER_SIZE	4096

namespace Audio
{
//+----------------------------------------------------------------------------+
//|Constructor(void)
//\----------------------------------------------------------------------------+
AudioEngineSDL::AudioEngineSDL(void)
	: m_BufferPlaybackLeft(nullptr)
	, m_BufferPlaybackRight(nullptr)
{

}
//+----------------------------------------------------------------------------+
//|Destructor(void)
//\----------------------------------------------------------------------------+
AudioEngineSDL::~AudioEngineSDL(void)
{
	Cleanup();
}
//+----------------------------------------------------------------------------+
//|const bool Initialize(const int maxChannels, const int listeners)
//\----------------------------------------------------------------------------+
const bool AudioEngineSDL::Initialize(const int maxChannels, const int listeners)
{
	///Initialize SDL audio
	if(SDL_Init(SDL_INIT_AUDIO) < 0)
		return false;

	///Create audio device
	SDL_AudioSpec want;
	want.channels = AUDIO_CHANNELS;
	want.freq = AUDIO_SAMPLE_RATE;
	want.format = AUDIO_FORMAT;
	want.samples = AUDIO_BUFFER_SIZE;
	want.padding = 0;
	want.silence = 0;
	want.size = 0;
	want.callback = AudioEngineSDL::AudioCallback;
	want.userdata = this;

	if(SDL_OpenAudio(&want, &m_SystemSpec) < 0)
	{
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}

	m_BufferPlaybackLeft = pNewArray(Uint8, m_SystemSpec.size);
	m_BufferPlaybackRight = pNewArray(Uint8, m_SystemSpec.size);

	///Initialize audioengine last
	AudioEngine::Initialize(maxChannels, listeners);

	/* Start playing */
	SDL_PauseAudio(0);
	return true;
}
//+----------------------------------------------------------------------------+
//|void Cleanup(void)
//\----------------------------------------------------------------------------+
void AudioEngineSDL::Cleanup(void)
{
	SDL_CloseAudio();

	AudioEngine::Cleanup();

	pDeleteArray(m_BufferPlaybackLeft);
	pDeleteArray(m_BufferPlaybackRight);
}
//+----------------------------------------------------------------------------+
//|const bool Update(const float deltaTime)
//\----------------------------------------------------------------------------+
const bool AudioEngineSDL::Update(const float deltaTime)
{
	AudioEngine::Update(deltaTime);

	for(std::vector<ListenerOrientation>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); it++)
	{
		UpdateChannel3d((*it));
	}

	return true;
}
//+---------------------------------------------------+
//|Sound* CreateSound(rString path, const bool asStream)
//|Create the sound using the input path.
//\---------------------------------------------------+
Sound* AudioEngineSDL::CreateSound(const rString &path, bool asStream)
{
	SDL_LockAudio();

	SoundSDL* sound = pNew(SoundSDL);

	if(asStream)
	{
		if(sound->Initialize(this, path, SoundType::STREAM))
			m_Sounds.push_back(sound);
		else
		{
			pDelete(sound);
			fprintf(stderr, "Failed to create sound stream: %s\n", path.c_str());
			SDL_UnlockAudio();
			return nullptr;
		}
	}
	else
	{
		if(sound->Initialize(this, path, SoundType::SAMPLE))
			m_Sounds.push_back(sound);
		else
		{
			pDelete(sound);
			fprintf(stderr, "Failed to create sound sample: %s\n", path.c_str());
			SDL_UnlockAudio();
			return nullptr;
		}
	}

	SDL_UnlockAudio();
	return sound;
}
//+---------------------------------------------------+
//|void InitializeChannels(const int num)
//\---------------------------------------------------+
void AudioEngineSDL::InitializeChannels(const int num)
{
	SDL_LockAudio();
	m_Channels.reserve(num);
	for(int i = 0; i < num; i++)
	{
		ChannelSDL* channel = pNew(ChannelSDL);
		channel->Initialize(this);
		m_Channels.push_back(channel);
	}
	SDL_UnlockAudio();
}
//+----------------------------------------------------------------------------+
//|Channel* GetFreeChannel(void)
//|Locate and return an unused channel
//\----------------------------------------------------------------------------+
Channel* AudioEngineSDL::GetFreeChannel(void)
{
	SDL_LockAudio();
	Channel* channel = 0;

	///Locate the first free(released) channel
	for(std::vector<Channel*>::iterator it = m_Channels.begin(); it != m_Channels.end(); it++)
	{
		if((*it)->IsReleased())
		{
			channel = (*it);
			break;
		}
	}

	///No free channel found, create new one instead
	if(!channel)
	{
		channel = pNew(ChannelSDL);
		m_Channels.push_back(channel);
	}

	SDL_UnlockAudio();
	return channel;
}
//+----------------------------------------------------------------------------+
//|rVector<Channel*>* AudioEngineSDL::GetActiveChannels(void)
//\----------------------------------------------------------------------------+
rVector<Channel*>* AudioEngineSDL::GetActiveChannels(void)
{
	return &m_Channels;
}
//+----------------------------------------------------------------------------+
//|const SDL_AudioSpec* AudioEngineSDL::GetSystemSpec(void) const
//\----------------------------------------------------------------------------+
const SDL_AudioSpec* AudioEngineSDL::GetSystemSpec(void) const
{
	return &m_SystemSpec;
};
//+---------------------------------------------------+
//|const bool SetListernerOrientation(ListenerOrientation data)
//|Set the position of the listener(e.g. the camera)
//\---------------------------------------------------+
const bool AudioEngineSDL::SetListernerOrientation(ListenerOrientation data)
{
	m_Listeners.at(data.Id) = data;

	return true;
}
//+----------------------------------------------------------------------------+
//|void UpdateChannel3d(ListenerOrientation input)
//|Update all the channels volumes regarding the listeners position
//\----------------------------------------------------------------------------+
void AudioEngineSDL::UpdateChannel3d(ListenerOrientation input)
{
	SDL_LockAudio();
	for(std::vector<Channel*>::iterator it = m_Channels.begin(); it != m_Channels.end(); it++)
	{
		ChannelSDL* channel = reinterpret_cast<ChannelSDL*>((*it));
		channel->Update3d(Vector3(input.Px, input.Py, input.Pz), Vector3(input.Fx, input.Fy, input.Fz), Vector3(input.Ux, input.Uy, input.Uz));
	}
	SDL_UnlockAudio();
}
//+----------------------------------------------------------------------------+
//|void AudioCallback(void* userdata, Uint8* stream, int len)
//|Handle mixing of all the active channels
//\----------------------------------------------------------------------------+
void AudioEngineSDL::AudioCallback(void* userdata, Uint8* stream, int len)
{
	auto audioEngine = (AudioEngineSDL*)userdata;
	rVector<Channel*>* channels = audioEngine->GetActiveChannels();
	Uint8* bufferLeft = audioEngine->GetBufferPlaybackLeft();
	Uint8* bufferRight = audioEngine->GetBufferPlaybackRight();
	const SDL_AudioSpec* spec = audioEngine->GetSystemSpec();

	///Whipe buffers clean since last use
	SDL_memset(stream, spec->silence, len);
	SDL_memset(bufferLeft, spec->silence, len);
	SDL_memset(bufferRight, spec->silence, len);

	for(std::vector<Channel*>::iterator it = channels->begin(); it != channels->end(); it++)
	{
		ChannelSDL* channel = reinterpret_cast<ChannelSDL*>((*it));

		///Dont update relased or paused channels
		if(channel->IsReleased() || channel->IsPaused())
			continue;

		SoundSDL* sound = reinterpret_cast<SoundSDL*>(channel->GetSound());

		if(sound == nullptr || sound->GetBufferData() == nullptr)
			continue;
		
		///Don't use streaming audio thats not active
		if(sound->GetType() == SoundType::STREAM && !sound->GetStreamingActive())
			continue;

		int channelPos = channel->GetBufferPosition();

		///Dont mix muted channels, just skip further
		if(channel->IsMuted())
		{
			channelPos += len;
			channel->SetBufferPosition(channelPos);
			continue;
		}

		AudioBuffer* bufferdata = sound->GetBufferData();
		const int bufferLength = bufferdata->chunkSize;
		int sampleLength = len;

		///End of chunk
		if(channelPos + len > bufferLength)
		{
			///If its a stream, load new data into the buffer if needed
			if(sound->GetType() == SoundType::STREAM)
			{
				if(bufferdata->readBytesTotal + len < bufferdata->spec.size) // bufferdata->spec.size == total size of audio data
					bufferdata->ResetChunkReading();
	
				sound->UpdateStream();

				if(sound->GetBufferData()->readBytes != len)
					continue;

				channelPos = 0;

				///TODOJS:Fix so even streams wont have to loop
			}
			///Samples may be looped or released
			else
			{
				if(channel->GetLooping())
				{
					channelPos = 0;
				}
				else
				{
					sound->Stop(channel);
					continue;
				}
			}
		}

		///Check how much that is left to play, play the remaining samples if its finished this frame
		if(sound->GetType() == SoundType::SAMPLE)
		{
			if(channelPos + len > bufferLength)
			{
				///Get the remaining length 
				int diff = channelPos + len - bufferLength;
				sampleLength = sampleLength - diff;
			}
		}

		///Prepare for sampling to streambuffer
		Uint8* sample = bufferdata->chunk + channelPos;

		if(channel->GetIs3d())
			audioEngine->MixStereoStream(stream, sample, sampleLength, channel->GetVolumeLeft(), channel->GetVolumeRight());
		else
			audioEngine->MixStream(stream, sample, sampleLength, channel->GetVolume());

		///Update position in the buffer that the channel is at(oldPos + streambufferLength)
		channelPos += sampleLength;

		channel->SetBufferPosition(channelPos);
	}
}
//+----------------------------------------------------------------------------+
//|const bool MixStream(Uint8* stream, Uint8* sample, const int length, const float volume)
//\----------------------------------------------------------------------------+
const bool AudioEngineSDL::MixStream(Uint8* stream, Uint8* sample, const int length, const float volume)
{
	//Mix the whole sample into the stream
	///Assuming sample is using the same format as the engine
	const SDL_AudioSpec* spec = GetSystemSpec();
	int volumeI = static_cast<int>(volume * SDL_MIX_MAXVOLUME);
	SDL_MixAudioFormat(stream, sample, spec->format, length, volumeI);
	return true;
}
//+----------------------------------------------------------------------------+
//|const bool MixStereoStream(Uint8* stream, Uint8* sample, const int length, const float volumeLeft, const float volumeRight)
//\----------------------------------------------------------------------------+
const bool AudioEngineSDL::MixStereoStream(Uint8* stream, Uint8* sample, const int length, const float volumeLeft, const float volumeRight)
{
	///Sample data is separated into to temporary left/right buffers and later combined into final stream
	int volumeLeftI = static_cast<int>(volumeLeft * SDL_MIX_MAXVOLUME);
	int volumeRightI = static_cast<int>(volumeRight * SDL_MIX_MAXVOLUME);
	Uint8* bufferLeft = GetBufferPlaybackLeft();
	Uint8* bufferRight = GetBufferPlaybackRight();
	const SDL_AudioSpec* spec = GetSystemSpec();

	///Mix audio according how the format specifies audio should be arranged in stream
	switch(spec->format)
	{
		///Stereo 8-bit (LR-byteorder)
		case AUDIO_U8:
		{
			for(int index = 0; index < length; index += 2)
			{
				bufferLeft[index] = sample[index];

				if(index + 1 < length)
					bufferRight[index + 1] = sample[index + 1];
			}

			break;
		}

		///Stereo 16-bit (LLRR-byteorder)
		case AUDIO_S16LSB:
		{
			for(int index = 0; index < length; index += 4)
			{
				bufferLeft[index] = sample[index];

				if(index + 1 < length)
					bufferLeft[index + 1] = sample[index + 1];

				if(index + 2 < length)
					bufferRight[index + 2] = sample[index + 2];

				if(index + 3 < length)
					bufferRight[index + 3] = sample[index + 3];
			}
			break;
		}

		default:
		{
			fprintf(stderr, "Failed audio mixing, unsupported format: %d\n", spec->format);
			return false;
		}

	};

	///Mix temporary left/right buffers into the final stream
	SDL_MixAudioFormat(stream, bufferLeft, spec->format, length, volumeLeftI);
	SDL_MixAudioFormat(stream, bufferRight, spec->format, length, volumeRightI);
	return true;
}
//+----------------------------------------------------------------------------+
//|void RepitchSample(Uint8* &inOutSample, const SDL_AudioSpec* &sampleSpec, const SDL_AudioSpec* &sysSpec, const float factor, const int sampleSize)
//\----------------------------------------------------------------------------+
void AudioEngineSDL::RepitchSample(Uint8* &inOutSample, const SDL_AudioSpec* &sampleSpec, const SDL_AudioSpec* &sysSpec, const float factor, const int sampleSize)
{
	SDL_AudioCVT cvt;

	const int convertedFreq = static_cast<int>(sampleSpec->freq * factor);

	if(SDL_BuildAudioCVT(&cvt, sampleSpec->format, sampleSpec->channels, convertedFreq, sysSpec->format, sysSpec->channels, sysSpec->freq) < 0)
	{
		return;
	}

	///Start conversion and delete the loaded buffer
	const int convertedBufferSize = sampleSize * cvt.len_mult;
	cvt.buf = fNewArray(Uint8, convertedBufferSize);
	cvt.len = sampleSize;
	SDL_memcpy(cvt.buf, inOutSample, sampleSize);

	///Delete old buffer data since its not useful anymore
	tDeleteArray(inOutSample);

	///Convert the audio stored in the CVT into engine format
	if(SDL_ConvertAudio(&cvt) < 0)
	{
		return;
	}

	///Copy the converted data back into the original, resized buffer
	inOutSample = tNewArray(Uint8, convertedBufferSize);
	SDL_memcpy(inOutSample, cvt.buf, convertedBufferSize);

	///Delete converted data since its stored in the original buffer again
	fDeleteArray(cvt.buf);
}
}