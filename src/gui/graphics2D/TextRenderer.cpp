/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "TextRenderer.h"
#include <GL/glew.h>
#include <profiler/AutoGPUProfiler.h>
#include <stdio.h>
#include <utility/Logger.h>

using glm::vec4;

namespace GUI 
{
	TextRenderer::TextRenderer()
	{ 		
	}

	TextRenderer::~TextRenderer()
	{ 
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteVertexArrays(1, &m_VBO);

		for ( auto& font : m_Fonts ) 
		{
			pDelete( font.second );
		}
	}

	void TextRenderer::Initialize()
	{
		glGenBuffers( 1, &m_VBO );
		glGenVertexArrays( 1, &m_VAO );

		glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
		glBufferData( GL_ARRAY_BUFFER, MAX_TEXT_VERTICES * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW );

		glBindVertexArray( m_VAO );
		
		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		glEnableVertexAttribArray( 2 );
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof( GLfloat ), 0 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE,  13 * sizeof( GLfloat ), ( void* )( 3 * sizeof(GLfloat) ) );
		glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE,  13 * sizeof( GLfloat ), ( void* )( 5 * sizeof(GLfloat) ) );
		glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE,  13 * sizeof( GLfloat ), ( void* )( 9 * sizeof(GLfloat) ) );

		m_TextQueue.reserve( 50 );

		m_TempText.reserve( 50 );
		m_TempColourText.reserve( 50 );

		m_Lines.reserve( 20 );
		m_Lines2.reserve( 20 );

		m_Fonts[FONT_ID_CONSOLE_11] = LoadFont( "SourceCodePro-Regular.otf", 11 );
		m_Fonts[FONT_ID_CONSOLE_14] = LoadFont( "SourceCodePro-Regular.otf", 14 );
		m_Fonts[FONT_ID_DEFAULT_12] = LoadFont( "SourceCodePro-Regular.otf", 12 );
		m_Fonts[FONT_ID_LEKTON_8] = LoadFont( "Lekton-Bold.ttf", 8 );
		m_Fonts[FONT_ID_LEKTON_11] = LoadFont( "Lekton-Bold.ttf", 11 );
		m_Fonts[FONT_ID_LEKTON_14] = LoadFont( "Lekton-Bold.ttf", 14 );
		m_Fonts[FONT_ID_LEKTON_16] = LoadFont( "Lekton-Bold.ttf", 16 );
		m_Fonts[FONT_ID_LEKTON_20] = LoadFont( "Lekton-Bold.ttf", 20 );
	}

	void TextRenderer::EnqueueText(TextDefinition* textDefinition)
	{
		m_TextQueue.push_back( textDefinition );
	}

	void SplitIntoLines( pVector<rString> &result, const rString &s )
	{
		rStringStream ss( s.c_str() );
		rString line;
		while( std::getline( ss, line ) )
		{
			result.push_back( line );
		}
	}

	int GetNumLines( const rString &s )
	{
		int num = 0;
		rStringStream ss( s.c_str() );
		rString line;
		while( std::getline( ss, line ) )
		{
			num++;
		}
		return num;
	}

	void TextRenderer::Render( int windowWidth, int windowHeight, gfx::ShaderProgram* shaderProg )
	{
		if( static_cast<int>( m_TextQueue.size() ) == 0 )
			return;

		shaderProg->Apply();

		//Render all text that use the same font
		for( auto& fontIt : m_Fonts )
		{
			int n = 0;
			int pointCount = 0;
			Font* font = fontIt.second;

			for( auto& textDefIt : m_TextQueue )
			{
				TextDefinition& textDef = *textDefIt;


				if( textDef.FontID == fontIt.first )
				{
					vec4 colour = textDef.Colour;
				
					int lineSpacing = 0;
					if( textDef.LineSpacing == LINESPACING_QUARTER )
						lineSpacing = static_cast<int>( font->GetAtlasHeight() / 4 );
					else if ( textDef.LineSpacing == LINESPACING_HALF )
						lineSpacing = static_cast<int>( font->GetAtlasHeight() / 2 );
				

					int totalHeight = 0;

					bool centerX =		textDef.Alignment == ALIGNMENT_TOP_CENTER	|| textDef.Alignment == ALIGNMENT_MIDDLE_CENTER || textDef.Alignment == ALIGNMENT_BOTTOM_CENTER;
					bool alignLeft =	textDef.Alignment == ALIGNMENT_TOP_LEFT		|| textDef.Alignment == ALIGNMENT_MIDDLE_LEFT	|| textDef.Alignment == ALIGNMENT_BOTTOM_LEFT;
					bool alignRight =	textDef.Alignment == ALIGNMENT_TOP_RIGHT	|| textDef.Alignment == ALIGNMENT_MIDDLE_RIGHT	|| textDef.Alignment == ALIGNMENT_BOTTOM_RIGHT;
					bool centerY =		textDef.Alignment == ALIGNMENT_MIDDLE_LEFT	|| textDef.Alignment == ALIGNMENT_MIDDLE_CENTER || textDef.Alignment == ALIGNMENT_MIDDLE_RIGHT;
					bool alignTop =		textDef.Alignment == ALIGNMENT_TOP_LEFT		|| textDef.Alignment == ALIGNMENT_TOP_CENTER	|| textDef.Alignment == ALIGNMENT_TOP_RIGHT;
					bool alignBottom =	textDef.Alignment == ALIGNMENT_BOTTOM_LEFT  || textDef.Alignment == ALIGNMENT_BOTTOM_CENTER || textDef.Alignment == ALIGNMENT_BOTTOM_RIGHT;
					//Handling line breaks

					m_Lines.clear();
					//std::deque <rString> lines; 
					SplitIntoLines( m_Lines, textDef.Text );
					int numLines =  static_cast<int>( m_Lines.size() );
					glm::ivec2 position = textDef.Position;

					totalHeight = numLines * (font->GetFontHeight() + lineSpacing) - lineSpacing*2;

					if( numLines > 1 )
						int brdasdfa = 0;

					int lineID = 0;

					for( rString& text : m_Lines )
					{
						
						int charCount = 0;
						glm::ivec2 linePosition;
						m_Colours.clear();
						ParseText( text, &textDef, colour );


						int len = static_cast<int>(strlen( text.c_str() ));
						glm::ivec2 textSize = GetTextSize( text, textDef.FontID );

						//totalHeight += textSize.y;

						if( len > 0 )
						{
							int offsetX = 0;
							int offsetY = 0;

							if( centerX )
								offsetX = static_cast<int>( -textSize.x / 2.0f );
							else if( alignRight )
								offsetX = static_cast<int>( -textSize.x );

							if( numLines  <= 1 )
							{


								if( centerY )
									offsetY = static_cast<int>( textSize.y / 2.0f );

							}
							else
							{
								/*if( centerX  || alignRight )
									offsetX = m_BreakXOffsets[0];*/

								if( centerY )
									offsetY = static_cast<int>( -totalHeight / 2 ) + (textSize.y / 2);

							}
						
							if( alignLeft )
							{
								linePosition.x = position.x;
							}
							else if( centerX )
								linePosition.x = position.x + ( textDef.BoundsSize.x / 2 );
							else if( alignRight )
								linePosition.x = position.x + textDef.BoundsSize.x;

							if( alignTop )
								linePosition.y = position.y + font->GetFontHeight();
							else if( centerY )
								linePosition.y = position.y + ( textDef.BoundsSize.y / 2 );
							else if( alignBottom )
								linePosition.y = position.y + textDef.BoundsSize.y;

							//Calculate screenposition in 0-1 range
							float xPos = (textDef.Origin.x + linePosition.x + offsetX) / (float)windowWidth;
							float yPos = (textDef.Origin.y + linePosition.y + offsetY) / (float)windowHeight;

							float sx = 2.0f / windowWidth;
							float sy = 2.0f / windowHeight;

							float x = (((xPos)* 2) - 1);
							float y = -(((yPos)* 2) - 1);

							CharacterInfo* c = font->GetCharInfo();

							for( const char *p = text.c_str(); *p; p++ )
							{
								int charInt = static_cast<int>( *p ) ;
								if( charInt >= 32 && charInt <= 128 )
								{
									if (  m_Colours.find( charCount ) !=  m_Colours.end() )
										colour = m_Colours.at( charCount );

									float w = c[*p].bw * sx;
									float h = c[*p].bh * sy;

									charCount++;

									float x2 = x + c[*p].bl * sx;
									float y2 = -y - c[*p].bt * sy;

									// Advance the cursor to the start of the next character
									x += c[*p].ax * sx;
									y += c[*p].ay * sy;

									if ( !w || !h )
										continue;

									//Position
									m_Vertices[n++] = x2;
									m_Vertices[n++] = -y2;
									m_Vertices[n++] = textDef.Depth;
									//Size
									m_Vertices[n++] = w;
									m_Vertices[n++] = h;
									//Top-left texture coords
									m_Vertices[n++] = c[*p].tx;
									m_Vertices[n++] = 0.0f;
									//Bottom-right texture coords
									m_Vertices[n++] = c[*p].tx + c[*p].bw / font->GetAtlasWidth();
									m_Vertices[n++] = c[*p].bh / font->GetAtlasHeight();
									//Colour
									m_Vertices[n++] = colour.r;
									m_Vertices[n++] = colour.g;
									m_Vertices[n++] = colour.b;
									m_Vertices[n++] = colour.a * textDef.Opacity;

									pointCount++;
								}
							}
						}
						lineID++;
						position.y += lineSpacing + textSize.y;
					}
				}
			}


			glBindBuffer( GL_ARRAY_BUFFER, m_VBO );

			glBufferSubData( GL_ARRAY_BUFFER, 0, n * sizeof ( GLfloat ), m_Vertices );

			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, font->GetAtlasTexture() );

			glBindVertexArray( m_VAO );

			{
				GPU_PROFILE( AutoGPUProfiler gpuProfile("GUIRenderText", true) );
				glDrawArrays( GL_POINTS, 0, pointCount );
				GPU_PROFILE( gpuProfile.Stop(); );
			}

			pointCount = 0;
		}

		glBindVertexArray( 0 );
		glUseProgram( 0 );

		m_TextQueue.clear();
	}

	glm::ivec2 TextRenderer::GetTextSize( const rString& text, FONT_ID fontID )
	{
		int width = 0;
		int height = 0;

		auto fontIt = m_Fonts.find( fontID );
		Font* font = fontIt->second;
		if( fontIt == m_Fonts.end() )
		{
			//The font was not found, wtf
			rStringStream ss;
			ss << "The text size of '" << text << "' could not be calculated because the font was not found.";
			Logger::Log( ss.str(), "Graphics2D", LogSeverity::WARNING_MSG );
			return glm::ivec2( 0, 0 );
		}

		//Handle linebreaks
		m_Lines2.clear();
		SplitIntoLines( m_Lines2, text );

		int numLines =  static_cast<int>( m_Lines2.size() );
		if( numLines > 1 )
		{
			int lineSpacing = static_cast<int>( font->GetAtlasHeight() / 4 );
			height = numLines * (font->GetFontHeight() + lineSpacing) - lineSpacing;
		}

		
		CharacterInfo* c = fontIt->second->GetCharInfo();

		for( rString& lineText : m_Lines2 )
		{
			int tWidth = 0;
			int ax = 0;
			for( const char *p = lineText.c_str(); *p; p++ )
			{
				tWidth = c[*p].bl + ax + c[*p].bw;
				ax += c[*p].ax;

				//if( c[*p].bh > height )
				//	height = c[*p].bh;
			}
			if( tWidth > width )
				width = tWidth;
		}

		if( height == 0 )
			height = font->GetFontHeight(); //TODIA: Figure out if I can get away with this.
		
		return glm::ivec2( width, height );
	}

	void TextRenderer::ParseText( rString& text, TextDefinition* textDefinition, glm::vec4& colourRef  )
	{

		m_TempColourText =  "DEFAULT";
		rString& colour = m_TempColourText;

		int pos1 = static_cast<int>( text.find( '[' ) );
		int pos2 = static_cast<int>( text.find( ']' ) );

		//Colours
		while ( pos1 != rString::npos && pos2 != rString::npos )
		{
			if( text[pos1 + 1] == 'C' )
			{
				colour = text.substr( pos1 + 3, pos2 - (pos1+3) );
				if( pos1 == 0 )
				{
					m_Colours[0] = GetColour( colour );
					text = text.substr( pos2 + 1, static_cast<int>( text.size() ) );
				}
				else
				{
					m_Colours[pos1] = GetColour( colour );
					m_TempText = text.substr( 0, pos1 );
					text = m_TempText + text.substr( pos2 + 1, static_cast<int>( text.size() ) );
				}
			}
			else
				break; //When more formatting is added, keep this at the end to avoid getting stuck

			//colourRef = GetColour( colour );

			pos1 = static_cast<int>( text.find( '[' ) );
			pos2 = static_cast<int>( text.find( ']' ) );
		}
	}

	glm::vec4 TextRenderer::GetColour( const rString& colour )
	{
		if( colour == "RED" )
			return vec4( 0.8f, 0.0f, 0.0f, 1.0f );
		else if( colour == "GREEN" )
			return vec4( 0.0f, 0.8f, 0.0f, 1.0f );
		else if( colour == "BLUE" )
			return vec4( 0.0f, 0.5f, 0.8f, 1.0f );
		else if( colour == "YELLOW" )
			return vec4( 0.8f, 0.8f, 0.0f, 1.0f );
		else if( colour == "PURPLE" )
			return vec4( 0.8f, 0.0f, 0.8f, 1.0f );
		else if( colour == "WHITE" )
			return vec4( 1.0f, 1.0f, 1.0f, 1.0f );
		else if( colour == "BLACK" )
			return vec4( 0.0f, 0.0f, 0.0f, 1.0f );
		else if( colour == "GREY" )
			return vec4( 0.6f, 0.6f, 0.6f, 1.0f );
		else
			return vec4( 1.0f );
	}

	Font* GUI::TextRenderer::LoadFont( const rString& fontName, unsigned int fontSize )
	{
		Font* font = pNew( Font );
		rString fontPath = "../../../asset/font/" + fontName;

		font->Load( fontPath, fontSize );
		return font;
	}

}


