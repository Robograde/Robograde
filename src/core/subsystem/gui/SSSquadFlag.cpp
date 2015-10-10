/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSSquadFlag.h"
#include <input/Input.h>
#include <SDL2/SDL.h>
#include <gfx/GraphicsEngine.h>
#include "../gamelogic/SSCamera.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameData.h"
#include "../../utility/Alliances.h"
#include "../gfx/SSFogOfWar.h"
#include "../input/SSPicking.h"
#include "../utility/SSMail.h"

#include <messaging/GameMessages.h>

#include "../../component/ColourComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/SelectionComponent.h"
#include "../../datadriven/DenseComponentCollection.h"

#include "../gamelogic/SSUpgrades.h"

SSSquadFlag& SSSquadFlag::GetInstance()
{
	static SSSquadFlag instance;
	return instance;
}

void SSSquadFlag::Startup()
{
	glm::ivec2 winSize = g_GUI.GetWindowSize( "RootWindow" );
	
	GUI::Window* window = g_GUI.AddWindow( m_SquadFlagWindowName, GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "InGameWindow" );
	window->Open();

	window = g_GUI.AddWindow( m_UISquadFlagWindowName, GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "InGameWindow" );
	window->Open();
}

void SSSquadFlag::Shutdown()
{
	g_GUI.DeleteObject( m_SquadFlagWindowName );
	g_GUI.DeleteObject( m_UISquadFlagWindowName );
	m_Flags.clear();
	m_UIFlags.clear();
}

void SSSquadFlag::UpdateUserLayer( const float deltaTime )
{
	int flagsSize = static_cast<int>( m_Flags.size() );
	int UIflagsSize = static_cast<int>( m_UIFlags.size() );
	
	int flagID = 0;
	int UIflagID = 0;
	
	for( SSAI::TeamAI& team : g_SSAI.m_Teams )
	{
		for( Squad* squad : team.Squads )
		{
			if (squad->GetSize() == 0)
				continue;
			
			glm::vec3 squadWorldPos = glm::vec3( squad->GetPosition().x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( squad->GetPosition().x, squad->GetPosition().y ) + m_FlagYOffset, squad->GetPosition().y );
			if( g_SSFogOfWar.CalcVisibilityForPosition( squadWorldPos ) )
			{
				glm::ivec2 pixelPos = g_SSPicking.WorldToScreen( squadWorldPos );
				pixelPos.y -= m_FlagSize.y;
				pixelPos.x -= m_FlagSize.x / 2;
				int upgradeSpriteSize = ( m_FlagSize.x / 2 ) - 2;
				
				int x = 2;
				int y = 2;

				if( flagID > flagsSize - 1 )
				{
					m_Flags.push_back( CreateFlag( "Flag", m_FlagSize, m_SquadFlagWindowName ) );
				}

				SquadFlag& flag = m_Flags[flagID];
				
				UpdateFlag( flag, pixelPos, glm::ivec2( 2 ), upgradeSpriteSize, squad );
				
				HandleClicks( squad, squadWorldPos, flag );
				
				flagID++;
			}

			if( team.TeamID == g_PlayerData.GetPlayerID() )
			{
				if( UIflagID > UIflagsSize - 1 )
				{
					m_UIFlags.push_back( CreateFlag( "UIFlag", m_UIFlagSize, m_UISquadFlagWindowName ) );

					g_GUI.BringWindowToFront( m_UISquadFlagWindowName );
				}

				SquadFlag& flag = m_UIFlags[UIflagID];

				int offset = 2;

				int xPos = 256;
				int yPos = g_GameData.GetWindowHeight() - m_UIFlagSize.y - offset - 22;

				

				UpdateFlag( flag, glm::ivec2( xPos + m_UIFlagSize.x * UIflagID + offset*(UIflagID+1) , yPos ), glm::ivec2( 0 ), m_UIFlagSize.x / 2, squad );
				HandleClicks( squad, squadWorldPos, flag );
				UIflagID++;
			}

		}
	}
	
	
	if( flagsSize > flagID )
	{
		for( int i = flagID; i < flagsSize; i++ )
		{
			SquadFlag& flag = m_Flags[i];
			
			flag.Background->SetVisible( false );
			for ( int slot = 0; slot < 4; slot++ )
				flag.Slots[slot]->SetVisible( false );
			flag.UpgradeBar->SetVisible( false );

			flag.SizeTextBackground->SetVisible( false );
			flag.SizeText->SetVisible( false );

			flag.ControlGroups->SetVisible( false );
			flag.ControlGroupsBackground->SetVisible( false );
		}
	}

	if( UIflagsSize > UIflagID )
	{
		for( int i = UIflagID; i < UIflagsSize; i++ )
		{
			SquadFlag& flag = m_UIFlags[i];
			
			flag.Background->SetVisible( false );
			for ( int slot = 0; slot < 4; slot++ )
				flag.Slots[slot]->SetVisible( false );
			flag.UpgradeBar->SetVisible( false );

			flag.SizeTextBackground->SetVisible( false );
			flag.SizeText->SetVisible( false );

			flag.ControlGroups->SetVisible( false );
			flag.ControlGroupsBackground->SetVisible( false );
		}
	}
}

