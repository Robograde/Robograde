/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <picojson/picojson.h>
#include "Texture.h"
#include "GFXLibraryDefine.h"
namespace gfx
{
	struct TextureFrame
	{
		int X;
		int Y;
		int Width;
		int Height;
		rString	Filename;
	};

	struct GPUTexture
	{
		float S;
		float T;
		float NormWidth;
		float NormHeight;
	};

	class TextureAtlas
	{
	public: 
		GFX_API TextureAtlas();
		GFX_API ~TextureAtlas();
		GFX_API bool LoadFile(const rString& filename);
		GFX_API gfx::Texture* GetTexture();
		GFX_API int GetHandle(const rString& name);
		GFX_API rVector<GPUTexture>& GetGPUTextures();
		GFX_API void Clear();
	private:
		 rString GetDir(rString str);

		rMap<rString, int>			m_Handles;
		rVector<TextureFrame>		m_Frames;
		rVector<GPUTexture>			m_GPUFrames;
		rString						m_Filename;
		rString						m_ImageFilename;
		int							m_Width;
		int							m_Height;
		Texture*					m_Texture;

	};
}

