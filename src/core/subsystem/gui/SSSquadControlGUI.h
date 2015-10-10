/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <input/KeyBindings.h>
#include <gui/GUIEngine.h>

#define g_SSSquadControlGUI SSSquadControlGUI::GetInstance()

struct ButtonPressMemory;

class SSSquadControlGUI : public Subsystem
{
public:
	static SSSquadControlGUI&	GetInstance();

	void						Startup() override;
	void						Shutdown() override;
	void						UpdateUserLayer( const float deltaTime ) override;

	void						UpdateSquadHUDCenter();
private:
	// No external instancing allowed
	SSSquadControlGUI() : Subsystem( "SquadControlGUI" ) {}
	SSSquadControlGUI( const SSSquadControlGUI & rhs );
	~SSSquadControlGUI() {};
	SSSquadControlGUI& operator=( const SSSquadControlGUI & rhs );

	void						InitializeScriptFunctions();
	void						SendOrder(int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand);
	void						SendOrderInvoke(const rVector<int>& squads, int command, const int teamID, int genericValue);

	GUI::Window*				m_SquadInfoCenterWindow;
	GUI::Window*				m_SquadControlWindow;
	GUI::Text*					m_SquadInfoCenterText;


	
	const rString				m_SquadInfoCenterName = "SquadInfoCenter";
	const rString				m_SquadControlName = "SquadControl";

	const glm::vec4				m_ButtonSprite = glm::vec4( 0.8f, 0.8f, 0.8f, 1.0f );

	GUI::Button*				m_AttackMoveButton;
	GUI::Button*				m_AutoAttackButton;
	GUI::Button*				m_HoldPositionButton;
	const rString				m_FormationSetGroup = "FormationSet";
	const rString				m_FormationSpacingGroup = "FormationSpacing";
};