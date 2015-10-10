/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include "../gamelogic/SSAI.h" //TODIA change to squad

#define g_SSSquadFlag SSSquadFlag::GetInstance()

struct SquadFlag
{
	int					Squad;
	rString				Sprite;
	rString				HighLightSprite;
	glm::vec4			HighLightColour;
	GUI::Rectangle		BoundingBox;
	GUI::Sprite*		Background;
	GUI::Sprite*		Slots[4];
	GUI::ProgressBar*	UpgradeBar;

	GUI::Sprite*		ControlGroupsBackground;
	GUI::Text*			ControlGroups;
	bool				Selected = false;

	GUI::Sprite* SizeTextBackground;
	GUI::Text* SizeText;
};


class SSSquadFlag : public Subsystem
{
public:
	static SSSquadFlag& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSSquadFlag() : Subsystem( "SquadFlag" ) {}
	SSSquadFlag( const SSSquadFlag& rhs );
	~SSSquadFlag() {};
	SSSquadFlag& operator=( const SSSquadFlag& rhs );

	SquadFlag CreateFlag( const rString& sprite, glm::ivec2 size, const rString& windowName );

	void UpdateFlag( SquadFlag& flag, glm::ivec2 position, glm::ivec2 iconOffset, int, Squad* squad );

	void HandleClicks( Squad* squad, const glm::vec3& squadWorldPos, const SquadFlag& flag );
	
	rVector<SquadFlag> m_Flags;
	rVector<SquadFlag> m_UIFlags;
	
	
	
	const glm::ivec2	m_FlagSize = glm::ivec2( 52, 70 );
	float				m_FlagYOffset = 10;

	const glm::ivec2	m_UIFlagSize = glm::ivec2( 48, 48 );
	
	const rString m_SquadFlagWindowName = "SquadFlags";
	const rString m_UISquadFlagWindowName = "UISquadFlags";
	
	const glm::vec4	m_HighlightColour = glm::vec4( 0.2f, 0.2f, 0.2f, 0.0f );
};
