/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once

#include <gui/GUIEngine.h>
#include "../Subsystem.h"
#include "../gamelogic/SSCamera.h"

#define CURSOR_SPRITE_SIZE	48

#define g_SSCursor SSCursor::GetInstance()

class SSCursor : public Subsystem
{
public:
	static SSCursor& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSCursor() : Subsystem( "Cursor" ) {}
	SSCursor( const SSCursor& rhs );
	~SSCursor() {};
	SSCursor& operator=( const SSCursor& rhs );
	
	
	GUI::Sprite*			m_MouseCursor;
	GUI::SpriteDefinition	m_StandardDef;
	GUI::SpriteDefinition	m_AttackDef;
	GUI::SpriteDefinition	m_PingDef;
	GUI::SpriteDefinition	m_PanDefs[ CAMERA_PAN_DIRECTION_SIZE ];
	
	const glm::vec4			m_AttackColour	= glm::vec4( 0.7f, 0.1f, 0.1f, 1.0f );
	const glm::vec4			m_PingColour	= glm::vec4( 0.1f, 0.7f, 0.1f, 1.0f );
};