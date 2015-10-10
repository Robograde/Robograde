/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSSquadControlGUI.h"
#include "../../utility/PlayerData.h"
#include "../../component/UpgradeComponent.h"
#include "../../datadriven/ComponentHelper.h"
#include "../../input/GameMessages.h"
#include "../gamelogic/SSAI.h"
#include "../gamelogic/SSUpgrades.h"

#include "../../utility/GameData.h"

#include "../gamelogic/SSAI.h"
#include <input/Input.h>
#include "../input/SSKeyBinding.h"
#include "../input/SSButtonInput.h"
#include "../utility/SSMail.h"

SSSquadControlGUI& SSSquadControlGUI::GetInstance()
{
	static SSSquadControlGUI guiInput;
	return guiInput;
}

void SSSquadControlGUI::Startup()
{
	InitializeScriptFunctions();

	g_GUI.UseFont(FONT_ID_CONSOLE_11);
	int numButtons = 15; //Including empty buttons
	int minimapSize = 256;
	int upgradeWindowSize = 256;
	int squadControlHeight = 48;
	int buttonSize = 48;
	int centerBarHeight = 96;
	glm::ivec2 winSize = g_GUI.GetWindowSize( "InGameWindow" );
	
	
	m_SquadInfoCenterWindow = g_GUI.AddWindow( m_SquadInfoCenterName, GUI::Rectangle( minimapSize, winSize.y - centerBarHeight, winSize.x - 512, centerBarHeight ), "InGameWindow", true );
	g_GUI.OpenWindow( m_SquadInfoCenterName );
	g_GUI.SetWindowClickThrough( m_SquadInfoCenterName, true );
	

	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	m_SquadInfoCenterText = g_GUI.AddText( "", GUI::TextDefinition( "", 5, 0), m_SquadInfoCenterName );
	m_SquadInfoCenterText->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_LEFT );
	
	
	m_SquadControlWindow = g_GUI.AddWindow( m_SquadControlName, GUI::Rectangle( upgradeWindowSize, winSize.y - centerBarHeight - buttonSize, winSize.x - minimapSize - upgradeWindowSize, squadControlHeight), "InGameWindow", true );
	g_GUI.OpenWindow( m_SquadControlName );
	m_SquadControlWindow->BringToFront();
	m_SquadControlWindow->GetBackgroundRef().Colour = glm::vec4( 0.1f, 0.1f, 0.1f, 0.8f );
	m_SquadControlWindow->GetBackgroundRef().Texture = "bar1.png";
	m_SquadControlWindow->SetClickThrough( true );
	

	//g_SSButtonInput.AddButtonBinding( nullptr, m_SquadControlName, ACTION_CONTEXT_2, "" );
	
	
	
	int btnIndex = 0;
	int x = winSize.x - minimapSize - upgradeWindowSize - ( numButtons * buttonSize);
	int y = 0;
	auto AddGUIButton = [&] (const rString& script, ACTION action, const rString& imagePath, const rString& info, const rString& parent ) mutable -> GUI::Button*
	{
		GUI::Button* btn;
		btn = g_GUI.AddButton( script, GUI::Rectangle(x, y, buttonSize, buttonSize), m_SquadControlName);
		btn->SetBackgroundImage( "Button_48.png" );
		btn->SetImagePath( imagePath );
		btn->GetBackgroundRef().BorderColour = glm::vec4( 0.7f );
		btn->SetClickScript( script );
		btn->SetColour(glm::vec4( 0.9f, 0.9f, 0.9f, 0.9f ));
		btn->SetHighLightColour( glm::vec4( 0.3f, 0.6f, 1.0f, 1.0f ) );
		btn->SetImageHighLightColour( glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f ) );
		btn->GetImageRef().Colour = m_ButtonSprite;
		btn->GetBackgroundRef().BorderSize = 0;
		
		rString keyBinding = "Keybinding: ";
		if( parent == "" )
			keyBinding += g_SSKeyBinding.GetScancodeNameForAction( action );
		else if( parent == m_FormationSetGroup )
			keyBinding += g_SSKeyBinding.GetScancodeNameForAction( ACTION_CONTEXT_2 ) + " -> " + g_SSKeyBinding.GetScancodeNameForAction( action );
		else if( parent == m_FormationSpacingGroup )
			keyBinding += g_SSKeyBinding.GetScancodeNameForAction( ACTION_CONTEXT_3 ) + " -> " + g_SSKeyBinding.GetScancodeNameForAction( action );

		int toolTipPos;
		if( x + 288 > m_SquadControlWindow->GetSize().x )
			toolTipPos = m_SquadControlWindow->GetPosition().x + m_SquadControlWindow->GetSize().x - 288;
		else
			toolTipPos = m_SquadControlWindow->GetPosition().x + x;

		btn->SetMouseEnterScript( "GE_SetInfoText('" + info + "\\n" + keyBinding + "', 'InfoWindow2', " + rToString( toolTipPos ) + " )" );
		btn->SetMouseLeaveScript( "GE_TryCloseInfo()" );

		if( parent != "" )
			btn->SetToggleGroup( parent );
		
		g_SSButtonInput.AddButtonBinding( btn, "", action, parent );
		
		x += buttonSize;
		btnIndex++;

		return btn;
	};
	int halfButton = buttonSize / 2;

	GUI::Button* minibtn;
	
	minibtn = g_GUI.AddButton( "", GUI::Rectangle( x + 4*buttonSize, y + halfButton, halfButton, halfButton ), m_SquadControlName );
	g_SSButtonInput.AddButtonBinding( minibtn, m_FormationSetGroup, ACTION_CONTEXT_2, "" );
	minibtn->SetSecondaryTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	minibtn->SetBackgroundImage( "Button_32.png" );
	minibtn->GetBackgroundRef().BorderSize = 0;

	AddGUIButton( "GE_SetFormation(1)", ACTION_CONTEXT_1, "squadcontrol/FORM_COLUMN.png", "Set Column formation", m_FormationSetGroup );
	AddGUIButton( "GE_SetFormation(2)", ACTION_CONTEXT_2, "squadcontrol/FORM_LINE.png", "Set Line formation", m_FormationSetGroup );
	AddGUIButton( "GE_SetFormation(3)", ACTION_CONTEXT_3, "squadcontrol/FORM_HEDGEHOG.png", "Set Hedgehog formation", m_FormationSetGroup );
	AddGUIButton( "GE_SetFormation(4)", ACTION_CONTEXT_4, "squadcontrol/FORM_POCKET.png", "Set Pocket formation", m_FormationSetGroup );
	
	x += buttonSize;

	minibtn = g_GUI.AddButton( "", GUI::Rectangle( x + 3*buttonSize, y + halfButton, halfButton, halfButton ), m_SquadControlName );
	g_SSButtonInput.AddButtonBinding( minibtn, m_FormationSpacingGroup, ACTION_CONTEXT_3, "" );
	minibtn->SetSecondaryTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	minibtn->SetBackgroundImage( "Button_32.png" );
	minibtn->GetBackgroundRef().BorderSize = 0;

	AddGUIButton( "GE_SetFormationSpacing(1)", ACTION_CONTEXT_1, "squadcontrol/SPACE_TIGHT.png", "Set squad spacing 1", m_FormationSpacingGroup);
	AddGUIButton( "GE_SetFormationSpacing(2)", ACTION_CONTEXT_2, "squadcontrol/SPACE_NORMAL.png", "Set squad spacing 2", m_FormationSpacingGroup );
	AddGUIButton( "GE_SetFormationSpacing(3)", ACTION_CONTEXT_3, "squadcontrol/SPACE_SPARSE.png", "Set squad spacing 3", m_FormationSpacingGroup );

	x += buttonSize;
	
	m_AttackMoveButton =	AddGUIButton( "GE_AttackMove()", ACTION_AI_ATTACK, "squadcontrol/ATTACK.png", "Toggle attack move", "" );
							AddGUIButton( "GE_Stop()", ACTION_SQUADS_STOP, "squadcontrol/STOP.png", "Stop squad", "" );
							AddGUIButton( "GE_Merge()", ACTION_AI_SQUAD_MERGE, "squadcontrol/MERGE.png", "Merge selected squads", "" );
							AddGUIButton( "GE_Split()", ACTION_AI_SQUAD_SPLIT, "squadcontrol/SPLIT.png", "Split selected squad into two", "" );
	m_AutoAttackButton =	AddGUIButton( "GE_AutoAttack()", ACTION_AI_INVOKE_AUTOATTACK, "squadcontrol/AUTO_ATTACK.png", "Toggle auto attack", "" );
	m_HoldPositionButton =	AddGUIButton( "GE_HoldPosition()", ACTION_SQUADS_HOLD_POSITION, "squadcontrol/HOLD.png", "Hold position", "" );




}

