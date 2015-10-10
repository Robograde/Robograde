/**************************************************
Copyright 2015 Jens Stjernkvist
***************************************************/

#include "SSAudio.h"

#include <fstream>

#include <utility/Logger.h>
#include <utility/Randomizer.h>
#include <utility/ConfigManager.h>

#include "../../../audio/sdl/AudioEngineSDL.h"

#include "../gamelogic/SSCamera.h"

using namespace Audio;

#define DEFAULT_VOLUME_MASTER	0.3f
#define DEFAULT_VOLUME_MUSIC	0.5f
#define DEFAULT_VOLUME_SFX		1.0f

#define USE_MUSIC				true
#define USE_SFX					true

#define DEFAULT_BPM				140
#define DEFAULT_LISTENERS		1
#define DEFAULT_CHANNELS		100
//+----------------------------------------------------------------------------+
//|SSAudio& GetInstance(void)
//\----------------------------------------------------------------------------+
SSAudio& SSAudio::GetInstance(void)
{
	static SSAudio instance;
	return instance;
}

//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSAudio::~SSAudio()
{

}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSAudio::Startup( void )
{
	m_AudioEngine = pNew(AudioEngineSDL);

	m_VolumeMaster = DEFAULT_VOLUME_MASTER;
	m_VolumeMusic = DEFAULT_VOLUME_MUSIC;
	m_VolumeSFX = DEFAULT_VOLUME_SFX;

	m_UseMusic = USE_MUSIC;
	m_UseSFX = USE_SFX;

	m_AudioEngine->Initialize(DEFAULT_CHANNELS, DEFAULT_LISTENERS);
	SetBPM(DEFAULT_BPM);

	m_BeatManager.ResetBeatCount();

	CallbackConfig* cfg = g_ConfigManager.GetConfig("audio.cfg");
	assert( cfg );
	cfg->RegisterInterest([this] (CallbackConfig*)
	{
		ReadConfig("audio.cfg");
	} );
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSAudio::Shutdown( void )
{
	for(rVector<SFXCollection*>::iterator it = m_SFXCollections.begin(); it != m_SFXCollections.end(); it++)
	{
		if(*it)
			pDelete((*it));
	}
    m_SFXCollections.clear();

	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		if(*it)
			tDelete((*it));
	}

	m_SFXChannels.clear();

	m_BufferSFX.clear();
	m_BufferMusic.clear();
	m_MusicEvents.clear();
    m_MusicChannels.clear();

	pDelete(m_AudioEngine);

	m_AudioEngine = nullptr;
}

