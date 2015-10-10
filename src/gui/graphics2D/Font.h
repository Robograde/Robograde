/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once

#include <FreeType/ft2build.h>
#include FT_FREETYPE_H
#include <FreeType/ftglyph.h>
#include <FreeType/ftoutln.h>
#include <FreeType/fttrigon.h>
#include <memory/Alloc.h>
#include <gfx/Shader/ShaderProgram.h>
#include "FontInfo.h"
namespace GUI 
{
	struct CharacterInfo 
	{
		int		ax; // advance.x
		int		ay; // advance.y

		int		bw; // bitmap.width;
		int		bh; // bitmap.rows;

		int		bl; // bitmap_left;
		int		bt; // bitmap_top;

		float 	tx; // x offset of glyph in texture coordinates
	};

	class Font 
	{
	public:
						~Font( );
		void			Load( const rString& fontName, GLuint fontHeight );
		void			ReSize( GLuint fontHeight );

		glm::vec2		Draw( const char * text, glm::vec4 color, float x, float y, float wWidth, float wHeight );

		GLuint			GetFontHeight( ) const	{ return m_FontHeight; }

		float			GetAtlasWidth( ) const 	{ return m_AtlasWidth; }
		float			GetAtlasHeight( ) const	{ return m_AtlasHeight; }
		GLuint			GetAtlasTexture( ) const{ return m_AtlasTexture; }
		CharacterInfo*	GetCharInfo( ) 			{ return m_CharInfo; }
		
		rString			GetIdent( ) const		{ return m_FontIdent; }
		
		void			SetIdent ( rString fontIdent ) { m_FontIdent = fontIdent.c_str(); }
		
		
		GLuint			GetCharInfoBuffer( ) const { return m_CharInfoBuffer; }
	private:
		rString			m_FontName;
		GLuint			m_FontHeight;
		GLuint			m_AtlasTexture;
		float			m_AtlasWidth;
		float			m_AtlasHeight;

		CharacterInfo 	m_CharInfo[128];
		
		GLuint			m_CharInfoBuffer;
		
		rString			m_FontIdent;
		FONT_ID			m_FontID;
	};
}
