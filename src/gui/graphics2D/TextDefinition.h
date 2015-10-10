/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "RenderDefinition.h"
#include <memory/Alloc.h>
#include <glm/glm.hpp>
#include "FontInfo.h"

namespace GUI
{	
	enum LINESPACING
	{
		LINESPACING_NONE,
		LINESPACING_QUARTER,
		LINESPACING_HALF
	};
	
	struct TextDefinition : public RenderDefinition
	{
		TextDefinition() : TextDefinition( "", 0, 0 )
		{ }
	
		TextDefinition( const char* text, int posX, int posY, int width, int height, glm::vec4 colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) ) : RenderDefinition( posX, posY, RenderType::Text, colour )
		{
			Text = text;
			BoundsSize = glm::ivec2( width, height );
		}
		
		TextDefinition( const char* text, int posX, int posY, glm::vec4 colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) ) : RenderDefinition( posX, posY, RenderType::Text, colour )
		{
			Text = text;
		}
		
	
		rString					Text;
		FONT_ID					FontID = FONT_ID_DEFAULT_12;
		LINESPACING				LineSpacing = LINESPACING_QUARTER;

		float					Opacity = 1.0f;
		
		bool CenterX = false;
		bool CenterY = false;
		
		glm::ivec2 BoundsSize = glm::ivec2( -1, -1 );
	};
}
