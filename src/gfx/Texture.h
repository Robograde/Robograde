/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <soil2/SOIL2.h>
#include "GL/glew.h"
#include <memory/Alloc.h>
#include <stdio.h>
#include "GL/glew.h"
#include "GFXLibraryDefine.h"

typedef int TextureHandle;
namespace gfx
{
	enum TextureType
	{
		TEXTURE_2D,
		CUBE_TEXTURE
	};

	class Texture
	{
	public:
	GFX_API	Texture();
	GFX_API	~Texture();

	GFX_API bool Init( const char* Filename, TextureType Type );
	GFX_API bool Init( char* data, int channels, int width, int height, TextureType Type );
	GFX_API bool InitLinear( const char* filename, int width, int height );
	GFX_API GLuint GetHandle();
	GFX_API void Apply( GLuint location, int index );
	GFX_API TextureType GetType();
	GFX_API bool GetLoaded();
	GFX_API void SetFilename( const char* filename );
	GFX_API rString GetFilename();

	private:
		GLuint m_Handle;
		int m_Width, m_Height;
		TextureType m_Type;
		bool m_Loaded;
		rString m_Filename;
	};
}
