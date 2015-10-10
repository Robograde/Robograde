/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Subsystem.h"

#include <glm/glm.hpp>
#include <memory/Alloc.h>

#include "../../../audio/AudioEngine.h"
#include "../../../audio/Sound.h"
#include "../../../audio/Channel.h"
#include "../../../audio/BeatManager.h"
#include "../../../audio/ChannelFader.h"

#define SFX_HANDLE_INVALID 0

typedef unsigned long long SFX_HANDLE;

#define g_SSAudio SSAudio::GetInstance()

//----+-----------------+------------------------------
//----| MusicEventType	|
//----+-----------------+------------------------------
//----| All the possible events that a music track may be controlled with
//----| All events (allmost)support fading in / out before its effect takes place				
//----+-----------------+------------------------------
enum class MusicEventType
{
	ADD,			///Add the music atop of the current playing music								(Fade in)
	ADD_MUTED,		///Add the music atop of the current playing music, but in a muted state		(No fading support)
	MUTE,			///Mute a specific music using its name											(Fade out)
	UNMUTE,			///Unmute a specific music using its name										(Fade in)
	PAUSE,			///Pause a specific music using its name										(Fade out)
	UNPAUSE,		///Unpause a specific music using its name										(Fade in)
	PLAY_EXCLUSIVE,	///Remove all music and play this one											(Fade in)
	STOP_ALL,		///Remove all music																(No fading support)
	STOP,			///Remove specific music using its name											(Fade out)
	CROSSFADE,		///Crossfade between the current music track(name) and a second track(FadeTo)	(Fade in and out)
};

//----+-----------------+------------------------------
//----| MusicEvent		|
//----+-----------------+------------------------------
//----| Event struct posted at the soundmanager to control music
//----| Set FadeDuration > 0.0f to make the music fade before the effect is triggered.
//----| AtBeat is used when music is added(MusicEventType::ADD), ignored by other event types
//----| FadeTo is used when crossfading two music tracks(MusicEventType::CROSSFADE)
//----+-----------------+------------------------------
struct MusicEvent
{
	MusicEvent()
		:	FadeDuration(0.0f)
		,	AtBeat(BeatType::NONE)
	{

	}

	MusicEventType	Type;
	rString			Name;
	rString			FadeTo;
	float			FadeDuration;
	BeatType		AtBeat;
};

//----+-----------------+------------------------------
//----| SFX3DInfo		|
//----+-----------------+------------------------------
//----|Used to contain data used for 3DSFX
//----+-----------------+------------------------------
struct SFX3DInfo
{
	SFX3DInfo()
		:	DistMin(10.0f)
		,	DistMax(100.0f)
		,	Is3D(false)
	{
	}

	bool		Is3D;
	float		DistMin,
				DistMax;
	glm::vec3	Position;
};

//----+-----------------+------------------------------
//----| SFXEvent		|
//----+-----------------+------------------------------
//----|Event struct posted at the soundmanager to play sound effects
//----+-----------------+------------------------------
struct SFXEvent
{
		SFXEvent()
		:	AtBeat(BeatType::NONE)
	{
	}
	rString		Name;
	BeatType	AtBeat;
	SFX3DInfo	Info3D;				///Only used for 3D, no need to modify if 3D is not used
};

//----+-----------------+------------------------------
//----| SFXCollection	|
//----+-----------------+------------------------------
//----| A collection of SFX that randomly used to get variations in sound 
//----+-----------------+------------------------------
struct SFXCollection
{
	SFXCollection() 
		:	Constraint(0.0f)
		,	Timer(0.0f)
		,	MaxInstances(0)
	{
	}

	rString				Name;
	rVector<rString>	Sounds;
	float				Constraint;
	float				Timer;
	unsigned int		MaxInstances;
};

//----+-----------------+------------------------------
//----| MusicInfo		|
//----| Used to retreive basic info about a playing music track
//----+-----------------+------------------------------
struct MusicInfo
{
	MusicInfo()
		:	Paused(false)
		,	Muted(false)
		,	Volume(1.0f)
		,	VolumeExplicit(1.0f)
	{

	}
	float	Volume,				///The modifyable volume used by the fading ///TODOJS: Make possible to modify volume externally 
			VolumeExplicit;		///The actual final volume combined with "Volume", master and musicvolume
	bool	Paused;
	bool	Muted;
};

