/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Font.h"
#include "TextDefinition.h"
#include "FontInfo.h"

namespace GUI 
{
	#define MAX_TEXT_VERTICES 70000
		
	struct Point 
	{
		Point() 
		{ }
		Point( GLfloat x, GLfloat y, GLfloat s, GLfloat t ) 
		{
			this->x = x; this->y = y; this->s = s; this->t = t;
		}
		GLfloat x; GLfloat y; GLfloat s; GLfloat t;
	};
	
	class TextRenderer
	{
	public:
						TextRenderer();
						~TextRenderer();
						
		void			Initialize( );
						
		void 			Render( int windowWidth, int windowHeight, gfx::ShaderProgram* shaderProg );
			 			
		void			EnqueueText( TextDefinition* textDefinition  );
		
		//rString		GetCurrentFontIdent( ) { return m_CurrentFontIdent; }
		
		glm::ivec2		GetTextSize( const rString& text, FONT_ID fontID );
		
	private:
		Font* LoadFont( const rString& fontName, unsigned int fontSize );
		
		void			ParseText( rString& text, TextDefinition* textDefinition, glm::vec4& colourRe );
		glm::vec4		GetColour( const rString& colour );

		rMap<FONT_ID, Font*> m_Fonts;
		
		GLuint			m_VBO;
		GLuint			m_VAO;
		
		GLfloat			m_Vertices[MAX_TEXT_VERTICES];
		
		pVector<TextDefinition*>	m_TextQueue;
		
		pVector<rString>		m_Lines;
		rMap<int, glm::vec4>	m_Colours;

		rString			m_TempText;
		rString			m_TempText2;
		rString			m_TempColourText;
		
		
	};
}
