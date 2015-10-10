/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include <glm/glm.hpp>

namespace GUI
{
	struct Rectangle
	{
		int X;
		int Y;
		int Width;
		int Height;
		
		glm::ivec2 Origin = glm::ivec2( 0 );
		
		Rectangle()
		{
			X = 0;
			Y = 0;
			Width = -1;
			Height = -1;
		}
		
		Rectangle( int x , int y, int width, int height )
		{
			X = x;
			Y = y;
			Width = width;
			Height = height;
		}
		
		int GetTop() const { return Origin.y + Y; }
		int GetRight() const { return Origin.x + X + Width; }
		int GetBottom() const { return Origin.y + Y + Height; }
		int GetLeft() const { return Origin.x + X; }
		
		glm::ivec2 GetPosition() const { return glm::ivec2( X, Y ); }
		glm::ivec2 GetSize() const { return glm::ivec2( Width, Height ); }
		
		bool Intersects( int x, int y ) const
		{
			if( x >= GetLeft() && x < GetRight() )
			{
				if( y >= GetTop() && y < GetBottom() )
					return true;
			}
			return false;
		}
		
		bool Intersects( const Rectangle& other ) const
		{
			if( GetLeft() < other.GetRight() && 
					GetRight() > other.GetLeft() && 
					GetTop() < other.GetBottom() &&
					GetBottom() > other.GetTop() )
			{
				return true;
			}
			return false;
		}

		bool Contains( const Rectangle& other ) const
		{
			if( GetLeft() < other.GetLeft() &&
					GetRight() > other.GetRight() &&
					GetTop() < other.GetTop() &&
					GetBottom() > other.GetBottom() )
			{
				return true;
			}
			return false;
		}
	};
}