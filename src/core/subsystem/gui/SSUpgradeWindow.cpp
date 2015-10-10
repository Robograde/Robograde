/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSUpgradeWindow.h"
#include <network/NetworkInfo.h>
#include <network/PacketPump.h>
#include "../../input/GameMessages.h"
#include <input/Input.h>
#include "../gamelogic/SSResearch.h"
#include "../gamelogic/SSUpgrades.h"
#include "../gamelogic/SSControlPoint.h"
#include "../gamelogic/SSAI.h"
#include "../input/SSKeyBinding.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameData.h"

#include "../SubsystemManager.h"

#include "../utility/SSMail.h"
#include "../../utility/Alliances.h"

#include "../input/SSButtonInput.h"
//#include "../gui/SSSquadControlGUI.h"
#include "../gui/SSGUIInfo.h"

#include "../../datadriven/ComponentHelper.h"
#include "../../component/PlacementComponent.h"
#include "../../component/OwnerComponent.h"

SSUpgradeWindow&SSUpgradeWindow::GetInstance()
{
	static SSUpgradeWindow instance;
	return instance;
}

void SSUpgradeWindow::Startup()
{
	int width = 256;
	int height = 64;
	
	glm::ivec2 screenSize = g_GUI.GetWindowSize( "RootWindow" );
	
	GUI::Window* window = g_GUI.AddWindow( m_UpgradeWindowName, GUI::Rectangle( 0, screenSize.y - height - m_CurrentResearchSize.y, width, height ), "InGameWindow", true );
	window->GetBackgroundRef().BorderSize = 0;
	g_GUI.OpenWindow( m_UpgradeWindowName );

	GUI::Button* minibtn = g_GUI.AddButton( "", GUI::Rectangle( width - 32, height, 32, 32 ), m_UpgradeWindowName );
	g_SSButtonInput.AddButtonBinding( minibtn, m_UpgradeWindowName, ACTION_CONTEXT_1, "" );
	minibtn->SetSecondaryTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	minibtn->SetBackgroundImage( "Button_32.png" );
	minibtn->GetBackgroundRef().BorderSize = 0;
	
	rString parentbutton = m_UpgradeWindowName; //Uncomment for second level access

	int btnIndex = 0;
	int x = 0;
	auto AddUpgradeTabWindow = [this, width, height, parentbutton, &btnIndex, &x] ( const rString& name, const rString& btnText  ) -> void
	{
		GUI::Window* window;
		window = g_GUI.AddWindow( name + "Window",	GUI::Rectangle( 0, 32, width, height ), m_UpgradeWindowName );
		window->SetToggleGroup( "URTabWindows" );

		int y = -32;
		int spacing = 0;

		g_GUI.UseFont( FONT_ID_LEKTON_11 );
		GUI::Button* btn;

		btn = g_GUI.AddButton( name, GUI::Rectangle( x, y, m_UpgradeButtonSize, 32 ), m_UpgradeWindowName );
		//btn->SetBackgroundImage( "Button_64x32.png" );
		btn->SetColour( glm::vec4( 0.0f ) );
		btn->SetDisabledColour( glm::vec4( 0.0f ) );
		btn->SetImagePath( "Button_UpgradeTab.png" );
		btn->SetIsCheckBox( true );
		btn->GetBackgroundRef().BorderSize = 0;
		//btn->SetHighLightColour( glm::vec4( 0.3f, 0.6f, 1.0f, 1.0f ) );
		btn->SetHighLightColour( glm::vec4( 0.8f ) );
		btn->SetImageHighLightColour( glm::vec4( 0.8f ) );

		btn->SetText( btnText );
		btn->SetClickScript( "ToggleWindowGroup('" + name + "Window', 'URTabWindows' )" );
		btn->SetToggleGroup( "URTabButtons" );
		
		g_SSButtonInput.AddButtonBinding( btn, name + "Window", static_cast<ACTION>( ACTION::ACTION_CONTEXT_1 + btnIndex ), parentbutton );
		x += m_UpgradeButtonSize + spacing;
		
		rString keyBinding = "Keybinding: " + g_SSKeyBinding.GetScancodeNameForAction( ACTION_CONTEXT_1 ) + 
				" -> " + g_SSKeyBinding.GetScancodeNameForAction( static_cast<ACTION>( ACTION_CONTEXT_1 + btnIndex ) );
		rString info = 
				"Choose what should be in the " + btnText + " slot \\nof the robots in your selected squad. \\n" +
				"Or research new upgrades. \\n" +
				keyBinding;
		btn->SetMouseEnterScript( "GE_SetInfoText( '" + info + "', 'InfoWindow1', 0 )" );
		btn->SetMouseLeaveScript( "GE_TryCloseInfo()" );

		btnIndex++;
	};

	AddUpgradeTabWindow( "FrontTab", "Front" );
	AddUpgradeTabWindow( "BackTab", "Back" );
	AddUpgradeTabWindow( "TopTab", "Top" );
	AddUpgradeTabWindow( "SidesTab", "Sides" );
	g_GUI.OpenWindow( "FrontTabWindow" );

	g_GUI.ToggleButtonGroup( "FrontTab", "URTabButtons" );
	g_GUI.ToggleWindowGroup( "FrontTabWindow", "URTabWindows" );

	auto replaceSpace = []( rString text ) -> rString
	{
		auto pos = text.find( ' ' );
		
		while( pos != rString::npos )
		{
			text.replace( pos, 1, "\n" );
			pos = text.find( ' ' );
		}
		
		return text;
	};
	
	//int upgradeIndex = 0;
	rString parent;
	rVector<UpgradeData>& upgradeData = g_SSUpgrades.GetUpgradeArray();

	int index = 0;
	for( UpgradeData& upgrade : upgradeData )
	{
		m_SortedUpgradeData.push_back( std::pair<UPGRADE_NAME, UpgradeData>( static_cast<UPGRADE_NAME>( index ), upgrade) );
		index++;
	}

	std::sort( m_SortedUpgradeData.begin(), m_SortedUpgradeData.end(), [](std::pair<UPGRADE_NAME, UpgradeData>& a, std::pair<UPGRADE_NAME, UpgradeData>& b ) 
	{ 
		return a.second.PointsToCompletion < b.second.PointsToCompletion;
	} );
	
	m_NumFrontButtons = 0;
	m_NumTopButtons = 0;
	m_NumBackButtons = 0;
	m_NumSidesButtons = 0;
	GUI::Button* btn;
	
	for( std::pair<UPGRADE_NAME, UpgradeData>& upgradePair : m_SortedUpgradeData )
	{
		UpgradeData& upgrade = upgradePair.second;
		UPGRADE_NAME upgradeName = upgradePair.first;
		int btnIndex = 0;
		int parentIndex = 0;
		
		if( upgrade.Default )
		{
			//upgradeIndex++;
			continue;
		}
		
		switch( upgrade.Slot )
		{
			case MODULE_SLOT_FRONT:
			{
				parent = "FrontTabWindow";
				btnIndex = m_NumFrontButtons;
				parentIndex = 0;
				m_NumFrontButtons++;
			} break;
			case MODULE_SLOT_BACK:
			{
				parent = "BackTabWindow";
				btnIndex = m_NumBackButtons;
				parentIndex = 1;
				m_NumBackButtons++;
			} break;
			case MODULE_SLOT_TOP:
			{
				parent = "TopTabWindow";
				btnIndex = m_NumTopButtons;
				parentIndex = 2;
				m_NumTopButtons++;
			} break;
			case MODULE_SLOT_SIDES:
			{
				parent = "SidesTabWindow";
				btnIndex = m_NumSidesButtons;
				parentIndex = 3;
				m_NumSidesButtons++;
			} break;
			default:
			{
			} break;
				
		}
		g_GUI.UseFont( FONT_ID_LEKTON_8 );
		btn = g_GUI.AddButton( upgrade.Name, GUI::Rectangle( 0, 0, m_UpgradeButtonSize, m_UpgradeButtonSize ), parent );
		
		btn->SetPosition( ( btnIndex % 4 ) * m_UpgradeButtonSize, -32 );
		btn->GetBackgroundRef().Texture = "Button_64.png";
		btn->SetImagePath( upgrade.ThumbnailPath );
		btn->GetBackgroundRef().BorderSize = 0;
		btn->SetImageHighLightColour( glm::vec4( 0.8f, 0.8f, 1.0f, 1.0f ) );
			
			
		btn->SetClickScript( "GE_UpgradeResearch(" + rToString( upgradeName ) + ")" );
		btn->SetMouseLeaveScript( "GE_TryCloseInfo()" );

		g_SSButtonInput.AddButtonBinding( btn, upgrade.Name, static_cast<ACTION>( ACTION::ACTION_CONTEXT_1 + btnIndex ), parent );

		GUI::ProgressBar* progressBar = g_GUI.AddProgressBar( "", GUI::Rectangle( btn->GetPosition().x, btn->GetPosition().y,
																					m_UpgradeButtonSize, m_ProgressBarHeight ), parent );
		progressBar->SetBackgroundColour( glm::vec4( 0.0f, 0.0f, 0.0f, 0.7f ) );

		g_GUI.UseFont( FONT_ID_LEKTON_11 );
		GUI::Text* researchTime = g_GUI.AddText( "", GUI::TextDefinition( "", btn->GetBoundingBoxRef().X + 1, progressBar->GetPosition().y, m_UpgradeButtonSize, m_ProgressBarHeight ), parent );
		researchTime->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
		researchTime->SetVisible( false );
		
		UpgradeButton button;
		button.Upgrade = upgradePair.first;
		button.Button = btn;
		button.ProgressBar = progressBar;
		button.ResearchTime = researchTime;
		button.ButtonIndex = btnIndex;
		button.ParentIndex = parentIndex;
		
		m_Buttons.push_back( button );
		
		//upgradeIndex++;
	}

	//m_ResearchGlow = g_GUI.AddSprite( "", GUI::SpriteDefinition( "ResearchGlow.png", -48, height - 48, m_CurrentResearchSize.x + 96, m_CurrentResearchSize.y + 96 , glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) ), m_UpgradeWindowName );

	g_GUI.UseFont( FONT_ID_LEKTON_8 );
	m_CurrentResearchProgress = g_GUI.AddProgressBar( "", GUI::Rectangle( 0, height, m_CurrentResearchSize.x, m_CurrentResearchSize.y ), m_UpgradeWindowName );
	m_CurrentResearchProgress->SetBackgroundTexture( "ResearchBar.png" );
	m_CurrentResearchProgress->SetBarColour( glm::vec4( 0.0f, 0.2f, 0.7f, 0.3f ) );

	m_CurrentResearchText = g_GUI.AddText( "", GUI::TextDefinition( "", 0, height, m_CurrentResearchSize.x, m_CurrentResearchSize.y ), m_UpgradeWindowName );
	m_CurrentResearchText->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );

	g_Script.Register( "GE_UpgradeResearch", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		UPGRADE_NAME name = static_cast<UPGRADE_NAME>( scriptEngine->PopInt() );
		
		if ( g_SSResearch.GetPlayerResearchDone( g_PlayerData.GetPlayerID(), name ) )
		{
			rVector<int> squads;
			
			for ( int squad : g_PlayerData.GetSelectedSquads() )
				if ( g_SSUpgrades.SquadSlotFree( g_PlayerData.GetPlayerID(), squad, g_SSUpgrades.GetUpgradeSlot( name ) ) )
					squads.push_back( squad );

			if ( !squads.empty( ) )
			{
				UpgradeMessage message = UpgradeMessage( g_GameData.GetFrameCount() + 1, g_PlayerData.GetPlayerID(), squads, name );
				g_SSMail.PushToNextFrame( message );
			}
			else
			{
				g_SSGUIInfo.DisplayMessage( "Squad already has an upgrade in that slot." );
			}

			//for ( int id : g_GameData.SelectedSquads )
			//g_SSUpgrades.GiveSquadUpgrade( g_GameData.GetPlayerID(), id, name );
		}
		else
		{
			if ( g_SSResearch.GetPlayerActiveResearch( g_PlayerData.GetPlayerID() ) != name )
			{
				if( g_Input->KeyDown( SDL_SCANCODE_LSHIFT, true ) || g_Input->KeyDown( SDL_SCANCODE_RSHIFT, true ) ) 
				{
					g_SSResearch.QueuePlayerResearch( g_PlayerData.GetPlayerID(), name );
				}
				else
				{
					g_SSResearch.SwitchPlayerActiveResearch( g_PlayerData.GetPlayerID(), name );
				}
			}
		}
		
		return 0; 
	
	}
	);

	StartupResearchQueue();
}