SquadFlag SSSquadFlag::CreateFlag( const rString& sprite, glm::ivec2 size, const rString& windowName )
{
	const int upgradeSpriteSize = ( size.x / 2 ) - 2;

	SquadFlag flag;
	flag.Sprite = sprite + ".png";

	flag.Background = g_GUI.AddSprite( "", GUI::SpriteDefinition( flag.Sprite, 0, 0, size.x, size.y, glm::vec4( 0.5f, 0.5f, 0.8f, 0.8f ) ), windowName );
	flag.HighLightSprite = sprite + "_Selected.png";
			
	const rString asset = "upgrade/";

	for ( int slot = 0; slot < 4; slot++ )
		flag.Slots[slot] = g_GUI.AddSprite( "", GUI::SpriteDefinition( asset + "", 0, 0, upgradeSpriteSize, upgradeSpriteSize, glm::vec4( 1.0f, 1.0f, 1.0f, 0.0f ) ), windowName );
					
	flag.BoundingBox = GUI::Rectangle( 0, 0, size.x, size.y );
					
	flag.UpgradeBar = g_GUI.AddProgressBar( "", GUI::Rectangle( 0, 0, size.x - 4, 10), windowName );
	flag.UpgradeBar->SetBackgroundColour( glm::vec4( 0.1f, 0.1f, 0.2f, 1.0f ) );
	flag.UpgradeBar->SetBorderColour( glm::vec4( 0.1f, 0.1f, 0.2f, 1.0f ) );
	flag.UpgradeBar->SetBarColour( glm::vec4( 0.2f, 0.2f, 0.9f, 1.0f ) );
	flag.UpgradeBar->SetMinValue( 0.0f);
	flag.UpgradeBar->SetMaxValue( 1.0f );
	flag.UpgradeBar->SetValue( 1.0f );

	flag.SizeTextBackground = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, size.x, 14, glm::vec4( 0.2f, 0.2f, 0.2f, 0.8f ) ), windowName );
	flag.SizeTextBackground->GetSpriteDefinitionRef().BorderSize = 2;
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	flag.SizeText = g_GUI.AddText( "", GUI::TextDefinition( "blaha", 0, 0, size.x - 4, 14 ), windowName );
	flag.SizeText->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );

	flag.ControlGroupsBackground = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, size.x, 14, glm::vec4( 0.2f, 0.2f, 0.2f, 0.8f ) ), windowName );
	flag.ControlGroupsBackground->GetSpriteDefinitionRef().BorderSize = 2;
	flag.ControlGroups = g_GUI.AddText( "", GUI::TextDefinition( "1", 0, 0, size.x, 14 ), windowName );
	flag.ControlGroups->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );

	return flag;
}

