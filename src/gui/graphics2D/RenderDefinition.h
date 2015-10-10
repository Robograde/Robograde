/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <glm/glm.hpp>

namespace GUI
{

	enum class RenderType
	{
		None, Text, Sprite, Polygon, Scissor
	};
	
	enum ALIGNMENT
	{
		ALIGNMENT_TOP_LEFT,
		ALIGNMENT_TOP_CENTER,
		ALIGNMENT_TOP_RIGHT,
		ALIGNMENT_MIDDLE_LEFT,
		ALIGNMENT_MIDDLE_CENTER,
		ALIGNMENT_MIDDLE_RIGHT,
		ALIGNMENT_BOTTOM_LEFT,
		ALIGNMENT_BOTTOM_CENTER,
		ALIGNMENT_BOTTOM_RIGHT,
	};
	
	struct RenderDefinition
	{
		RenderDefinition() : RenderDefinition( 0, 0, RenderType::None )
		{
			
		}
		
		RenderDefinition( int posX, int posY, RenderType renderType, glm::vec4 colour = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) )
		{
			Position = glm::ivec2( posX, posY );
			MyRenderType = renderType;
			Colour = colour;
		}

		glm::ivec2		Position;
		float			Depth;
		glm::ivec2		Origin = glm::ivec2( 0, 0 );
		glm::vec4		Colour;
		RenderType		MyRenderType;
		ALIGNMENT		Alignment = ALIGNMENT_TOP_LEFT;
		
	};
}