void SSUpgradeWindow::UpdateUserLayer(const float dt)
{
	bool totalResearching = false;
	bool hasResearchedEverything = true;
	int i = 0;
	int upgradeID = 0;

	int timeLeft = 0;
	//rVector<UpgradeData>& upgradeData = g_SSUpgrades.GetUpgradeArray();

	for( std::pair<UPGRADE_NAME, UpgradeData>& upgradePair : m_SortedUpgradeData )
	{
		UpgradeData& upgrade = upgradePair.second;
		upgradeID = upgradePair.first;
		
		if( upgrade.Default )
		{
			//upgradeID++;
			continue;
		}
		
		UpgradeButton& Button = m_Buttons[i];
		GUI::Button* btn = Button.Button;
		GUI::ProgressBar* progressBar = Button.ProgressBar;
		GUI::Text* currentResearchTime = Button.ResearchTime;
		//currentResearchTime->SetVisible( false );
		
		UPGRADE_NAME name = static_cast<UPGRADE_NAME>( upgradeID );
		
		bool researching = false;
		bool researched = false;
		bool upgraded = false;
		bool enabled = true;
	
		if ( g_SSResearch.GetPlayerResearchDone( g_PlayerData.GetPlayerID(), name ) )
		{
			btn->SetColour( m_ResearchedColour );
			progressBar->SetValue( 0.0f );
			researched = true;
			if( g_PlayerData.GetSelectedSquads().size() == 1 )
				currentResearchTime->SetText( "UC: " + rToString( g_SSUpgrades.GetUpgradeCost( name, g_PlayerData.GetPlayerID(), g_PlayerData.GetSelectedSquads()[0] ) ) );
			else
				currentResearchTime->SetText( "UC: N/A" );
		}
		else
		{
			hasResearchedEverything = false;
			
			int progress = g_SSResearch.GetPlayerResearchProgress( g_PlayerData.GetPlayerID(), name );
			

			if( progress > 0 )
			{
				progressBar->BringToFront();
				
			}
			currentResearchTime->SetVisible( true );

			progressBar->SetMaxValue( (float)upgrade.PointsToCompletion );
			progressBar->SetValue( (float)progress );

			currentResearchTime->SetVisible( true );

			int speed = g_SSControlPoint.GetOwnedControlPointsCount( g_PlayerData.GetPlayerID() );
			if( speed != 0 )
			{
				timeLeft = ( upgrade.PointsToCompletion - progress) / speed;
				m_UpgradeStatusInfo = rToString( ( upgrade.PointsToCompletion - progress) / speed ) + "s";
				
				currentResearchTime->SetText( rToString( timeLeft ) + "s" );
			}
			else
			{
				timeLeft = ( upgrade.PointsToCompletion - progress) / 1;
				m_UpgradeStatusInfo = "[C=RED]NEED RESEARCH CENTER";
				currentResearchTime->SetText( rToString( timeLeft ) + "s" );
			}

			if ( g_SSResearch.GetPlayerActiveResearch( g_PlayerData.GetPlayerID() ) == name )
			{
				btn->SetColour( m_ResearchingColour );
				
				m_CurrentResearchProgress->SetBackgroundColour( glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f ) );
				m_CurrentResearchProgress->SetMaxValue( (float)upgrade.PointsToCompletion );
				m_CurrentResearchProgress->SetValue( (float)progress );
				m_CurrentResearchText->SetText( "Current research:\n" + upgrade.Name );

				researching = true;
				totalResearching = true;
				
				m_UpgradeStatus = "[C=BLUE]RESEARCHING";

				if( speed == 1 )
					m_UpgradeStatusInfo = rToString( speed ) + " Research Center owned";
				else if( speed > 1 )
					m_UpgradeStatusInfo = rToString( speed ) + " Research Centers owned";
			}
			else
			{
				btn->SetColour( m_UnResearchedColour );
				researched = false;

				if( speed != 0 && progress > 0 )
				{
					if( speed == 1 )
						m_UpgradeStatusInfo = rToString( speed ) + " Research Center owned";
					else if( speed > 1 )
						m_UpgradeStatusInfo = rToString( speed ) + " Research Centers owned";
					
					currentResearchTime->SetText( rToString( ( upgrade.PointsToCompletion - progress) / speed ) + "s" );
					currentResearchTime->SetPosition( btn->GetBoundingBoxRef().X, btn->GetBoundingBoxRef().Y );
					currentResearchTime->SetVisible( true );
				}
			}
			
		}
		
		if ( g_PlayerData.GetSelectedSquads().size() == 0 && g_SSResearch.GetPlayerResearchDone( 0, name ) )
		{
			//btn->SetEnabled( false );
			enabled = false;
		}
		else
		{
			//btn->SetEnabled( true );
			enabled = true;
		}
		
		bool canAffordUpgrade = false;

		for ( int id : g_PlayerData.GetSelectedSquads() )
		{
			if ( g_SSUpgrades.SquadHasUpgrade( g_PlayerData.GetPlayerID( ), id, name ) )
			{
				//btn->SetEnabled( false );
				btn->SetColour( m_UpgradedColour );
				upgraded = true;
			} else if( g_SSUpgrades.GetUpgradeCost( name, g_PlayerData.GetPlayerID(), id ) < g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), id )->GetSize() )
				canAffordUpgrade = true;
			else
				btn->SetEnabled( true );


		}

		if( !canAffordUpgrade && researched &&  g_PlayerData.GetSelectedSquads().size() > 0 )
		{
			btn->SetColour( m_CantAffordColour );
		}
		
		if( researched && !upgraded )
		{
			m_UpgradeStatus = "[C=BLUE]RESEARCHED";
			if( enabled )
				m_UpgradeStatusInfo = "[C=WHITE]Click to upgrade";
			else
				m_UpgradeStatusInfo = "[C=WHITE]Select a squad to upgrade";
		}
		else if( upgraded )
		{
			m_UpgradeStatus = "[C=BLUE]UPGRADED";
			m_UpgradeStatusInfo = "[C=WHITE]Squad has this upgrade";
		}
		else if( !researched && !researching )
		{
			m_UpgradeStatus = "[C=PURPLE]NOT RESEARCHED";
			m_UpgradeStatusInfo = "[C=WHITE]Click to research";
		}
		
		rString keyBinding = "[C=WHITE]Keybinding:[C=BLUE] " + g_SSKeyBinding.GetScancodeNameForAction( ACTION_CONTEXT_1 ) +
				" -> " + g_SSKeyBinding.GetScancodeNameForAction( static_cast<ACTION>( ACTION_CONTEXT_1 + Button.ParentIndex ) ) + 
				" -> " + g_SSKeyBinding.GetScancodeNameForAction( static_cast<ACTION>( ACTION_CONTEXT_1 + Button.ButtonIndex ) );

		rString cost = "";

		if( !researched )
		{
			cost = "[C=BLLUE]Time to research:[C=BLUE] " + rToString( timeLeft ) + "s";
		}
		else
		{
			if( g_PlayerData.GetSelectedSquads().size() == 1 )
			{
				if( !upgraded )
					cost = "[C=WHITE]Upgrade cost:[C=BLUE] " + rToString( g_SSUpgrades.GetUpgradeCost( name, g_PlayerData.GetPlayerID(), g_PlayerData.GetSelectedSquads()[0] ) ) + " Units";
				else
					cost = "[C=WHITE]Upgrade cost:[C=PURPLE] Selected squad already have this upgrade";
			}
			else if( upgraded )
				cost = "[C=WHITE]Upgrade cost: [C=RED]Select ONE squad to see the cost";
		}

		rString info = 
				"[C=WHITE]Upgrade:[C=BLUE] " +		upgrade.Name + "\\n" +
				cost + "\\n" +
				//"[C=WHITE]Description:[C=WHITE] " +	upgrade.Description + "\\n" + 
				upgrade.Description + "\\n" +
				keyBinding + "\\n" +
				m_UpgradeStatus + " - " + m_UpgradeStatusInfo;
		
		btn->SetMouseEnterScript( "GE_SetInfoText( '" + info + "', 'InfoWindow1', 0 )" );
		
		i++;
		//upgradeID++;
	}
	
	if( !totalResearching  ) 
	{
		//m_CurrentResearchProgress->SetBackgroundColour( glm::vec4( 1.0f, 0.0f, 0.1f, 1.0f ) );
		m_CurrentResearchProgress->SetValue( 0.0f );
		

		if ( !hasResearchedEverything )
		{
			m_CurrentResearchProgress->SetBackgroundColour( glm::vec4( ( 1.2f - glm::sin( m_GlowTime ) ) / 2, 0.0f, 0.0f, 1.0f ) );
			m_CurrentResearchText->SetText( "No current\nresearch" );
		}
		else
		{
			m_CurrentResearchProgress->SetBackgroundColour( glm::vec4( 0.0f, 0.7f, 0.1f, 1.0f ) );
			m_CurrentResearchText->SetText( "Everything researched" );
		}

		//m_ResearchGlow->GetSpriteDefinitionRef().Colour.a = ( 1.2f - glm::sin( m_GlowTime ) ) / 2;
		m_GlowTime += dt * 10;
	}

	UpdateResearchQueue();
}