void SSSquadControlGUI::Shutdown()
{
	g_GUI.DeleteObject( m_SquadControlName );
	g_GUI.DeleteObject( m_SquadInfoCenterName );
	g_GUI.DeleteObject("PlayerGenericHelpText");
	g_GUI.DeleteObject("PlayerGenericHelpTextShadow");

	g_GUI.ClearButtonGroup( m_FormationSetGroup );
	g_GUI.ClearButtonGroup( m_FormationSpacingGroup );
}

void SSSquadControlGUI::UpdateUserLayer( float deltaTime )
{
	//UpdateSquadHUDCenter();

	if( g_PlayerData.GetLastActionPressed() == ACTION::ACTION_AI_ATTACK )
		m_AttackMoveButton->SetToggled( true );
	else
		m_AttackMoveButton->SetToggled( false );

	const rVector<int>& squadIDs = g_PlayerData.GetSelectedSquads();
	if( squadIDs.size() > 0 )
	{
		Squad* squad = g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), squadIDs[0] );
		switch( squad->GetFormation() )
		{
			case Squad::FORMATION_COLUMN:
				g_GUI.ToggleButtonGroup( "GE_SetFormation(1)", m_FormationSetGroup );
			break;
			case Squad::FORMATION_LINE:
				g_GUI.ToggleButtonGroup( "GE_SetFormation(2)", m_FormationSetGroup );
			break;
			case Squad::FORMATION_HEDGEHOG:
				g_GUI.ToggleButtonGroup( "GE_SetFormation(3)", m_FormationSetGroup );
			break;
			case Squad::FORMATION_LINE_OFFENSIVE:
				g_GUI.ToggleButtonGroup( "GE_SetFormation(4)", m_FormationSetGroup );
			break;
			default:
				g_GUI.ToggleButtonGroup( "", m_FormationSetGroup );
			break;
		}

		int spacing = squad->GetSpacingStep();
		switch( spacing )
		{
			case 1:
				g_GUI.ToggleButtonGroup( "GE_SetFormationSpacing(1)", m_FormationSpacingGroup );
			break;
			case 2:
				g_GUI.ToggleButtonGroup( "GE_SetFormationSpacing(2)", m_FormationSpacingGroup );
			break;
			case 3:
				g_GUI.ToggleButtonGroup( "GE_SetFormationSpacing(3)", m_FormationSpacingGroup );
			break;
			default:
				g_GUI.ToggleButtonGroup( "", m_FormationSpacingGroup );
			break;
		}

		if( squad->GetUseAutoAttack() )
			m_AutoAttackButton->SetToggled( true );
		else
			m_AutoAttackButton->SetToggled( false );

		if( squad->GetUseHoldPosition() )
			m_HoldPositionButton->SetToggled( true );
		else
			m_HoldPositionButton->SetToggled( false );
	}
	else
	{
		g_GUI.ToggleButtonGroup( "", m_FormationSetGroup );
		g_GUI.ToggleButtonGroup( "", m_FormationSpacingGroup );
		m_AutoAttackButton->SetToggled( false );
		m_HoldPositionButton->SetToggled( false );
	}
}