//+===+=================+==============================
//----| SSAudio	|
//----+-----------------+------------------------------
//----|The manager of all the sounds used in the game
//----|Manages music and SFX playing events
//----|Also manages groups of sounds, makes it possible to play a random sound from that group
//+===+================================================
class SSAudio :
	public Subsystem
{
	//----+-----------------+------------------------------
	//----| MusicChannel	|
	//----+-----------------+------------------------------
	struct MusicChannel
	{
		MusicChannel()
			:	Channel(nullptr)
			,	Sound(nullptr)
			,	AtBeat(BeatType::NONE)
			,	Triggered(false)
		{

		}
		Audio::Channel*	Channel;
		Audio::Sound*	Sound;
		MusicEventType	Type;
		rString			Name;
		ChannelFader	Fader;
		BeatType		AtBeat;
		bool			Triggered;
		MusicInfo		Info;
	};

	//----+-----------------+------------------------------
	//----| SFXChannel		|
	//----+-----------------+------------------------------
	struct SFXChannel
	{
		SFXChannel()
			:	Channel(nullptr)
			,	Sound(nullptr)
			,	AtBeat(BeatType::NONE)
			,	Triggered(false)
			,	Handle(0)
		{
		}
		Audio::Channel*	Channel;
		Audio::Sound*	Sound;
		rString			Name;
		BeatType		AtBeat;
		bool			Triggered;
		SFX_HANDLE		Handle;
		SFX3DInfo		Info3D;				///Only used for 3D, no need to modify if 3D is not used
	};

	//----+-----------------+------------------------------
	//----| SFXBuffer		|
	//----+-----------------+------------------------------
	struct SFXBuffer
	{
		SFXBuffer(Audio::Sound*	buffer, const unsigned int maxInstances)
			:	Buffer(buffer)
			,	MaxInstances(maxInstances)
			,	ActiveInstances(0)
		{
		}
		Audio::Sound*	Buffer;
		unsigned int	MaxInstances;
		unsigned int	ActiveInstances;
	};
//Public Functions	 
public:
	static SSAudio&			GetInstance(void);
	~SSAudio();

	void					Startup(void) override;
	void					Shutdown(void) override;

	void					UpdateUserLayer(const float deltaTime) override;
	
	const bool				ReadConfig(const rString & path);

	void					PostEventMusic(MusicEvent event);
	SFX_HANDLE				PostEventSFX(SFXEvent event);

	const bool				LoadSFXCollection(const rString &path);
	const bool				LoadSFX(const rString &path, const unsigned int maxInstanecs);
	const bool				LoadMusic(const rString &path);

	const bool				UnloadSFXGroup(const rString &path);
	const bool				UnloadSFX(const rString &path);
	const bool				UnloadMusic(const rString &path);
	const bool				UnloadSFXAll(const rString &path);
	const bool				UnloadMusicAll(const rString &path);

	///Setters
	void					SetVolumeMaster(const float vol);
	void					SetVolumeMusic(const float vol);
	void					SetVolumeSFX(const float vol);

	void					SetBPM(const int bpm);

	bool					SetSFXPosition(SFX_HANDLE handle, glm::vec3 position);
	bool					SetSFXStop(SFX_HANDLE handle);

	///Getters
	const bool				GetIsLoaded(rString path);
	const float				GetVolumeMaster(void) const;
	const float				GetVolumeMusic(void) const;
	const float				GetVolumeSFX(void) const;

	MusicInfo*				GetMusicInfo(const rString &name);

	const int				GetBPM(void) const;
	const float				GetBeatTime(void) const;
	const float				GetBeatProgress(BeatType type = BeatType::BEAT_100) const;
	const bool				GetAtBeat(BeatType type = BeatType::BEAT_100) const;

	const bool				GetSFXPosition(SFX_HANDLE handle, glm::vec3* &positionOut);
	const bool				GetSFXActive(SFX_HANDLE handle);

//Private Functions
private:
							SSAudio();
							SSAudio(const SSAudio &ref);
	SSAudio&				operator=(const SSAudio &ref);

	const SFX_HANDLE		PlaySFXCollection(const rString &name, const BeatType atBeat = BeatType::NONE, const SFX3DInfo* info3D = nullptr);
	const SFX_HANDLE		PlaySFX(const rString &name, const BeatType atBeat = BeatType::NONE, const SFX3DInfo* info3D = nullptr);
	const bool				PlayMusic(const rString &name, const BeatType atBeat = BeatType::NONE, bool muted = false, bool looping = true);

	const bool				LoadSFXCollectionSounds(SFXCollection* &group);
	SFXCollection*			GetSFXCollection(const rString &name);
	void					UpdateSFXCollections(const float deltaTime);

	Audio::Sound*			CreateSound(const rString &path, const bool asStream = false);

	void					UpdateListener(void);
	void					UpdateVolume(void);
	void					UpdateSFXVolume(void);
	void					UpdateMusicVolume(void);

	void					UpdateMusicFading(const float deltaTime);
	void					UpdateSFXPositions(void);

	void					UpdateQueuedChannels(void);
	const bool				TriggerAtBeat(Audio::Channel* &channel, BeatType type);
	void					RemoveInactiveChannels(void);

	void					HandleMusicEvents(void);

	void					EventMusic(MusicEvent* event);
	SFX_HANDLE				EventSFX(SFXEvent* event);

	void					MusicMute(MusicChannel* music, bool mute = true);
	void					MusicMuteAll(bool mute = true);
	void					MusicPause(MusicChannel* music, bool pause = true);
	void					MusicPauseAll(bool pause = true);
	void					MusicStop(MusicChannel* music);
	void					MusicStopAll(void);

	void					MusicCrossFade(const rString &from, const rString &to, const float duration);

	MusicChannel*			GetMusic(const rString &name);
	SFXChannel*				GetSFXChannel(SFX_HANDLE handle);

//Private Variables
private:
	Audio::AudioEngine*								m_AudioEngine;

	rVector<SFXCollection*>							m_SFXCollections;
	rUnorderedMap<rString, SFXBuffer>				m_BufferSFX;
	rUnorderedMap<rString, Audio::Sound*>			m_BufferMusic;

	rUnorderedMap<SFX_HANDLE, SFXChannel>			m_SFXChannels;
	rVector<MusicChannel*>							m_MusicChannels;

	rVector<MusicEvent>								m_MusicEvents;

	BeatManager										m_BeatManager;

	bool											m_UseMusic,
													m_UseSFX;

	float											m_VolumeMaster,
													m_VolumeMusic,
													m_VolumeSFX;

	SFX_HANDLE										m_SFXHandleCounter;
};