void SSUpgradeWindow::Shutdown()
{
	m_SortedUpgradeData.clear();
	m_Buttons.clear();
	m_ResearchQueueButtons.clear();
	g_GUI.DeleteObject( m_UpgradeWindowName );
	g_GUI.DeleteObject( "ResearchQueue" );
	g_GUI.ClearButtonGroup( "URTabButtons" );
	g_GUI.ClearWindowGroup( "URTabWindows" );
}


void SSUpgradeWindow::StartupResearchQueue()
{
	g_GUI.AddWindow( "ResearchQueue", GUI::Rectangle( 0, 0, 256, 64 ), "InGameWindow" );
	g_GUI.OpenWindow( "ResearchQueue" );


	g_Script.Register( "GE_UnQueueResearch", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		UPGRADE_NAME name = static_cast<UPGRADE_NAME>( scriptEngine->PopInt() );

		g_SSResearch.UnQueuePlayerResearch( g_PlayerData.GetPlayerID(), name );
		return 0;
	} );
}

void SSUpgradeWindow::UpdateResearchQueue()
{
	rVector<UpgradeData>& upgradeData = g_SSUpgrades.GetUpgradeArray();

	const pDeque<int>& researchQueue = g_SSResearch.GetPlayerResearchQueue( g_PlayerData.GetPlayerID() );

	int count = 0;
	for( int upgradeID : researchQueue )
	{
		UpgradeData& upgrade = upgradeData.at( upgradeID );
		GUI::Button* button;
		if( m_ResearchQueueButtons.size() < count + 1 )
		{
			button = g_GUI.AddButton( "", GUI::Rectangle( 0, 0, m_UpgradeButtonSize, m_UpgradeButtonSize ), "ResearchQueue" );
			button->SetBackgroundImage( "Button_64.png" );

			m_ResearchQueueButtons.push_back( button );
		}
		button = m_ResearchQueueButtons.at( count );
		button->SetPosition( count * m_UpgradeButtonSize, 0 );
		button->SetImagePath( upgrade.ThumbnailPath );
		button->SetVisible( true );
		button->SetClickScript( "GE_UnQueueResearch(" + rToString( upgradeID ) + ")" );
		count++;
	}

	if( m_ResearchQueueButtons.size() > count )
	{
		for( int i = count; i < m_ResearchQueueButtons.size(); i++ )
		{
			m_ResearchQueueButtons[i]->SetVisible( false ); //Remember to reset this :)
		}
	}
}