void SSSquadControlGUI::UpdateSquadHUDCenter()
{
	const rVector<int> squads = g_PlayerData.GetSelectedSquads();
	int numberOfSquadsSelected = static_cast<int>(squads.size());

	if (numberOfSquadsSelected > 0)
	{
		rVector<Squad*> squadPointers;

		rString squadstring = "Squads:";
		squadstring.resize(19, ' ');

		for (int i = 0; i < numberOfSquadsSelected; i++)
		{
			squadstring += rToString(squads[i]);
			if (i < numberOfSquadsSelected - 1)
				squadstring += ", ";

			squadPointers.push_back( g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), squads[i] ) );
		}

		rString sizes = "\nTotal size: ";
		sizes.resize(20, ' ');
		int sizeOfSelectedSquads = 0;
		for (int i = 0; i < numberOfSquadsSelected; i++)
		{
			sizeOfSelectedSquads += squadPointers[i]->GetSize();
		}
		sizes += rToString(sizeOfSelectedSquads);
		sizes += " / " + rToString( SQUAD_MAXIMUM_UNIT_COUNT * squadPointers.size( ) );

		rString formations = "\nFormation: ";
		formations.resize(20, ' ');
		rString savedFormation = "";
		for (int i = 0; i < numberOfSquadsSelected; i++)
		{
			rString newFormation = squadPointers[i]->GetFormationName();
			if (newFormation != savedFormation && savedFormation != "")
			{
				savedFormation = "Mixed";
				break;
			}
			else
				savedFormation = newFormation;
		}
		formations += savedFormation;

		rString autoattack = "\nAuto Attack: ";
		autoattack.resize(20, ' ');
		int usingAutoAttack = -1;
		bool isMixed = false;
		for (int i = 0; i < numberOfSquadsSelected; i++)
		{
			int temp = squadPointers[i]->GetUseAutoAttack();
			if (temp != usingAutoAttack && usingAutoAttack != -1)
			{
				isMixed = true;
				break;
			}
			else
				usingAutoAttack = temp;
		}
		if (isMixed)
			autoattack += "Mixed";
		else
			autoattack += rToString(usingAutoAttack);

		rString upgrades = "\nUpgrades: ";
		upgrades.resize(20, ' ');
		rString slots[4];
		slots[0] = "null"; slots[1] = "null"; slots[2] = "null"; slots[3] = "null";

		for (int i = 0; i < numberOfSquadsSelected; i++)
		{
			ParentComponent* parent = GetDenseComponent<ParentComponent>(squadPointers[i]->GetLeader()->GetEntityID());

			for (int slot = 0; slot < 4; slot++)
			{
				if (slots[slot] == "Mixed")
					continue;

				if (parent->Children[slot] != ENTITY_INVALID)
				{
					UpgradeComponent* upgrade = GetDenseComponent<UpgradeComponent>(parent->Children[slot]);

					if (slots[slot] == "null")
						slots[slot] = g_SSUpgrades.GetUpgrade(upgrade->UpgradeDataID).Name;
					else if (slots[slot] != g_SSUpgrades.GetUpgrade(upgrade->UpgradeDataID).Name)
						slots[slot] = "Mixed";
				}
				else if (slots[slot] == "null")
					slots[slot] = "Empty";
				else if (slots[slot] != "Empty")
					slots[slot] = "Mixed";
			}
		}
		upgrades += slots[0] + ", " + slots[1] + ", " + slots[2] + ", " + slots[3] + ", ";

		m_SquadInfoCenterText->SetText( squadstring + sizes + formations + autoattack + upgrades );
	}
	else
		m_SquadInfoCenterText->SetText( "" );
}