void SSSquadFlag::UpdateFlag( SquadFlag& flag, glm::ivec2 pixelPos, glm::ivec2 iconOffset, int iconSize, Squad* squad )
{
	flag.BoundingBox.X = pixelPos.x;
	flag.BoundingBox.Y = pixelPos.y;
				
	flag.Background->SetVisible( true );
	for ( int slot = 0; slot < 4; slot++ )
		flag.Slots[slot]->SetVisible( true );

	if( squad->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
	{
		flag.UpgradeBar->SetVisible( true );
		flag.UpgradeBar->SetValue( squad->GetUpgradeProgress() );
	}
	else
		flag.UpgradeBar->SetVisible( false );

	flag.SizeTextBackground->SetVisible( true );
	flag.SizeText->SetVisible( true );

	//Positioning
	flag.Slots[0]->GetSpriteDefinitionRef().Position = pixelPos +	glm::ivec2( iconOffset.x, iconOffset.y );
	flag.Slots[1]->GetSpriteDefinitionRef().Position = pixelPos +	glm::ivec2( iconOffset.x + iconSize, iconOffset.y );
	flag.Slots[2]->GetSpriteDefinitionRef().Position = pixelPos +	glm::ivec2( iconOffset.x, iconOffset.y + iconSize );
	flag.Slots[3]->GetSpriteDefinitionRef().Position = pixelPos +	glm::ivec2( iconOffset.x + iconSize, iconOffset.y + iconSize );
				
				
	flag.SizeTextBackground->SetPosition(  pixelPos.x, pixelPos.y - 12 );
	flag.SizeText->SetPosition(  pixelPos.x, pixelPos.y - 12 );

	flag.UpgradeBar->SetPosition( pixelPos.x + 2, pixelPos.y - 12 - 10 );
				
	//Colouring
	ColourComponent* colourComp = GetDenseComponent<ColourComponent>( squad->GetLeader()->GetEntityID() );
	flag.Background->GetSpriteDefinitionRef().Colour = colourComp->Colour - glm::vec4( 0.0f, 0.0f, 0.0f, 0.2f );
	flag.HighLightColour = (colourComp->Colour - glm::vec4( 0.0f, 0.0f, 0.0f, colourComp->Colour.a )) / glm::vec4( 2 );

	flag.Background->GetSpriteDefinitionRef().Position = glm::ivec2( pixelPos.x, pixelPos.y );
				
	flag.Selected = false;
	for( int selectedSquad : g_PlayerData.GetSelectedSquads() )
	{
		if( selectedSquad == squad->GetID() && squad->GetTeam() == g_PlayerData.GetPlayerID() )
			flag.Selected = true;
	}
				
	if( flag.Selected )
	{
		flag.Background->GetSpriteDefinitionRef().Colour += flag.HighLightColour;
		flag.Background->GetSpriteDefinitionRef().Texture = flag.HighLightSprite;
	}
	else
		flag.Background->GetSpriteDefinitionRef().Texture = flag.Sprite;
				
	//Upgrades
	ParentComponent* parent = GetDenseComponent<ParentComponent>(squad->GetLeader()->GetEntityID());
				
				
	for ( int slot = 0; slot < 4; slot++ )
	{
		if ( parent->Children[slot] != ENTITY_INVALID )
		{
			UpgradeComponent* upgrade = GetDenseComponent<UpgradeComponent>(parent->Children[slot]);
	
			flag.Slots[slot]->GetSpriteDefinitionRef().Texture = g_SSUpgrades.GetUpgrade(upgrade->UpgradeDataID).ThumbnailPath;
			flag.Slots[slot]->GetSpriteDefinitionRef().Colour.a = 1.0f;
		}
		else
		{
			flag.Slots[slot]->GetSpriteDefinitionRef().Texture = "";
		}
					
		if( flag.Slots[slot]->GetSpriteDefinitionRef().Texture == "" )
			flag.Slots[slot]->GetSpriteDefinitionRef().Colour.a = 0.0f;
	}

	//Squad size
	flag.SizeText->SetText( rToString( squad->GetSize() ) + "/" + rToString( SQUAD_MAXIMUM_UNIT_COUNT ) );

	if( squad->GetTeam() == g_PlayerData.GetPlayerID() )
	{
		fString controlGroups = g_PlayerData.GetControlGroupStringForSquad( squad->GetID(), 3 );
		flag.ControlGroups->SetText( controlGroups.c_str() );

		flag.ControlGroups->SetVisible( true );

		if( controlGroups != "" )
			flag.ControlGroupsBackground->SetVisible( true );
		else
			flag.ControlGroupsBackground->SetVisible( false );

		flag.ControlGroups->SetPosition(			pixelPos.x, pixelPos.y + flag.BoundingBox.Height - 2 );
		flag.ControlGroupsBackground->SetPosition(	pixelPos.x, pixelPos.y + flag.BoundingBox.Height - 2 );

	}
	else
	{
		flag.ControlGroupsBackground->SetVisible( false );
		flag.ControlGroups->SetVisible( false );
	}
				
	flag.Squad = squad->GetID();
}

void SSSquadFlag::HandleClicks( Squad* squad, const glm::vec3& squadWorldPos, const SquadFlag& flag )
{
	if( !flag.BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		return;

	if( !flag.Selected )
		flag.Background->GetSpriteDefinitionRef().Colour += flag.HighLightColour;

	for( Agent* agent : squad->GetAgents() )
		GetDenseComponent<SelectionComponent>( agent->GetEntityID() )->MouseOvered = true;
		
	if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT ) )
	{
		if( squad->GetTeam() != g_PlayerData.GetPlayerID() )
		{
			g_PlayerData.GetEditableSelectedEntities().clear();
			g_PlayerData.GetEditableSelectedSquads().clear();
			for( Agent* agent : squad->GetAgents() )
			{
				g_PlayerData.GetEditableSelectedEntities().push_back( agent->GetEntityID() );
			}
		}
		else
		{
			if( g_Input->KeyDown( SDL_SCANCODE_LSHIFT ) )
			{
				int copyIndex = -1;
				for ( int j = 0; j < g_PlayerData.GetSelectedSquads().size(); ++j )
				{
					if ( flag.Squad == g_PlayerData.GetSelectedSquads()[j] )
					{
						copyIndex = j;
						break;
					}
				}
				
				if( copyIndex != -1 )
					g_PlayerData.GetEditableSelectedSquads().erase( g_PlayerData.GetSelectedSquads().begin() + copyIndex );
				else
					g_PlayerData.GetEditableSelectedSquads().push_back( flag.Squad );

					
			}
			else
			{
				bool isInSelection = false;
				for( int squadID : g_PlayerData.GetSelectedSquads() )
				{
					if( squadID == flag.Squad )
					{
						const glm::vec2& squadPosition = g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), squadID )->GetPosition();
						g_SSCamera.GetRTSCamera()->LookAtPosition( glm::vec3( squadPosition.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( squadPosition.x, squadPosition.y ), squadPosition.y ) );
						isInSelection = true;
					}
				}
				rVector<int> selectedSquad;
				selectedSquad.push_back( flag.Squad );
				g_PlayerData.SetSelectedSquads( selectedSquad );
			}
		}
			
		g_Input->ConsumeMouseButtons();
	}
	if( g_Input->MouseDownUp( MOUSE_BUTTON_RIGHT ) )
	{
		g_Input->ConsumeMouseButtons();
		if (squad->GetTeam() != g_PlayerData.GetPlayerID() && g_PlayerData.GetSelectedSquads().size() > 0 && !g_Alliances.IsAllied(g_PlayerData.GetPlayerID(), squad->GetTeam()))
		{
			int					missionType			= Squad::MissionType::MISSION_TYPE_ATTACK;
			int					teamID				= g_PlayerData.GetPlayerID();
			const glm::vec3&	targetLocation		= squadWorldPos;
			Entity				targetEntity		= squad->GetLeader()->GetEntityID();
			bool				overridingCommand	= g_Input->KeyUp( SDL_SCANCODE_LSHIFT );

			OrderUnitsMessage message = OrderUnitsMessage( g_GameData.GetFrameCount(), missionType, teamID, g_PlayerData.GetSelectedSquads(), targetLocation, targetEntity, overridingCommand );
			g_SSMail.PushToCurrentFrame( message );
				
		}
	}
	if( g_Input->MouseUpDown( MOUSE_BUTTON_RIGHT ) )
	{
		g_Input->ConsumeMouseButtons();
	}
}
