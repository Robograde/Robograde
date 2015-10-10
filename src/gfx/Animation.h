/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "AnimationStructs.h"


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
	class Animation
	{

	public:
	GFX_API	Animation();
	GFX_API	~Animation();
	GFX_API rVector<SkeletonFrame>& GetEditableSkeletonList();
	GFX_API const rVector<glm::mat4>& GetBindPoseRef() const;
	GFX_API void CreateBindPoseMatrices();
	GFX_API rString GetName() const;
	GFX_API void SetName( rString name );
	GFX_API void SetDefaultFPS(const float fps);
	GFX_API void GetMatricesAtTime( rVector<glm::mat4>& matVec, float time, float secPerFrame = 0.0f );
	GFX_API void SetBindpose( SkeletonFrame& frame );

	private:
		rVector<SkeletonFrame> m_SkeletonFrames;
		rVector<glm::mat4> m_BindPoseMatrices;
		float m_FramesPerSec = 1.0f;
		rString m_Name = "";
	};
}