void SSSquadControlGUI::InitializeScriptFunctions()
{
	g_Script.Register( "GE_HoldPosition", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrder( Squad::MissionType::MISSION_TYPE_HOLD, g_PlayerData.GetPlayerID(), g_PlayerData.GetSelectedSquads(), glm::vec3( 0.0f ), ENTITY_INVALID, g_Input->KeyUp( SDL_SCANCODE_LSHIFT ) ); // TODOOE / TODOVK: Get rid of g_Input?
		return 0; 
	} );

	g_Script.Register( "GE_Stop", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::INVOKE_STOP, g_PlayerData.GetPlayerID(), 0 );
		return 0; 
	} );
	
	g_Script.Register( "GE_Merge", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::SQUAD_MERGE, g_PlayerData.GetPlayerID(), 0 );
		return 0; 
	} );
	
	g_Script.Register( "GE_Split", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::SQUAD_SPLIT, g_PlayerData.GetPlayerID(), 0 );
		return 0; 
	} );
	
	g_Script.Register( "GE_AutoAttack", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::INVOKE_AUTO_ATTACK, g_PlayerData.GetPlayerID(), 2 );
		return 0; 
	} );
	
	g_Script.Register( "GE_SetFormationSpacing", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::FORMATION_SET_SPACE, g_PlayerData.GetPlayerID(), scriptEngine->PopInt() );
		return 0; 
	} );
	
	g_Script.Register( "GE_SetFormation", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::FORMATION_SET, g_PlayerData.GetPlayerID(), scriptEngine->PopInt() );
		return 0; 
	} );
	
	g_Script.Register( "GE_AttackMove", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		g_PlayerData.SetLastActionPressed( ACTION_AI_ATTACK );
		return 0; 
	} );
}

void SSSquadControlGUI::SendOrder( int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand )
{
	OrderUnitsMessage message = OrderUnitsMessage( g_GameData.GetFrameCount(), missionType, teamID, squads, targetLocation, targetEntity, overridingCommand );
	g_SSMail.PushToCurrentFrame( message );
}

void SSSquadControlGUI::SendOrderInvoke( const rVector<int>& squads, int command, const int teamID, int genericValue )
{
	OrderInvokeMessage message = OrderInvokeMessage( g_GameData.GetFrameCount(), squads, command, teamID, genericValue );
	g_SSMail.PushToCurrentFrame( message );
}