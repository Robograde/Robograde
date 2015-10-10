/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSCursor.h"
#include <input/Input.h>
#include <SDL2/SDL.h>
#include <utility/ConfigManager.h>
#include "../../utility/PlayerData.h"

SSCursor& SSCursor::GetInstance()
{
	static SSCursor instance;
	return instance;
}

void SSCursor::Startup()
{
	// Set the mouse cursor
	glm::ivec2 winSize = g_GUI.GetWindowSize( "RootWindow" );
	g_GUI.AddWindow( "CursorWindow", GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "RootWindow" );
	g_GUI.OpenWindow( "CursorWindow" );

	m_StandardDef	= GUI::SpriteDefinition( "cursor/CursorBlank.png", 0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_MoveDef		= GUI::SpriteDefinition( "cursor/CursorMove.png", 0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_AttackDef		= GUI::SpriteDefinition( "cursor/CursorBlank.png", 0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE, m_AttackColour );
	m_PingDef		= GUI::SpriteDefinition( "cursor/CursorBlank.png", 0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE, m_PingColour );

	m_PanDefs[ CAMERA_PAN_DIRECTION_UP			] = GUI::SpriteDefinition( "cursor/Cursor_Up.png",			0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_DOWN		] = GUI::SpriteDefinition( "cursor/Cursor_Down.png",		0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_LEFT		] = GUI::SpriteDefinition( "cursor/Cursor_Left.png",		0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_RIGHT		] = GUI::SpriteDefinition( "cursor/Cursor_Right.png",		0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_UP_LEFT		] = GUI::SpriteDefinition( "cursor/Cursor_UpLeft.png",		0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_UP_RIGHT	] = GUI::SpriteDefinition( "cursor/Cursor_UpRight.png",		0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_DOWN_LEFT	] = GUI::SpriteDefinition( "cursor/Cursor_DownLeft.png",	0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );
	m_PanDefs[ CAMERA_PAN_DIRECTION_DOWN_RIGHT	] = GUI::SpriteDefinition( "cursor/Cursor_DownRight.png",	0, 0, CURSOR_SPRITE_SIZE, CURSOR_SPRITE_SIZE );

	m_MouseCursor = g_GUI.AddSprite( "MouseCursor", m_StandardDef, "CursorWindow" );

	SDL_ShowCursor( 0 );

	CallbackConfig* gfxConfig = g_ConfigManager.GetConfig( "graphics.cfg" );
	SDL_SetRelativeMouseMode( gfxConfig->GetBool( "lockmouse", true ) == true ? SDL_TRUE : SDL_FALSE );
}

void SSCursor::Shutdown()
{
	g_GUI.DeleteObject( "CursorWindow" );
}

void SSCursor::UpdateUserLayer( const float deltaTime )
{
	const int halfCursorSpriteSize = CURSOR_SPRITE_SIZE / 2;

	// Update mouse cursor position
	m_MouseCursor->GetSpriteDefinitionRef().Position = glm::vec2( g_Input->GetMousePosX() - halfCursorSpriteSize, g_Input->GetMousePosY() - halfCursorSpriteSize );
	m_MouseCursor->BringToFront();

	CAMERA_PAN_DIRECTION panDirection = g_SSCamera.GetRTSCamera()->GetPanDirection();

	if ( panDirection < CAMERA_PAN_DIRECTION_SIZE )
	{
		m_MouseCursor->GetSpriteDefinitionRef().Texture = m_PanDefs[panDirection].Texture;
		m_MouseCursor->GetSpriteDefinitionRef().Colour = m_PanDefs[panDirection].Colour;
	}
	else
	{
		if (g_PlayerData.GetLastActionPressed() == ACTION::ACTION_AI_ATTACK)
		{
			m_MouseCursor->GetSpriteDefinitionRef().Texture = m_AttackDef.Texture;
			m_MouseCursor->GetSpriteDefinitionRef().Colour = m_AttackDef.Colour;
		}
		else if ( g_PlayerData.GetLastActionPressed( ) == ACTION::ACTION_PING )
		{
			m_MouseCursor->GetSpriteDefinitionRef( ).Texture = m_PingDef.Texture;
			m_MouseCursor->GetSpriteDefinitionRef( ).Colour = m_PingDef.Colour;
		}
		else if ( g_PlayerData.GetLastActionPressed( ) == ACTION::ACTION_EDITOR_MOVE )
		{
			m_MouseCursor->GetSpriteDefinitionRef( ).Texture = m_MoveDef.Texture;
			m_MouseCursor->GetSpriteDefinitionRef( ).Colour = m_MoveDef.Colour;
		}
		else
		{
			m_MouseCursor->GetSpriteDefinitionRef().Texture = m_StandardDef.Texture;
			m_MouseCursor->GetSpriteDefinitionRef().Colour = m_StandardDef.Colour;
		}
	}
}
