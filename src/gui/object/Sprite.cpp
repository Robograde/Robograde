/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Sprite.h"
#include "../GUIEngine.h"

GUI::Sprite::Sprite()
{
}

GUI::Sprite::Sprite( rString name, GUI::SpriteDefinition spriteDefinition, rString parent ) :
Object( name, parent, Rectangle( spriteDefinition.Position.x, spriteDefinition.Position.y, spriteDefinition.Width, spriteDefinition.Height ) )
{
	m_SpriteDefinition = spriteDefinition;
}

GUI::Sprite::~Sprite()
{
}

void GUI::Sprite::Initialize()
{
}

void GUI::Sprite::Update(float deltaTime , glm::ivec2 parentPos)
{
}

void GUI::Sprite::Render( glm::ivec2 parentPos )
{
	if( m_Visible )
	{
		m_SpriteDefinition.Origin = parentPos;
		m_BoundingBox.Origin = parentPos;
		g_GUI.EnqueueSprite( &m_SpriteDefinition );
	}
}

GUI::SpriteDefinition& GUI::Sprite::GetSpriteDefinitionRef()
{
	return m_SpriteDefinition;
}

void GUI::Sprite::SetTexture( rString texturePath )
{
	m_SpriteDefinition.Texture = texturePath.c_str();
}

void GUI::Sprite::SetPosition( int x, int y )
{
	m_SpriteDefinition.Position = glm::ivec2( x, y );
}

glm::ivec2 GUI::Sprite::GetPosition( )
{
	return m_SpriteDefinition.Position;
}