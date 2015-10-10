/**************************************************
Copyright 2015 Ola Enberg & Daniel Bengtsson
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <gui/graphics2D/SpriteDefinition.h>
#include <gfx/DebugRenderer.h>
#include <script/ScriptEngine.h>
#include "../../picking/Picking.h"
#include "../../datadriven/ComponentTypes.h"

#define PICKING_ENTITY_DEBUG	false

#define g_SSPicking SSPicking::GetInstance()		// Conveniency access function

class SSPicking : public Subsystem
{
public:
	static SSPicking&		GetInstance					();

	void					Startup						() override;
	void					UpdateUserLayer				( const float deltaTime ) override;
	void					Shutdown					() override;
	
	bool					IsBoxSelecting				() const;

	void					SetShowTerrainPicking		( bool showPicking );
	
	glm::ivec2				WorldToScreen				( const glm::vec3& worldPos );
private:
							// No external instancing allowed
							SSPicking					() : Subsystem( "Picking" ) {}
							SSPicking					( const SSPicking & rhs );
							~SSPicking					() {};
	SSPicking&				operator=					(const SSPicking & rhs);

	int						TranslateToMissionType		( PickingAction pickingAction );
	void					PrepairSelectionRender		();
	void					RenderBoxSelection			();
	void					RenderClickedTerrainPos		();

	int						ScriptSetShowTerrainPicking	( IScriptEngine* scriptEngine );

	Picking					m_Picking;
	GUI::SpriteDefinition	m_BoxSprite;
	rVector<Entity>			m_MouseOveredEntities;
	bool					m_ShowTerrainPicking;
	gfx::DebugRenderer		m_DebugRenderer;
};