//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateUserLayer(const float deltaTime)
{
	m_BeatManager.Update(deltaTime);
	UpdateListener();
	UpdateQueuedChannels();
	UpdateSFXCollections(deltaTime);
	HandleMusicEvents();
	UpdateMusicFading(deltaTime);
	RemoveInactiveChannels();
	UpdateSFXPositions();

	m_AudioEngine->Update(deltaTime);
}
//+----------------------------------------------------------------------------+
//|const bool ReadConfig(const rString & path)
//\----------------------------------------------------------------------------+
const bool SSAudio::ReadConfig(const rString & path)
{
	CallbackConfig* config = g_ConfigManager.GetConfig( path );
	if( config == nullptr )
	{
		Logger::Log("Failed to read audio settings config", "SSAudio", LogSeverity::WARNING_MSG);
		return false;
	}

	m_VolumeMaster = config->GetFloat("volumeMaster", DEFAULT_VOLUME_MASTER);
	SetVolumeMaster( m_VolumeMaster );
	m_VolumeMusic = config->GetFloat("volumeMusic", DEFAULT_VOLUME_MUSIC);
	SetVolumeMusic( m_VolumeMusic );
	m_VolumeSFX = config->GetFloat("volumeSFX", DEFAULT_VOLUME_SFX);
	SetVolumeSFX( m_VolumeSFX );

	m_UseMusic = config->GetBool("useMusic", true);
	m_UseSFX = config->GetBool("useSFX", true);

	///Update volume of music with the new settings and mute / unmute if music is disabled
	MusicMuteAll(!m_UseMusic);
	MusicPauseAll(!m_UseMusic);
	UpdateVolume();

	return true;
}
//+----------------------------------------------------------------------------+
//|void PostEventMusic(MusicEvent event)
//\----------------------------------------------------------------------------+
void SSAudio::PostEventMusic( MusicEvent event )
{
	m_MusicEvents.push_back(event);
}
//+----------------------------------------------------------------------------+
//|SFX_HANDLE PostEventSFX(SFXEvent event)
//|Returns a unique SFX_HANDLE if the sfx was succesfully started
//|Returns SFX_HANDLE == 0 of failed
//\----------------------------------------------------------------------------+
SFX_HANDLE SSAudio::PostEventSFX(SFXEvent event)
{
	return EventSFX(&event);
}
//+----------------------------------------------------------------------------+
//|const bool LoadSFXCollection(const rString &path)
//|Load a collection of sounds using the path of a sfxc-File
//\----------------------------------------------------------------------------+
const bool SSAudio::LoadSFXCollection(const rString &path)
{
	MemoryAllocator::ResetPos();

	///Try to load the file
	std::ifstream file;
    file.open(path.c_str());

	///Error check, see if a file could be located.
    if(!file)
	{
		Logger::Log("LoadSFXCollection. Loading sfxcollection failed, invalid path:" + path, "SSAudio", LogSeverity::WARNING_MSG);

		file.close();
		return false; 
    }

	///Check if its a .sfxc file that its requested to read from
	rString line;
	rString::size_type location = path.find(".sfxc", 0);

	if(location != rString::npos)
	{
		SFXCollection* collection = pNew(SFXCollection);
		collection->Name = path;

		///Continue to read from the file untill the "end-of-file" is found.
		while(!file.eof())
		{
			///Read a line form the file and store it as a string
			std::getline(file, line);

			///Locate the '='-charcter, it will be used to seperate the id from the value.
			location = line.find("=", 0);

			///Check if the line is valid, did it contain an '=' character at all?
			if(location != rString::npos) 
			{
				rString id = line;
				rString value = line;

				///Separate the id and value
				id = id.substr(0, location);
				value = value.substr(location + 1 , value.size());

				///Get all the values  
				if(id == "soundPath")
				{
					collection->Sounds.push_back(value);
				}
				else if(id == "constraint")
				{
					collection->Constraint = static_cast<float>(atof(value.c_str()));
				}
				else if(id == "instances")
				{
					collection->MaxInstances = static_cast<unsigned int>(atoi(value.c_str()));
				}
				else
				{
					Logger::Log("LoadSFXCollection. Reading sfxcollection failed, no matching id:" + id, "SSAudio", LogSeverity::WARNING_MSG);
					pDelete(collection);
					file.close();
					return false;
				}

			}
		}

		//Load the sounds in the collection
		if(LoadSFXCollectionSounds(collection))
		{
			m_SFXCollections.push_back(collection);
			file.close();
			return true;
		}

		pDelete(collection);
		Logger::Log("LoadSFXCollection. Failed loading sfxcollection, error loading sounds in collection:" + path, "SSAudio", LogSeverity::WARNING_MSG);
	}
	///The requested file isn not a sfxc-file
	else
	{
		Logger::Log("LoadSFXCollection. Loading sfxcollection failed, not a sfxc - file:" + path, "SSAudio", LogSeverity::WARNING_MSG);
    }

	file.close();
	return false;
}
//+----------------------------------------------------------------------------+
//|const bool LoadSFX(const rString &path, const unsigned int maxInstanecs
//|Load the sound using the input path, only sounds not previously loaded are created
//|A certain number of maximum instances of the SFX may be set. 
//\----------------------------------------------------------------------------+
const bool SSAudio::LoadSFX(const rString &path, const unsigned int maxInstanecs)
{
	MemoryAllocator::ResetPos();

	///Check if the sound needs to be loaded
	///Check if sound already was created, otherwise create it
	rUnorderedMap<rString, SFXBuffer>::iterator it = m_BufferSFX.find(path);

	if(it == m_BufferSFX.end())
	{
		///No mathing name found, create the sound
		Audio::Sound* sfx = CreateSound(path);

		if(sfx)
		{
			m_BufferSFX.emplace(path, SFXBuffer(sfx, maxInstanecs));
			return true;
		}

		Logger::Log("Error LoadSFX: Invalid path!" + path, "SSAudio", LogSeverity::WARNING_MSG );

		return false;
	}

	//Sound already exists
	return true;
}
//+----------------------------------------------------------------------------+
//|const bool LoadMusic(const rString &path)
//|Load the sound using the input path, only sounds not previously loaded are created
//\----------------------------------------------------------------------------+
const bool SSAudio::LoadMusic(const rString &path)
{
	MemoryAllocator::ResetPos();
	///Check if the music needs to be loaded
	///Check if music already was created, otherwise create it
	rUnorderedMap<rString, Audio::Sound*>::iterator it = m_BufferMusic.find(path);

	if(it == m_BufferMusic.end())
	{
		///No mathing name found, create the music
		Audio::Sound* music = CreateSound(path, true);

		if(music)
		{
			m_BufferMusic.emplace(path, music);
			return true;
		}

		Logger::Log("Error LoadMusic: Invalid path!" + path, "SSAudio", LogSeverity::WARNING_MSG );

		return false;
	}

	//Music already exists
	return true;
}
//+----------------------------------------------------------------------------+
//|const bool UnloadSFX(rString &path)
//|Unload a loaded sound effect using the input path
//\----------------------------------------------------------------------------+
const bool SSAudio::UnloadSFX(const rString &path)
{
	rUnorderedMap<rString, SFXBuffer>::iterator it = m_BufferSFX.find(path);

	if(it == m_BufferSFX.end())
	{
		Logger::Log("UnloadSFX: Sound not loaded!" + path, "SSAudio", LogSeverity::WARNING_MSG );

		return false;
	}

	///Check if there are SFXchannels that are using this loaded data and remove them
	for(rUnorderedMap<SFX_HANDLE, SFXChannel>::iterator jt = m_SFXChannels.begin(); jt != m_SFXChannels.end();)
	{
		if(strcmp((*jt).second.Name.c_str(), path.c_str()) == 0)
		{
			if((*jt).second.Channel)
				(*jt).second.Sound->Stop((*jt).second.Channel);

			jt = m_SFXChannels.erase(jt);
		}
		else
			++jt;
	}

	///Finally delete the SFX from memory
	m_AudioEngine->DeleteSound(path.c_str());
	m_BufferSFX.erase(it);

	return true;
}
//+----------------------------------------------------------------------------+
//|const bool UnloadMusic(const rString &path)
//|Unload a loaded music track using the input path
//\----------------------------------------------------------------------------+
const bool SSAudio::UnloadMusic(const rString &path)
{
	///Check if the music has been loaded
	rUnorderedMap<rString, Audio::Sound*>::iterator it = m_BufferMusic.find(path);

	if(it == m_BufferMusic.end())
	{
		Logger::Log("UnloadMusic: Sound not loaded!" + path, "SSAudio", LogSeverity::WARNING_MSG );

		return false;
	}

	///Check if there are musicchannels that are using this track and remove them
	for(rVector<MusicChannel*>::iterator jt = m_MusicChannels.begin(); jt != m_MusicChannels.end();)
	{
		if(strcmp((*jt)->Name.c_str(), path.c_str()) == 0)
		{
			MusicStop((*jt));
			tDelete((*jt));
			jt = m_MusicChannels.erase(jt);
		}
		else
		{
			++jt;
		}

	}

	///Finally delete the music from memory
	m_AudioEngine->DeleteSound(path.c_str());
	m_BufferMusic.erase(it);

	return true;
}
//+----------------------------------------------------------------------------+
//|void SetVolumeMaster(const float vol)
//\----------------------------------------------------------------------------+
void SSAudio::SetVolumeMaster( const float vol )
{
	m_VolumeMaster = vol;
	UpdateVolume();
}
//+----------------------------------------------------------------------------+
//|void SetVolumeMusic(const float vol)
//\----------------------------------------------------------------------------+
void SSAudio::SetVolumeMusic( const float vol )
{
	m_VolumeMusic = vol;
	UpdateMusicVolume();
}
//+----------------------------------------------------------------------------+
//|void SetVolumeSFX(const float vol)
//\----------------------------------------------------------------------------+
void SSAudio::SetVolumeSFX( const float vol )
{
	m_VolumeSFX = vol;
	UpdateSFXVolume();
}
//+---------------------------------------------------+
//|void SetBpm(const int bpm)
//\---------------------------------------------------+
void SSAudio::SetBPM( const int bpm )
{
	m_BeatManager.SetBPM(bpm);
	m_BeatManager.ResetBeatCount();
}
//+---------------------------------------------------+
//|bool SetSFXPosition(SFX_HANDLE handle, glm::vec3 position)
//|Sets the postion of the located channel, it the channel is not found false is returned
//\---------------------------------------------------+
bool SSAudio::SetSFXPosition(SFX_HANDLE handle, glm::vec3 position)
{
	SFXChannel* channel = GetSFXChannel(handle);

	if(channel)
	{
		if(channel->Info3D.Is3D)
		{
			channel->Info3D.Position = position;
			return true;
		}
	}

	return false;
}
//+---------------------------------------------------+
//|bool SetSFXStop(SFX_HANDLE handle)
//|Sets the postion of the located channel, it the channel is not found false is returned
//\---------------------------------------------------+
bool SSAudio::SetSFXStop(SFX_HANDLE handle)
{
	SFXChannel* channel = GetSFXChannel(handle);

	if(channel)
	{
		channel->Sound->Stop(channel->Channel);
		return true;
	}

	return false;
}
//+----------------------------------------------------------------------------+
//|const float GetVolumeMaster(void)
//\----------------------------------------------------------------------------+
const float	SSAudio::GetVolumeMaster( void ) const
{
	return m_VolumeMaster;
}
//+----------------------------------------------------------------------------+
//|const float GetVolumeMusic(void)
//\----------------------------------------------------------------------------+
const float	SSAudio::GetVolumeMusic( void ) const
{
	return m_VolumeMusic;
}
//+----------------------------------------------------------------------------+
//|const float GetVolumeSFX(void)
//\----------------------------------------------------------------------------+
const float	SSAudio::GetVolumeSFX( void ) const
{
	return m_VolumeSFX;
}
//+----------------------------------------------------------------------------+
//|MusicInfo* GetMusicInfo(const rString &name)
//\----------------------------------------------------------------------------+
MusicInfo* SSAudio::GetMusicInfo(const rString &name)
{
	MusicChannel* channel = GetMusic(name);

	if(channel)
		return &channel->Info;

	return nullptr;
}
//+----------------------------------------------------------------------------+
//|const int GetBPM(void)
//\----------------------------------------------------------------------------+
const int SSAudio::GetBPM( void ) const
{
	return m_BeatManager.GetBPM();
}
//+----------------------------------------------------------------------------+
//|const float GetBeatTime(void)
//|Returns the time in seconds on how long a beat is
//\----------------------------------------------------------------------------+
const float	SSAudio::GetBeatTime( void ) const
{
	return m_BeatManager.GetBeatTime();
}
//+----------------------------------------------------------------------------+
//|const float GetBeatProgress(BeatType beat)
//|Get the progress of the input beat, ranged from 0.0f -> 1.0f
//\----------------------------------------------------------------------------+
const float SSAudio::GetBeatProgress( BeatType type ) const
{
	return m_BeatManager.GetBeatProgress(type);
}
//+----------------------------------------------------------------------------+
//|const bool GetAtBeat(BeatType type)
//\----------------------------------------------------------------------------+
const bool SSAudio::GetAtBeat( BeatType type ) const
{
	if(type == BeatType::NONE)
		return false;

	return m_BeatManager.GetAtBeat(type);
}
//+---------------------------------------------------+
//|const bool GetSFXPosition(SFX_HANDLE handle, glm::vec3* &position)
//|Get the postion of the located channel, it the channel is not found false is returned
//\---------------------------------------------------+
const bool SSAudio::GetSFXPosition(SFX_HANDLE handle, glm::vec3* &positionOut)
{
	SFXChannel* channel = GetSFXChannel(handle);

	if(channel)
	{
		if(channel->Info3D.Is3D)
		{
			positionOut->x = channel->Info3D.Position.x;
			positionOut->y = channel->Info3D.Position.y;
			positionOut->z = channel->Info3D.Position.z;
			return true;
		}
	}
	
	positionOut->x = positionOut->y = positionOut->z = 0.0f;
	
	return false;
}
//+---------------------------------------------------+
//|const bool SetSFXPosition(SFX_HANDLE handlen)
//|Get if the channel exists that the input handle belongs to, it the channel is not found false is returned
//\---------------------------------------------------+
const bool SSAudio::GetSFXActive(SFX_HANDLE handle)
{
	return GetSFXChannel(handle) != nullptr;
}
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SSAudio::SSAudio( )
	:	Subsystem("Audio")
	,	m_AudioEngine(nullptr)
	,	m_SFXHandleCounter(0)
	,	m_UseMusic(false)
	,	m_UseSFX(false)
{
}
//+----------------------------------------------------------------------------+
//|const SFX_HANDLE PlaySFXCollection(const rString &name, const BeatType atBeat, const SFX3DInfo* info3D)
//|Locate the group with the input name, and play a random sound of that group on the input position
//\----------------------------------------------------------------------------+
const SFX_HANDLE SSAudio::PlaySFXCollection(const rString &name, const BeatType atBeat, const SFX3DInfo* info3D)
{
	SFXCollection* group = GetSFXCollection(name);

	if(group == nullptr)
		return false;

	unsigned int id = g_Randomizer.UserRand( 0, static_cast<unsigned int>(group->Sounds.size( ) ));
	rString soundName = group->Sounds[id];
	return PlaySFX(soundName, atBeat, info3D);
}
//+----------------------------------------------------------------------------+
//|const SFX_HANDLE PlaySFX(const rString &name, const BeatType atBeat, const SFX3DInfo* info3D)
//|Locate the Sound with the input name, and play the sound at the input position
//|A SFX_HANDLE is returned for access later
//\----------------------------------------------------------------------------+
const SFX_HANDLE SSAudio::PlaySFX(const rString &name, const BeatType atBeat, const SFX3DInfo* info3D)
{
	SFX_HANDLE out = SFX_HANDLE_INVALID;

	if(m_UseSFX)
	{
		rUnorderedMap<rString, SFXBuffer>::iterator it = m_BufferSFX.find(name);

		if(it == m_BufferSFX.end())
		{
			Logger::Log("PlaySFX. Trying to play SFX that's not loaded!" + name, "SSAudio", LogSeverity::WARNING_MSG );

			return false;
		}

		///Dont play more instances of an SFX than are set as maximum
		if((*it).second.ActiveInstances >= (*it).second.MaxInstances)
			return out;

		Audio::Channel* channel = nullptr;
		SFXChannel sfx;
		///Check if the sound should be in 3D. Create, set volume, position and min/max distance of the channel
		///Is 3D
		if(info3D->Is3D)
		{
			channel = (*it).second.Buffer->Play(nullptr, Audio::IS_3D | Audio::PAUSED);	
			channel->SetPosition(Audio::Vector3(info3D->Position.x, info3D->Position.y, info3D->Position.z));
			channel->SetMinMaxDistance(info3D->DistMin, info3D->DistMax);
			sfx.Info3D.Is3D = info3D->Is3D;
			sfx.Info3D.DistMax = info3D->DistMax;
			sfx.Info3D.DistMin = info3D->DistMin;
			sfx.Info3D.Position = info3D->Position;
		}
		///Not 3D
		else
		{
			channel = (*it).second.Buffer->Play(nullptr, Audio::PAUSED);	
		}

		channel->SetVolume(m_VolumeMaster * m_VolumeSFX);
		channel->UpdateVolume();

		sfx.Channel = channel;
		sfx.Sound = (*it).second.Buffer;
		sfx.AtBeat = atBeat;
		sfx.Name = name;

		///Assign unique handle to SFX
		m_SFXHandleCounter++;
		sfx.Handle = m_SFXHandleCounter;
		if(atBeat == BeatType::NONE)
		{
			channel->Unpause();
			sfx.Triggered = true;
		}

		m_SFXChannels.emplace(m_SFXHandleCounter, sfx);
		(*it).second.ActiveInstances++;
		out = sfx.Handle;

		if((*it).second.ActiveInstances > (*it).second.MaxInstances)
			Logger::Log("Instances:" + rToString((*it).second.ActiveInstances) + (*it).second.Buffer->GetName(), "SSAudio", LogSeverity::INFO_MSG );

	}

	return out;
}
//+----------------------------------------------------------------------------+
//|const bool PlayMusic(const rString &name, const BeatType atBeat, bool muted, bool looping)
//|Locate the music with the input name and play
//\----------------------------------------------------------------------------+
const bool SSAudio::PlayMusic(const rString &name, const BeatType atBeat, bool muted, bool looping)
{
	rUnorderedMap<rString, Audio::Sound*>::iterator it = m_BufferMusic.find(name);

	if(it == m_BufferMusic.end())
	{
		Logger::Log("PlayMusic. Trying to play music that's not loaded!" + name, "SSAudio", LogSeverity::WARNING_MSG );

		return false;
	}

	Audio::Channel* channel = (*it).second->Play(nullptr, looping ? Audio::LOOPING | Audio::PAUSED : Audio::PAUSED);

	if(channel == nullptr)
	{
		Logger::Log("PlayMusic. Trying to play music, channel not received" + name, "SSAudio", LogSeverity::WARNING_MSG );

		return false;
	}

	channel->SetVolume(m_VolumeMaster * m_VolumeMusic);
	channel->UpdateVolume();

	channel->SetLooping(looping);

	MusicChannel* music = tNew(MusicChannel);
	music->Channel = channel;
	music->Sound = (*it).second;
	music->AtBeat = atBeat;
	music->Name = name;

	if(atBeat == BeatType::NONE)
	{
		channel->Unpause();
		music->Triggered = true;
	}

	if(muted || !m_UseMusic)
		channel->Mute();

	m_MusicChannels.push_back(music);

	return true;
}
//+----------------------------------------------------------------------------+
//|const bool LoadSFXCollectionSounds(SoundGroup* group)
//|Load a collection of sounds using the data in the input SoundGroup
//\----------------------------------------------------------------------------+
const bool SSAudio::LoadSFXCollectionSounds( SFXCollection* &group )
{
	///Check if the sounds in the group needs to be loaded
	///Check if sound already was created, otherwise create it
	rVector<rString> paths = group->Sounds;
	bool success = true;

	for(rVector<rString>::iterator it = paths.begin(); it != paths.end(); it++)
	{
		success = LoadSFX((*it), group->MaxInstances);

		if(!success)
			return false;
	}

	return true;
}
//+----------------------------------------------------------------------------+
//|SFXCollection* GetSFXCollection(const rString &name)
//|Locate the group the input name belongs to
//\----------------------------------------------------------------------------+
SFXCollection* SSAudio::GetSFXCollection(const rString &name)
{
	for(rVector<SFXCollection*>::iterator it = m_SFXCollections.begin(); it != m_SFXCollections.end(); it++)
	{
		if(strcmp((*it)->Name.c_str(), name.c_str()) == 0)
		{
			return (*it);
		}
	}

	return nullptr;
}
//+----------------------------------------------------------------------------+
//|void UpdateSFXCollections(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateSFXCollections( const float deltaTime )
{
	///Update the timers of the groups that uses time constraints
	for(rVector<SFXCollection*>::iterator it = m_SFXCollections.begin(); it != m_SFXCollections.end(); it++)
	{
		if((*it)->Constraint == 0.0f)
			continue;

		(*it)->Timer += deltaTime;
	}
}
//+----------------------------------------------------------------------------+
//|Audio::Sound* CreateSound(const rString &path, const bool asStream)
//\----------------------------------------------------------------------------+
Audio::Sound* SSAudio::CreateSound(const rString &path, const bool asStream)
{
	Audio::Sound* sound = m_AudioEngine->CreateSound(path.c_str(), asStream);

	if(!sound)
	{
		Logger::Log("Failed to create sound!" + path, "SSAudio", LogSeverity::WARNING_MSG );
	}

	return sound;
}
//+----------------------------------------------------------------------------+
//|void UpdateListener(void)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateListener( void )
{
	Camera* camera = g_SSCamera.GetActiveCamera();

	if(camera)
	{
		glm::vec3 pos = camera->GetPosition();
		glm::vec3 up = camera->GetUp();
		glm::vec3 forward = camera->GetForward();

		Audio::ListenerOrientation lo;
		lo.Id = 0;
		lo.Px = pos.x;
		lo.Py = pos.y;
		lo.Pz = pos.z;
		lo.Ux = up.x;
		lo.Uy = up.y;
		lo.Uz = up.z;
		lo.Fx = forward.x;
		lo.Fy = forward.y;
		lo.Fz = forward.z;
		m_AudioEngine->SetListernerOrientation(lo);
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateVolume(void)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateVolume( void )
{
	UpdateSFXVolume();
	UpdateMusicVolume();
}
//+----------------------------------------------------------------------------+
//|void UpdateSFXVolume(void)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateSFXVolume( void )
{
	for(rUnorderedMap<SFX_HANDLE, SFXChannel>::iterator it = m_SFXChannels.begin(); it != m_SFXChannels.end(); it++)
	{
		(*it).second.Channel->SetVolume(m_VolumeMaster * m_VolumeSFX);
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateMusicVolume(void)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateMusicVolume( void )
{
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		(*it)->Channel->SetVolume((*it)->Info.Volume * m_VolumeMaster * m_VolumeMusic);
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateMusicFading(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSAudio::UpdateMusicFading( const float deltaTime )
{
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		if(!(*it)->Fader.GetActive())
			continue;

		(*it)->Fader.Update(deltaTime);
		(*it)->Info.Volume = (*it)->Fader.GetVolume();
		(*it)->Info.VolumeExplicit = (*it)->Info.Volume * m_VolumeMaster * m_VolumeMusic;

		(*it)->Channel->SetVolume((*it)->Info.VolumeExplicit);

		if((*it)->Fader.GetFinished())
		{
			FadeEvent event = (*it)->Fader.GetEvent();

			switch(event)
			{
				case FadeEvent::MUTE:
				{
					MusicMute((*it), true);
				}
				break;

				case FadeEvent::PAUSE:
				{
					MusicPause((*it), true);
				}
				break;

				case FadeEvent::STOP:
				{
					MusicStop((*it));
				}
				break;

				default:
				break;
			}

			(*it)->Fader.Reset();
		}
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateSFXPositions(void)
//|Update the positions of the channels that uses 3D
//\----------------------------------------------------------------------------+
void SSAudio::UpdateSFXPositions(void)
{
	for(rUnorderedMap<SFX_HANDLE, SFXChannel>::iterator it = m_SFXChannels.begin(); it != m_SFXChannels.end(); it++)
	{
		if((*it).second.Info3D.Is3D)
		{
			Audio::Vector3 pos = Audio::Vector3((*it).second.Info3D.Position.x, (*it).second.Info3D.Position.y, (*it).second.Info3D.Position.z);
			(*it).second.Channel->SetPosition(pos);
		}
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateQueuedChannels(void)
//|Used to play sounds timed at the closest beat
//|Check if all the delayed channels waiting for the next beat should be activated
//|These channels may only be activated once per beat
//\----------------------------------------------------------------------------+
void SSAudio::UpdateQueuedChannels( void )
{
	///Music
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		if((*it)->AtBeat != BeatType::NONE && !(*it)->Triggered)
		{
			(*it)->Triggered = TriggerAtBeat((*it)->Channel, (*it)->AtBeat);
		}
	}

	///SFX
	for(rUnorderedMap<SFX_HANDLE, SFXChannel>::iterator it = m_SFXChannels.begin(); it != m_SFXChannels.end(); it++)
	{
		if((*it).second.AtBeat != BeatType::NONE && !(*it).second.Triggered)
		{
			(*it).second.Triggered = TriggerAtBeat((*it).second.Channel, (*it).second.AtBeat);
		}
	}
}
//+----------------------------------------------------------------------------+
//|const bool TriggerAtBeat(Audio::Channel* &channel, BeatType type)
//|If the input beat type is triggerd during this call, unpause the delayed input channel
//\----------------------------------------------------------------------------+
const bool SSAudio::TriggerAtBeat( Audio::Channel* &channel, BeatType type )
{
	bool triggered = false;

	if(type == BeatType::BEAT_25 && GetAtBeat(BeatType::BEAT_25))
		triggered = true;
	else if(type == BeatType::BEAT_50 && GetAtBeat(BeatType::BEAT_50))
		triggered = true;
	else if(type == BeatType::BEAT_100 && GetAtBeat(BeatType::BEAT_100))
		triggered = true;
	else if(type == BeatType::BEAT_200 && GetAtBeat(BeatType::BEAT_200))
		triggered = true;
	else if(type == BeatType::BEAT_400 && GetAtBeat(BeatType::BEAT_400))
		triggered = true;
	else if(type == BeatType::BEAT_800 && GetAtBeat(BeatType::BEAT_800))
		triggered = true;

	if(triggered)
	{
		channel->Unpause();
	}

	return triggered;
}
//+----------------------------------------------------------------------------+
//|void RemoveInactiveChannels(void)
//\----------------------------------------------------------------------------+
void SSAudio::RemoveInactiveChannels( void )
{
	for(rUnorderedMap<SFX_HANDLE, SFXChannel>::iterator it = m_SFXChannels.begin(); it != m_SFXChannels.end();)
	{
		if((*it).second.Channel->IsReleased())
		{
			rUnorderedMap<rString, SFXBuffer>::iterator bufferIt = m_BufferSFX.find((*it).second.Name);

			if(bufferIt != m_BufferSFX.end())
			{
				if((*bufferIt).second.ActiveInstances != 0)
					(*bufferIt).second.ActiveInstances--;
			}

			it = m_SFXChannels.erase(it);
		}
		else
		{
			++it;	
		}
	}

	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end();)
	{
		if((*it)->Channel->IsReleased())
		{
 			tDelete(*it);
			(*it) = nullptr;
			it = m_MusicChannels.erase(it);
		}
		else
			++it;
	}
}
//+----------------------------------------------------------------------------+
//|void HandleMusicEvents(void)
//|Handle all the music related events that was posted since last update
//\----------------------------------------------------------------------------+
void SSAudio::HandleMusicEvents(void)
{
	///Music
	for(rVector<MusicEvent>::iterator it = m_MusicEvents.begin(); it != m_MusicEvents.end(); it++)
	{
		EventMusic(&(*it));
	}

	m_MusicEvents.clear();
}
//+----------------------------------------------------------------------------+
//|void EventMusic(MusicEvent* event)
//|Handle a music event
//\----------------------------------------------------------------------------+
void SSAudio::EventMusic( MusicEvent* event )
{
	MusicChannel* music = nullptr;
	float volume = 0.0f;
	
	if(event->Type != MusicEventType::ADD && event->Type != MusicEventType::ADD_MUTED && event->Type != MusicEventType::STOP_ALL)
	{
		music = GetMusic(event->Name);

		if(music)
			volume = music->Info.Volume;
		else
			return;
	}

	switch(event->Type)
	{
		case MusicEventType::ADD:
		{
			if(!PlayMusic(event->Name, event->AtBeat, false, true))
				return;

			if(event->FadeDuration > 0)
			{
				music = GetMusic(event->Name);///TODOJS: Make this less h4xx...Music created above is searched for again.
				if(music)
				{
					music->Info.Volume = 0.0f;
					volume = music->Info.Volume;
					music->Fader.Start(FadeType::FADE_IN, FadeEvent::ADD, volume, event->FadeDuration);
				}
			}

			break;
		}

		case MusicEventType::ADD_MUTED:
		{
			if(!PlayMusic(event->Name, event->AtBeat, true, true))
				return;
			break;
		}

		case MusicEventType::MUTE:
		{
			if(event->FadeDuration > 0)
				music->Fader.Start(FadeType::FADE_OUT, FadeEvent::MUTE, volume, event->FadeDuration);
			else
				MusicMute(music, true);
			break;
		}

		case MusicEventType::UNMUTE:
		{
			if(event->FadeDuration > 0)
			{
				MusicMute(music, false);
				music->Fader.Start(FadeType::FADE_IN, FadeEvent::UNMUTE, volume, event->FadeDuration);
			}
			else
				MusicMute(music, false);
			break;
		}

		case MusicEventType::PAUSE:
		{
			if(event->FadeDuration > 0)
				music->Fader.Start(FadeType::FADE_OUT, FadeEvent::PAUSE, volume, event->FadeDuration);
			else
				MusicPause(music, true);
			break;
		}

		case MusicEventType::UNPAUSE:
		{
			if(event->FadeDuration > 0)
			{
				MusicPause(music, false);
				music->Fader.Start(FadeType::FADE_IN, FadeEvent::UNPAUSE, volume, event->FadeDuration);
			}

			else
				MusicPause(music, false);
			break;
		}

		case MusicEventType::PLAY_EXCLUSIVE:
		{
			MusicStopAll();
			PlayMusic(event->Name, event->AtBeat);
			break;
		}

		case MusicEventType::STOP_ALL:
		{
			MusicStopAll();
			break;
		}

		case MusicEventType::STOP:
		{
			if(event->FadeDuration > 0)
			{
				music->Fader.Start(FadeType::FADE_OUT, FadeEvent::STOP, volume, event->FadeDuration);
			}

			else
				MusicStop(music);
			break;
		}

		case MusicEventType::CROSSFADE:
		{
			MusicCrossFade(event->Name, event->FadeTo, event->FadeDuration);
			break;
		}

		default:
		{
			Logger::Log("EventMusic: No such EventType!" + rToString(static_cast<int>(event->Type)), "SSAudio", LogSeverity::WARNING_MSG );
			break;
		}
	};
}
//+----------------------------------------------------------------------------+
//|SFX_HANDLE EventSFX(SFXEvent* event)
//|Handle a SFX event
//\----------------------------------------------------------------------------+
SFX_HANDLE SSAudio::EventSFX(SFXEvent* event)
{
	rString::size_type location = event->Name.find(".sfxc", 0);

	///Check if its a SFXCollection that should be used
	///Is collection
	if(location != rString::npos) 
	{
		///Locate collection
		SFXCollection* collection = GetSFXCollection(event->Name);

		if(collection == nullptr)
		{
			Logger::Log("EventSFX: No such collection!" + event->Name, "SSAudio", LogSeverity::WARNING_MSG );
			return false;
		}

		///Group uses time constraint
		if(collection->Constraint > 0)
		{
			if(collection->Timer >= collection->Constraint)
			{
				collection->Timer = 0.0f;
				return PlaySFXCollection(event->Name, event->AtBeat, &event->Info3D);
			}
		}
		///No constraint
		else
		{
			return PlaySFXCollection(event->Name, event->AtBeat, &event->Info3D);
		}
	}
	///Single
	else
	{
		return PlaySFX(event->Name, event->AtBeat, &event->Info3D);
	}

	return 0;
}
//+----------------------------------------------------------------------------+
//|void MusicMute(MusicChannel* music, bool mute)
//\----------------------------------------------------------------------------+
void SSAudio::MusicMute( MusicChannel* music, bool mute )
{
	if(!music)
		return;

	music->Info.Muted = mute;

	if(mute)
		music->Channel->Mute();
	else
		music->Channel->Unmute();
}
//+----------------------------------------------------------------------------+
//|void MusicMuteAll(bool mute)
//\----------------------------------------------------------------------------+
void SSAudio::MusicMuteAll(bool mute)
{
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		MusicMute((*it), mute);
	}
}
//+----------------------------------------------------------------------------+
//|void MusicPause(MusicChannel* music, bool pause)
//\----------------------------------------------------------------------------+
void SSAudio::MusicPause( MusicChannel* music, bool pause )
{
	if(!music)
		return;

	music->Info.Paused = pause;

	if(pause)
		music->Channel->Pause();
	else
		music->Channel->Unpause();
}
//+----------------------------------------------------------------------------+
//|void MusicPauseAll(bool pause)
//\----------------------------------------------------------------------------+
void SSAudio::MusicPauseAll(bool pause)
{
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		MusicPause((*it), pause);
	}
}
//+----------------------------------------------------------------------------+
//|void MusicStop(MusicChannel* music)
//\----------------------------------------------------------------------------+
void SSAudio::MusicStop( MusicChannel* music )
{
	if(music)
	{
		music->Sound->Stop(music->Channel);
	}
}
//+----------------------------------------------------------------------------+
//|void MusicStopAll(void)
//\----------------------------------------------------------------------------+
void SSAudio::MusicStopAll( void )
{
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		MusicStop((*it));
	}
}
//+----------------------------------------------------------------------------+
//|void MusicCrossFade(const rString from, const rString to, const float duration)
//|Trigger activation of crossfading between two tracks, one that exists and one to be started
//\----------------------------------------------------------------------------+
void SSAudio::MusicCrossFade(const rString &from, const rString &to, const float duration)
{
	///Make sure both music tracks exists, is added and is active(not muted and / or paused)

	///Check if the fadeTo music exists
	MusicChannel* fadeTo = GetMusic(to);

	//If it does, check if it needs to be unmuted /unpaused
	if(fadeTo)
	{
		if(fadeTo->Info.Muted)
			MusicMute(fadeTo, false);

		if(fadeTo->Info.Paused)
			MusicPause(fadeTo, false);
	}
	///If not, create it
	else
	{
		PlayMusic(to, BeatType::NONE, false, true);
		fadeTo = GetMusic(to);		//TODOJS: Make this less h4xx...Music created above is searched for again.
		fadeTo->Info.Volume = 0.0f;
		fadeTo->Info.VolumeExplicit = 0.0f;
	}

	///Same goes for the fading out music
	MusicChannel* fadeFrom = GetMusic(from);

	if(fadeFrom)
	{
		if(fadeFrom->Info.Muted)
			MusicMute(fadeFrom, false);

		if(fadeFrom->Info.Paused)
			MusicPause(fadeFrom, false);
	}

	fadeTo->Fader.Start(FadeType::FADE_IN, FadeEvent::ADD, fadeTo->Info.Volume, duration);
	fadeFrom->Fader.Start(FadeType::FADE_OUT, FadeEvent::STOP, fadeFrom->Info.Volume, duration);
}
//+----------------------------------------------------------------------------+
//|MusicChannel* GetMusic(const rString &name)
//\----------------------------------------------------------------------------+
SSAudio::MusicChannel* SSAudio::GetMusic(const rString &name)
{
	for(rVector<MusicChannel*>::iterator it = m_MusicChannels.begin(); it != m_MusicChannels.end(); it++)
	{
		if(strcmp(name.c_str(), (*it)->Name.c_str()) == 0)
		{
			return (*it);
		}
	}

	return nullptr;
}
//+----------------------------------------------------------------------------+
//|SFXChannel* GetSFXChannel(SFX_HANDLE handle)
//\----------------------------------------------------------------------------+
SSAudio::SFXChannel* SSAudio::GetSFXChannel(SFX_HANDLE handle)
{
	///Check if the music has been loaded
	rUnorderedMap<SFX_HANDLE, SFXChannel>::iterator it = m_SFXChannels.find(handle);

	if(it == m_SFXChannels.end())
	{
		return nullptr;
	}

	return &(*it).second;
}
