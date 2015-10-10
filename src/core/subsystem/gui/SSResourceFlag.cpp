/**************************************************
2015 Ola Enberg
***************************************************/

#include "SSResourceFlag.h"

#include <input/Input.h>
#include <gfx/GraphicsEngine.h>
#include "../gamelogic/SSResourceManager.h"
#include "../utility/SSMail.h"
#include "../../ai/Squad.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../input/SSPicking.h"

SSResourceFlag& SSResourceFlag::GetInstance( )
{
    static SSResourceFlag instance;
    return instance;
}

void SSResourceFlag::Startup( )
{
    glm::ivec2		winSize	= g_GUI.GetWindowSize( "RootWindow" );
	GUI::Window*	window	= g_GUI.AddWindow( m_ResourceFlagWindowName, GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "InGameWindow" );
	window->Open();

    Subsystem::Startup();
}

void SSResourceFlag::Shutdown( )
{
	g_GUI.DeleteObject( m_ResourceFlagWindowName );
	m_Flags.clear();

    Subsystem::Shutdown();
}

void SSResourceFlag::UpdateUserLayer( const float deltaTime )
{
	int flagsSize = static_cast<int>( m_Flags.size() );
	
	int flagID = 0;
	
	for( auto& resourceSpawn : g_SSResourceManager.GetResources() )
	{	
		glm::vec3	spawnWorldPos		= glm::vec3( resourceSpawn.Position.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( resourceSpawn.Position.x, resourceSpawn.Position.z ) + m_FlagYOffset, resourceSpawn.Position.z );
		glm::ivec2	pixelPos			= g_SSPicking.WorldToScreen( spawnWorldPos );
					pixelPos.y			-= m_FlagSize.y / 2;
					pixelPos.x			-= m_FlagSize.x / 2;

		if ( pixelPos.x + m_FlagSize.x <= 0 || pixelPos.y + m_FlagSize.y <= 0 )
			continue;

		if( flagID > flagsSize - 1 )
		{
			m_Flags.push_back( g_GUI.AddSprite( "", GUI::SpriteDefinition( "UIResourceDrop.png", 0, 0, m_FlagSize.x, m_FlagSize.y, g_SSResourceManager.GetResourceSpawnTypeColour( resourceSpawn.RespawnType ) ), m_ResourceFlagWindowName ) );
		}

		GUI::Sprite* flag = m_Flags[flagID];

		flag->SetVisible( true );
		flag->SetPosition( pixelPos.x, pixelPos.y );
		flag->GetSpriteDefinitionRef().Colour = g_SSResourceManager.GetResourceSpawnTypeColour( resourceSpawn.RespawnType );

		GUI::Rectangle boundingBox( pixelPos.x, pixelPos.y, m_FlagSize.x, m_FlagSize.y );
		
		if( boundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		{
			flag->GetSpriteDefinitionRef().Colour += glm::vec4( 0.2f, 0.2f, 0.2f, 0.0f );

			if ( g_Input->MouseUpDown( MOUSE_BUTTON_RIGHT ) )
			{
				if ( !g_PlayerData.GetSelectedSquads().empty() )
				{
					int					missionType			= Squad::MissionType::MISSION_TYPE_MINE;
					int					teamID				= g_PlayerData.GetPlayerID();
					const glm::vec3&	targetLocation		= resourceSpawn.Position;
					Entity				targetEntity		= ENTITY_INVALID;
					bool				overridingCommand	= g_Input->KeyUp( SDL_SCANCODE_LSHIFT );

					OrderUnitsMessage message = OrderUnitsMessage( g_GameData.GetFrameCount(), missionType, teamID, g_PlayerData.GetSelectedSquads(), targetLocation, targetEntity, overridingCommand );
					g_SSMail.PushToCurrentFrame( message );
				}

				g_Input->ConsumeMouseButtons();
			}
		}
		flagID++;
	}
	
	
	if( flagsSize > flagID )
	{
		for( int i = flagID; i < flagsSize; i++ )
		{
			m_Flags[i]->SetVisible( false );
		}
	}
}