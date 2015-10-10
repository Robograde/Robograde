/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "ListenerOrientation.h"
#include <memory/Alloc.h>
#include "audio/AudioDLLExport.h"
namespace Audio
{
	//+===+================================================
	//----| AudioEngine |
	//+===+================================================
	class AudioEngine
	{
	//Public functions
	public:
		//General
		AUDIO_API							AudioEngine(void);
		AUDIO_API virtual					~AudioEngine(void);

		AUDIO_API virtual const bool		Initialize(const int maxChannels = 100, const int listeners = 1);
		AUDIO_API virtual void				Cleanup(void);

		AUDIO_API virtual const bool		Update(const float deltaTime);
		
		AUDIO_API virtual class Sound*		CreateSound(const rString &path, const bool asStream = false) = 0;
		AUDIO_API virtual const bool		DeleteSound(const rString &path );
		//Getters
		AUDIO_API const int				GetNumlisterners(void);
		AUDIO_API ListenerOrientation*		GetListernerorientation(const int id);
		AUDIO_API virtual class Channel*	GetFreeChannel(void);

		//Setters
		AUDIO_API virtual const bool		SetListernerOrientation(ListenerOrientation data);

	//Protected functions
	protected:
											AudioEngine(AudioEngine &_ref);
		virtual void						InitializeListerners(int num);
		virtual void						InitializeChannels(const int num) = 0;

		void								UpdateChannels(const float deltaTime);
	//Protected variables
	protected:
		rVector<class Sound*>			m_Sounds;
		rVector<class Channel*>			m_Channels;
		rVector<ListenerOrientation>	m_Listeners;
	};
}