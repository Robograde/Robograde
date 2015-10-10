/**************************************************
2015 Mattias Wilelmsen
***************************************************/

#pragma once 

#include "gfx/AnimationStructs.h"
#include "gfx/Animation.h"

struct AnimationComponent
{
	rMap<gfx::AnimationType, gfx::AnimationHandle> HandleMap;

	rVector<glm::mat4>		MatrixList;
	//glm::mat4				MatrixArray[gfx::MAX_SKELETON_JOINTS];
	gfx::AnimationHandle	AnimationHandle = -1;
	gfx::AnimationType		CurrentAnimation	= gfx::AnimationType::Undefined;
	gfx::Animation*			AnimationRef	= nullptr;
	float					CurrentTime		= 0.0f;
	bool					Playing			= true;
	float					SecPerFrame		= 0.1f;
	float					FramesPerSec = 0;

	~AnimationComponent( )
	{
		MatrixList.clear( );
	}

	void Pause() { Playing = false; }
	void Play()  { Playing = true; }
	void Stop()	
	{ 
		Playing = false; 
		CurrentTime = 0.0f;
	}
	void StartAnimation( gfx::AnimationType type )
	{
		if (CurrentAnimation != type)
		{
			CurrentAnimation = type;
			CurrentTime = 0.0f;
			Playing = true;
		}
	}
};