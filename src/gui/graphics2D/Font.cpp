/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Font.h"
#include <utility/Logger.h>
namespace GUI 
{
		
	Font::~Font( )
	{
		glDeleteBuffers( 1, &m_CharInfoBuffer );
	}

	void Font::Load( const rString& fontName, GLuint fontHeight )
	{		
		m_FontName = fontName.c_str();
		m_FontHeight = fontHeight;

		FT_Library library;
		if ( FT_Init_FreeType( &library ) ) 
		{
			Logger::Log( "FT_Init_FreeType failed!", "Graphics2D", LogSeverity::ERROR_MSG );
			return;
		}

		FT_Face face;


		if ( FT_New_Face( library, fontName.c_str(), 0, &face ) ) 
		{
			Logger::Log("FT_New_Face failed (possible problem with font file)", "Graphics2D", LogSeverity::ERROR_MSG);
			return;
		}

		FT_Set_Char_Size( face, fontHeight << 6, fontHeight << 6, 96, 96 );

		FT_GlyphSlot g = face->glyph;

		int w = 0, h = 0;

		for ( int i = 32; i < 128; i++ ) 
		{
			if ( FT_Load_Char(face, i, FT_LOAD_RENDER))  
			{
				fOStringStream ss;
				ss << i;
				Logger::Log( ("Loading character " + ss.str() + " failed!").c_str(), "Graphics2D", LogSeverity::ERROR_MSG );
				continue;
			}

			w += g->bitmap.width;
			h = glm::max<int>(h, g->bitmap.rows);

		}

		m_AtlasWidth = (float)w;
		m_AtlasHeight = (float)h;

		//Setting up texture atlas
		glActiveTexture( GL_TEXTURE0 );
		glGenTextures( 1, &m_AtlasTexture );
		glBindTexture( GL_TEXTURE_2D, m_AtlasTexture );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0 );

		int x = 0;

		for ( int i = 32; i < 128; i++ ) 
		{
			if ( FT_Load_Char( face, i, FT_LOAD_RENDER ) ) 
			{
				continue;
			}

			glTexSubImage2D( GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer );
			
			m_CharInfo[i].ax = g->advance.x >> 6;
			m_CharInfo[i].ay = g->advance.y >> 6;
			m_CharInfo[i].bw = g->bitmap.width;
			m_CharInfo[i].bh = g->bitmap.rows;
			m_CharInfo[i].bl = g->bitmap_left;
			m_CharInfo[i].bt = g->bitmap_top;
			m_CharInfo[i].tx = (float)x / w;

			x += g->bitmap.width;
		}
		
		glGenBuffers( 1, &m_CharInfoBuffer );
		glBindBuffer( GL_UNIFORM_BUFFER, m_CharInfoBuffer );
		glBufferData( GL_UNIFORM_BUFFER, 128 * sizeof( CharacterInfo ), m_CharInfo, GL_DYNAMIC_DRAW );
		
		rStringStream ss;
		ss << "Loaded font: " << fontName << " Size: " << fontHeight;
		Logger::Log( ss.str() , "Graphics2D", LogSeverity::INFO_MSG);
	}

	void Font::ReSize( GLuint fontHeight ) 
	{
		Load( m_FontName.c_str(), fontHeight);
	}
}
