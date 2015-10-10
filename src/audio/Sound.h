/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "audio/AudioDLLExport.h"
namespace Audio
{
	//----+-----------------+------------------------------
	//----| SoundType |
	//----+-----------------+------------------------------
	enum class SoundType
	{
		SAMPLE,
		STREAM,
	};

	//+===+================================================
	//----| Sound	|
	//+===+================================================
	class Sound
	{
		//Public functions
	public:
		//General
		AUDIO_API							Sound(void);
		AUDIO_API virtual					~Sound(void);

		AUDIO_API virtual bool				Initialize( class AudioEngine* engine, const rString &path, SoundType type ) = 0;
		AUDIO_API virtual void				Cleanup(void) = 0;

		AUDIO_API virtual class Channel*	Play(class Channel* dedicatedChannel = 0, int flags = 0) = 0;
		AUDIO_API virtual void				Stop(class Channel* channel) = 0;
		
		AUDIO_API const SoundType			GetType(void);
		AUDIO_API const rString				GetName(void);

		//Protected functions
	protected:
											Sound(Sound &_ref);
		//Protected variables
	protected:
		class AudioEngine*					m_AudioEngine;
		SoundType							m_Type;
		rString								m_Name;
		unsigned int						m_NumInstaces;

	};
}