/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#pragma once
#include "Animation.h"
#include "ObjectImporter.h"

#define g_AnimationBank AnimationBank::GetInstance()
#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

namespace gfx
{
	class AnimationBank{
	public:
		GFX_API ~AnimationBank();
		GFX_API static AnimationBank& GetInstance();
		GFX_API const Animation& FetchAnimationWithFilename( const char* filename ); // TODOMW : Implement
		GFX_API Animation& FetchAnimationWithHandle( const AnimationHandle handle );
		GFX_API AnimationHandle LoadAnimation( const char* filename );
		GFX_API AnimationHandle AddAnimation( Animation& animation );
		GFX_API void UpdateAnimation( AnimationHandle& handle, Animation& animation );
	private:
		AnimationBank();
		gfx::ObjectImporter m_Importer;
		rMap<AnimationHandle, Animation> m_Animations;
		int m_Numerator = 0; // TODOMW : Make sure this is needed
	};
}
