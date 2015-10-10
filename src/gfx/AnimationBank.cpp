/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#include "AnimationBank.h"
#include <utility/Logger.h>
gfx::AnimationBank::AnimationBank()
{

}

gfx::AnimationBank::~AnimationBank()
{

}

gfx::AnimationBank& gfx::AnimationBank::GetInstance()
{
	static AnimationBank instance;
	return instance;
}

gfx::AnimationHandle gfx::AnimationBank::LoadAnimation( const char* filename )
{
	for ( auto& it : m_Animations )
	{
		if ( it.second.GetName() == filename )
			return it.first;
	}

	Animation animation;
	AnimationHandle id = ++m_Numerator;
	if ( m_Importer.LoadAnimation(filename, animation ))
	{
		animation.SetName( filename );
		m_Animations[id] = animation;
		return id;
	}
	else
	{
		return -1;
	}
	m_Importer.Release();
}

gfx::AnimationHandle gfx::AnimationBank::AddAnimation( Animation& animation )
{
	AnimationHandle id = ++m_Numerator;
	m_Animations[id] = animation;
	printf("\n\n Number of animations %d \n\n", m_Numerator);
	return id;
}

gfx::Animation& gfx::AnimationBank::FetchAnimationWithHandle( const AnimationHandle handle )
{
	return m_Animations[handle];
}

void gfx::AnimationBank::UpdateAnimation( AnimationHandle& handle, Animation& animation )
{
	m_Animations[handle] = animation;
}
