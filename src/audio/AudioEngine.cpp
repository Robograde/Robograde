/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#include <math.h>
#include "AudioEngine.h"
#include "Sound.h"
#include "Channel.h"
namespace Audio
{
//+----------------------------------------------------------------------------+
//|Constructor(void)
//\----------------------------------------------------------------------------+
AudioEngine::AudioEngine(void)
{

}
//+----------------------------------------------------------------------------+
//|Destructor(void)
//\----------------------------------------------------------------------------+
AudioEngine::~AudioEngine(void)
{
	Cleanup();
}
//+----------------------------------------------------------------------------+
//|const bool Initialize(const int maxChannels, const int listeners)
//\----------------------------------------------------------------------------+
const bool AudioEngine::Initialize(const int maxChannels, const int listeners)
{
	InitializeListerners(listeners);
	InitializeChannels(maxChannels);

	return true;
}
//+----------------------------------------------------------------------------+
//|void Cleanup(void)
//\----------------------------------------------------------------------------+
void AudioEngine::Cleanup(void)
{
	///Remove all channels
	for(rVector<Channel*>::iterator it = m_Channels.begin(); it != m_Channels.end(); it++)
	{
		if((*it))
			pDelete( *it );
	}

	m_Channels.clear();

	///Remove all sounds
	for(rVector<Sound*>::iterator it = m_Sounds.begin(); it != m_Sounds.end(); it++)
	{
		if((*it))
			pDelete( *it );
	}

	m_Sounds.clear();

	m_Listeners.clear();
}
//+----------------------------------------------------------------------------+
//|const bool Update(const float deltaTime)
//\----------------------------------------------------------------------------+
const bool AudioEngine::Update(const float deltaTime)
{
	UpdateChannels(deltaTime);
	return true;
}
//+----------------------------------------------------------------------------+
//|const bool DeleteSound(const rString path)
//\----------------------------------------------------------------------------+
const bool AudioEngine::DeleteSound(const rString &path)
{
	for(rVector<Sound*>::iterator it = m_Sounds.begin(); it != m_Sounds.end(); ++it)
	{
		if(strcmp((*it)->GetName().c_str(), path.c_str()) == 0)
		{
			pDelete( *it );
			m_Sounds.erase(it);
			return true;
		}
	}
	return false;
}
//+----------------------------------------------------------------------------+
//|const int GetNumlisterners(void)
//\----------------------------------------------------------------------------+
const int AudioEngine::GetNumlisterners(void)
{
	return static_cast<int>(m_Listeners.size());
}
//+---------------------------------------------------+
//|ListenerOrientation* GetListernerorientation(const int id)
//\---------------------------------------------------+
ListenerOrientation* AudioEngine::GetListernerorientation(const int id)
{
	if(id < 0 || id > static_cast<int>(m_Listeners.size()) - 1)
		return nullptr;

	return &m_Listeners.at(id);
}
//+----------------------------------------------------------------------------+
//|Channel* GetFreeChannel(void)
//\----------------------------------------------------------------------------+
Channel* AudioEngine::GetFreeChannel(void)
{
	Channel* channel = 0;

	///Locate the first free(released) channel
	for(rVector<Channel*>::iterator it = m_Channels.begin(); it != m_Channels.end(); it++)
	{
		if((*it)->IsReleased())
		{
			channel = (*it);
			break;
		}
	}

	return channel;
}
//+---------------------------------------------------+
//|const bool SetListernerorientation(ListenerOrientation data)
//\---------------------------------------------------+
const bool AudioEngine::SetListernerOrientation(ListenerOrientation data)
{
	if(data.Id > m_Listeners.size() - 1)
		return false;

	m_Listeners.at(data.Id) = data;

	return true;
}
//+---------------------------------------------------+
//|void InitializeListerners(const int num)
//\---------------------------------------------------+
void AudioEngine::InitializeListerners(const int num)
{
	if(num <= 0)
		return;

	for(int i = 0; i < num; i++)
		m_Listeners.push_back(ListenerOrientation());
}
//+----------------------------------------------------------------------------+
//|void UpdateChannels(const float deltaTime)
//\----------------------------------------------------------------------------+
void AudioEngine::UpdateChannels(const float deltaTime)
{
	for(rVector<Channel*>::iterator it = m_Channels.begin(); it != m_Channels.end(); it++)
	{
		(*it)->Update(deltaTime);
	}
}
}