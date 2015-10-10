/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "RenderDefinition.h"
#include "Rectangle.h"

namespace GUI
{
	struct SpriteDefinition : public RenderDefinition
	{
		SpriteDefinition( ) : SpriteDefinition( "", 0, 0, 1, 1 )
		{
			
		}
		
		SpriteDefinition( const rString& texture, int posX, int posY, int width, int height, glm::vec4 colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) ) : 
			RenderDefinition( posX, posY, RenderType::Sprite, colour )
		{
			Texture  = texture;
			Width = width;
			Height = height;
		}
		
		void SetBounds( Rectangle& boundingBoxRef )
		{
			Position = glm::ivec2( boundingBoxRef.X, boundingBoxRef.Y );
			Width = boundingBoxRef.Width;
			Height = boundingBoxRef.Height;
			Origin = boundingBoxRef.Origin;
		}
		
		rString Texture;
		int TextureHandle = -1;
		int TextureHandle2 = -1;
		int Width;
		int Height;

		int BorderSize = 0;
		glm::vec4 BorderColour =	glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
		
		glm::vec2 TopLeftUV =		glm::vec2( 0.0f );
		glm::vec2 BottomRightUV =	glm::vec2( 1.0f );
		
		bool CenterX = false;
		bool CenterY = false;
	};
}
