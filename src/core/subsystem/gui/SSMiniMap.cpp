/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSMiniMap.h"

#include <network/NetworkInfo.h>
#include <network/PacketPump.h>
#include <messaging/GameMessages.h>
#include "../SubsystemManager.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/AgentComponent.h"
#include "../../component/ColourComponent.h"
#include "../../component/ResourceComponent.h"
#include "../../component/ControlPointComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/ComponentHelper.h"

#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/Alliances.h"

#include "../gfx/SSFogOfWar.h"

#include "../network/SSNetworkController.h"

#include "../gamelogic/SSCamera.h"
#include "../gamelogic/SSAI.h"
#include "../gamelogic/SSResourceManager.h"
#include "../utility/SSMail.h"

#include <input/Input.h>

#include "../gamelogic/SSSceneLoader.h"

#include "../input/SSPicking.h"
#include "../gamelogic/SSRadar.h"

#include "../input/SSKeyBinding.h"
#include "../input/SSButtonInput.h"

#include "../../utility/GameModeSelector.h"

SSMiniMap& SSMiniMap::GetInstance()
{
	static SSMiniMap instance;
	return instance;
}

void SSMiniMap::Startup()
{
	g_SSMail.RegisterSubscriber( this );
	m_UserInterests = MessageTypes::USER_PING_MESSAGE;

	g_SSMiniMap.SetMapSize(g_GameData.GetFieldWidth(), g_GameData.GetFieldHeight());
	glm::ivec2 winSize = g_GUI.GetWindowSize( "InGameWindow" );
	int mapSize = 256;
	
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
//	GUI::Button* btn = g_GUI.AddButton("MiniMapToggle", GUI::Rectangle(winSize.x - 96, winSize.y - 32 - mapSize, 96, 32), "InGameWindow");
//	btn->SetText( "MiniMap" );
//	btn->SetClickScript( "ToggleWindow( 'MiniMapWindow' )" );
//	btn->SetToggled( true );
	
	
	g_GUI.AddWindow( "MiniMapWindow", GUI::Rectangle( winSize.x - mapSize, winSize.y - mapSize, mapSize, mapSize ), "InGameWindow", true );
	//g_GUI.SetWindowMoveable( "MiniMapWindow", true );
	
	g_GUI.AddWindow( "MiniMapMainWindow", GUI::Rectangle( 0, 0, mapSize, mapSize ), "MiniMapWindow" );
	g_GUI.AddWindow( "MiniMapPingsWindow", GUI::Rectangle( 0, 0, mapSize, mapSize ), "MiniMapWindow" );
	g_GUI.AddWindow( "MiniMapCameraWindow", GUI::Rectangle( 0, 0, mapSize, mapSize ), "MiniMapWindow" );
	g_GUI.AddWindow( "MiniMapButtonWindow", GUI::Rectangle( 0, 0, mapSize, mapSize ), "MiniMapWindow" );
	g_GUI.OpenWindow( "MiniMapMainWindow" );
	g_GUI.OpenWindow( "MiniMapPingsWindow" );
	g_GUI.OpenWindow( "MiniMapCameraWindow" );
	g_GUI.OpenWindow( "MiniMapButtonWindow" );
	
	//TODOIA: Temp background that should be more dynamic when proper levels are available
	//const rString diffuseTexture = g_SSSceneLoader.GetScene().GetDiffuseTex();
	m_BackgroundSprite = g_GUI.AddSprite( "mapbg" , GUI::SpriteDefinition( "minimap/Battlefield.png" , 1 , 1 , mapSize - 2 , mapSize - 2 ) , "MiniMapMainWindow" );
	m_BackgroundSprite->GetSpriteDefinitionRef().TopLeftUV.y = 0.0f;
	m_BackgroundSprite->GetSpriteDefinitionRef().BottomRightUV.y = 1.0f;
	if ( m_MiniMapTexturePath != "" )
	{
		m_MiniMapTexture = tNew( gfx::Texture );
		m_MiniMapTexture->Init( m_MiniMapTexturePath.c_str(), gfx::TextureType::TEXTURE_2D );

		m_BackgroundSprite->GetSpriteDefinitionRef().TextureHandle2 = m_MiniMapTexture->GetHandle();
		m_BackgroundSprite->GetSpriteDefinitionRef().Texture = "";
	}
	
//	GUI::Sprite* heightmap = g_GUI.AddSprite( "mapHeight", GUI::SpriteDefinition( "minimap/RoboWarMap.png", 1, 1, mapSize-2, mapSize-2 ), "MiniMapMainWindow" );
//	heightmap->GetSpriteDefinitionRef().Colour.a = 0.1f;
	

	
	m_FogSprite = g_GUI.AddSprite( "fogOfWar", GUI::SpriteDefinition( "", 1, 1, mapSize-2, mapSize-2 ), "MiniMapMainWindow" );
	m_FogSprite->GetSpriteDefinitionRef().TextureHandle = g_SSFogOfWar.GetTextureHandle();
	m_FogSprite->GetSpriteDefinitionRef().BottomRightUV.y = 0.0f;
	m_FogSprite->GetSpriteDefinitionRef().TopLeftUV.y = 1.0f;

	// Initialize camera area sprites, and variables needed for it.
	m_PickingTarget.push_back( PICKING_TYPE_TERRAIN );
	glm::ivec2 mapPos;
	GUI::Sprite* sprite;
	m_CameraSprites.reserve( 24 );
	for ( int i = 0; i < 24; ++i ) {
		mapPos = ConvertToMapCoordinates( glm::vec3( 10.0f + i ) );
		sprite = g_GUI.AddSprite( "CameraSprite" + rToString( i ), 
									   GUI::SpriteDefinition( "", mapPos.x, mapPos.y, CAMERA_SPRITE_SIZE, CAMERA_SPRITE_SIZE ), "MiniMapCameraWindow" );
		m_CameraSprites.push_back( sprite );
	};
	
	g_GUI.OpenWindow( "MiniMapWindow" );

	g_GUI.UseFont( FONT_ID_LEKTON_8 );

	if( g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::Replay )
	{
		g_Script.Register( "GE_MakePing", []( IScriptEngine* )
			{
				g_PlayerData.SetLastActionPressed( ACTION_PING );
				return 0;
			} );
		GUI::Button* btnPing = g_GUI.AddButton( "", GUI::Rectangle( 256 - 64 - m_PingButtonWidth, - m_PingButtonHeight, m_PingButtonWidth, m_PingButtonHeight ), "MiniMapButtonWindow" );
		btnPing->SetBackgroundImage( "Button_32.png" );
		btnPing->SetText( "Ping" );
		btnPing->SetClickScript( "GE_MakePing()" );

		GUI::Button* btnUnmoved = g_GUI.AddButton( "", GUI::Rectangle( 256 - 64, -32, 64, 32 ), "MiniMapButtonWindow" );
		btnUnmoved->SetBackgroundImage( "Button_64x32.png" );
		btnUnmoved->SetClickScript( "GE_SelectUnmovedSquads()" );

		btnUnmoved->SetText( "Select\nunused" );

		g_SSButtonInput.AddButtonBinding( btnUnmoved, "", ACTION_SELECT_UNMOVED, "" );
	}

	Subsystem::Startup();
}

void SSMiniMap::UpdateUserLayer( const float dt )
{
	for ( const Message* message : m_UserMailbox )
	{
		const UserPingMessage* pingMessage = static_cast<const UserPingMessage*>( message );
		if ( g_Alliances.IsAllied( g_PlayerData.GetPlayerID( ), pingMessage->PingingPlayerID ) )
		{
			PingLocation( pingMessage->WorldPosition, glm::vec3( 0.0f, 1.0f, 0.0f ) );
			g_SSMail.PushToUserLayer( SFXPingMessage( pingMessage->PingingPlayerID , 0) ); ///Type == 0 == user ping
		}
	}

	if( !g_GUI.IsWindowOpen( "MiniMapWindow" ) )
		return;

	glm::ivec2 mapPos;
	GUI::Sprite* sprite;
	int unitCount = 0;
	m_ControlPointCount = 0;
	int resourceCount = 0;
	
	EntityMask agentFlag = GetDenseComponentFlag<AgentComponent>();
	EntityMask controlPointFlag = GetDenseComponentFlag<ControlPointComponent>();
	EntityMask resourceFlag = GetDenseComponentFlag<ResourceComponent>();
	
	Entity entity = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		//Unit rendering
		if ( entityMask & agentFlag )
		{
			ColourComponent* colourComp = GetDenseComponent<ColourComponent>( entity );
			PlacementComponent* placementComp = GetDenseComponent<PlacementComponent>( entity );
			
			if( g_SSFogOfWar.CalcVisibilityForPosition( placementComp->Position ) )
			{
				mapPos = ConvertToMapCoordinates( placementComp->Position );
				
				if( m_UnitSprites.size() < unitCount + 1 )
				{
					sprite = g_GUI.AddSprite( "UnitSprite" + rToString( unitCount ), 
								   GUI::SpriteDefinition( "", mapPos.x, mapPos.y, UNIT_SPRITE_SIZE, UNIT_SPRITE_SIZE ), "MiniMapMainWindow" );
					sprite->GetSpriteDefinitionRef().CenterX = true;
					sprite->GetSpriteDefinitionRef().CenterY = true;
					m_UnitSprites.push_back( sprite  );
				}
				else
				{
					m_UnitSprites[unitCount]->SetVisible( true );
					m_UnitSprites[unitCount]->GetSpriteDefinitionRef().Position = mapPos;
				}
				
				m_UnitSprites[unitCount]->GetSpriteDefinitionRef().Colour = colourComp->Colour;
				unitCount++;
			}
		}
		
		//Control point rendering
		else if ( entityMask & controlPointFlag )
		{
			ColourComponent* colourComp = GetDenseComponent<ColourComponent>( entity );
			PlacementComponent* placementComp = GetDenseComponent<PlacementComponent>( entity );
			if( m_ControlPointSprites.size() < m_ControlPointCount + 1 )
			{
				mapPos = ConvertToMapCoordinates( placementComp->Position );
				sprite = g_GUI.AddSprite( "ControlPointSprite" + rToString( m_ControlPointCount ), 
							   GUI::SpriteDefinition( "minimap/controlPoint.png", mapPos.x, mapPos.y,
													  CONTROL_POINT_SPRITE_SIZE, CONTROL_POINT_SPRITE_SIZE ), "MiniMapMainWindow" );
				sprite->GetSpriteDefinitionRef().CenterX = true;
				sprite->GetSpriteDefinitionRef().CenterY = true;
				m_ControlPointSprites.push_back( sprite  );
			}
			
			m_ControlPointSprites[m_ControlPointCount]->GetSpriteDefinitionRef().Colour = colourComp->Colour;
			m_ControlPointSprites[m_ControlPointCount]->GetSpriteDefinitionRef().Colour.a = 0.8f;
			m_ControlPointCount++;
		}
		
		//Resource rendering
		else if ( entityMask & resourceFlag )
		{
			PlacementComponent* placementComp = GetDenseComponent<PlacementComponent>( entity );
			if( g_SSFogOfWar.CalcVisibilityForPosition( placementComp->Position ) )
			{
				mapPos = ConvertToMapCoordinates( placementComp->Position );
				
				if( m_ResourceSprites.size() < resourceCount + 1 )
				{
					sprite = g_GUI.AddSprite( "ResourceSprite" + rToString( resourceCount ), 
								   GUI::SpriteDefinition( "minimap/controlPoint.png", mapPos.x, mapPos.y, 
														  RESOURCE_SPRITE_SIZE, RESOURCE_SPRITE_SIZE ), "MiniMapMainWindow" );
					sprite->GetSpriteDefinitionRef().CenterX = true;
					sprite->GetSpriteDefinitionRef().CenterY = true;
					sprite->GetSpriteDefinitionRef().Colour = glm::vec4( 0.1f, 0.1f, 0.1f, 0.8f );
					
					m_ResourceSprites.push_back( sprite  );
				}
				else
				{
					m_ResourceSprites[resourceCount]->SetVisible( true );
					m_ResourceSprites[resourceCount]->GetSpriteDefinitionRef().Position = mapPos;
				}
				
				m_ResourceSprites[resourceCount]->GetSpriteDefinitionRef().Colour = glm::vec4( 0.1f, 0.1f, 0.1f, 0.8f );
				resourceCount++;
			}
		}
		
		entity++;
	}

	// Render radar objects
	int radarCount = 0;
	for ( auto& radarHit : g_SSRadar.GetRadarHits()[g_PlayerData.GetPlayerID()] )
	{
		if ( !g_SSFogOfWar.CalcVisibilityForPosition( radarHit.Position ) )
		{
			mapPos = ConvertToMapCoordinates( radarHit.Position );

			if ( m_RadarSprites.size( ) < radarCount + 1 )
			{
				sprite = g_GUI.AddSprite( "RadarSprite" + rToString( radarCount ),
					GUI::SpriteDefinition( "", mapPos.x, mapPos.y, UNIT_SPRITE_SIZE, UNIT_SPRITE_SIZE ), "MiniMapMainWindow" );
				sprite->GetSpriteDefinitionRef( ).CenterX = true;
				sprite->GetSpriteDefinitionRef( ).CenterY = true;
				m_RadarSprites.push_back( sprite );
			}
			else
			{
				m_RadarSprites[radarCount]->SetVisible( true );
				m_RadarSprites[radarCount]->GetSpriteDefinitionRef( ).Position = mapPos;
			}

			m_RadarSprites[radarCount]->GetSpriteDefinitionRef( ).Colour = glm::vec4( radarHit.SignalStrength, radarHit.SignalStrength, radarHit.SignalStrength, radarHit.SignalStrength );
			radarCount++;
		}
	}

	// Render resource spawns.
	m_ResourceSpawnCount = 0;
	for ( auto& resourceSpawn : g_SSResourceManager.GetResources() )
	{
		mapPos = ConvertToMapCoordinates( resourceSpawn.Position );

		if ( m_ResourceSpawnSprites.size( ) < m_ResourceSpawnCount + 1 )
		{
			sprite = g_GUI.AddSprite( "ResourceSpawnSprite" + rToString( m_ResourceSpawnCount ),
				GUI::SpriteDefinition( "minimap/Resource.png", mapPos.x, mapPos.y, RESOURCE_SPAWN_SPRITE_SIZE, RESOURCE_SPAWN_SPRITE_SIZE ), "MiniMapMainWindow" );
			sprite->GetSpriteDefinitionRef( ).CenterX = true;
			sprite->GetSpriteDefinitionRef( ).CenterY = true;
			m_ResourceSpawnSprites.push_back( sprite );
		}
		else
		{
			m_ResourceSpawnSprites[m_ResourceSpawnCount]->GetSpriteDefinitionRef( ).Position = mapPos;
			m_ResourceSpawnSprites[m_ResourceSpawnCount]->SetVisible( true );
		}

		m_ResourceSpawnSprites[m_ResourceSpawnCount]->GetSpriteDefinitionRef( ).Colour		= g_SSResourceManager.GetResourceSpawnTypeColour( resourceSpawn.RespawnType );
		m_ResourceSpawnSprites[m_ResourceSpawnCount]->GetSpriteDefinitionRef( ).Colour.a	= 0.8f;
		m_ResourceSpawnCount++;
	}
	
	//Don't want to render uneccesary sprites 
	if( m_UnitSprites.size() > unitCount + 1 )
	{
		for( int i = unitCount; i < m_UnitSprites.size(); i++ )
		{
			m_UnitSprites[i]->SetVisible( false ); //Remember to reset this :)
		}
	}
	
	if( m_ResourceSprites.size() > resourceCount + 1 )
	{
		for( int i = resourceCount; i < m_ResourceSprites.size(); i++ )
		{
			m_ResourceSprites[i]->SetVisible( false ); //Remember to reset this :)
		}
	}

	if( m_ResourceSpawnSprites.size() > m_ResourceSpawnCount + 1 )
	{
		for( int i = m_ResourceSpawnCount; i < m_ResourceSpawnSprites.size(); i++ )
		{
			m_ResourceSpawnSprites[i]->SetVisible( false ); //Remember to reset this :)
		}
	}

	if ( m_RadarSprites.size( ) > radarCount + 1 )
	{
		for ( int i = radarCount; i < m_RadarSprites.size( ); i++ )
		{
			m_RadarSprites[i]->SetVisible( false ); //Remember to reset this :)
		}
	}

	UpdatePings( dt );
	UpdateCameraArea( );
	HandleClicks();
}

void SSMiniMap::SendUserPing( const glm::vec3& position )
{
	short			playerID	= g_PlayerData.GetPlayerID( );
	UserPingMessage message		= UserPingMessage( playerID, position );

	for ( auto& ally : g_Alliances.GetAllies( playerID ) )
		if ( ally != playerID && g_SSNetworkController.DoesPlayerExist( ally ) )
			g_PacketPump.Send( message, ally );
	g_SSMail.PushToUserLayer( message );

	g_PlayerData.SetLastActionPressed( ACTION_SIZE );
	if ( g_SSKeyBinding.ActionDown( ACTION_PING ) )
		g_PlayerData.SetLastActionPressed( ACTION_PING );
}


void SSMiniMap::UpdatePings( float deltaTime )
{
	int pingCount = 0;
	for ( auto& ping : m_PingList )
	{
		ping.TimeLeft -= deltaTime;
		if ( ping.TimeLeft <= 0.0f )
			continue;

		glm::ivec2 mapPos = ping.Position;

		if ( m_PingSprites.size( ) < pingCount + 1 )
		{
			GUI::Sprite* sprite = g_GUI.AddSprite( "PingSprite" + rToString( pingCount ),
				GUI::SpriteDefinition( "minimap/Ping.png", mapPos.x, mapPos.y, PING_SPRITE_SIZE, PING_SPRITE_SIZE ), "MiniMapPingsWindow" );
			sprite->GetSpriteDefinitionRef( ).CenterX	= true;
			sprite->GetSpriteDefinitionRef( ).CenterY	= true;

			m_PingSprites.push_back( sprite );
		}
		else
		{
			m_PingSprites[pingCount]->SetVisible( true );
			m_PingSprites[pingCount]->GetSpriteDefinitionRef( ).Position = mapPos;
		}

		float interpol = ping.TimeLeft / ping.PingLength;

		int size = (int) (PING_SPRITE_SIZE * interpol);
		size += size % 2;

		float alpha = 1.0f - 1.5f * (1.0f - interpol);
		alpha = alpha < 0.0f ? 0.0f : alpha;

		m_PingSprites[pingCount]->GetSpriteDefinitionRef( ).Colour	= glm::vec4( ping.Colour.r, ping.Colour.g, ping.Colour.b, alpha );
		m_PingSprites[pingCount]->GetSpriteDefinitionRef( ).Width	= size;
		m_PingSprites[pingCount]->GetSpriteDefinitionRef( ).Height	= size;
		pingCount++;
	}

	if ( m_PingSprites.size( ) > pingCount + 1 )
	{
		for ( int i = pingCount; i < m_PingSprites.size( ); i++ )
		{
			m_PingSprites[i]->SetVisible( false ); //Remember to reset this :)
		}
	}
}

void SSMiniMap::PingLocation( glm::vec3 worldPos, glm::vec3 colour )
{
	PingData* newPing = nullptr;

	// Reuse old ping
	for ( auto& ping : m_PingList )
	{
		if ( ping.TimeLeft <= 0.0f )
		{
			newPing = &ping;
			break;
		}
	}

	// Create new ping
	if ( newPing == nullptr )
	{
		m_PingList.push_back( PingData( ) );
		newPing = &m_PingList.back( );
	}

	newPing->Position	= ConvertToMapCoordinates( worldPos );
	newPing->Colour		= colour;
	newPing->PingLength = PING_LIFE_TIME;
	newPing->TimeLeft	= PING_LIFE_TIME;
}

void SSMiniMap::SetMiniMapTexture( const rString& texturePath )
{
	m_MiniMapTexturePath = texturePath;
}

void SSMiniMap::Shutdown()
{
	g_SSMail.UnregisterSubscriber( this );
	g_GUI.DeleteObject( "MiniMapWindow" );
	g_GUI.DeleteObject( "MiniMapToggle" );
	m_UnitSprites.clear();
	m_SquadSprites.clear();
	m_ControlPointSprites.clear();
	m_ResourceSprites.clear();
	m_ResourceSpawnSprites.clear();
	m_CameraSprites.clear();
	m_PickingTarget.clear();
	m_PingSprites.clear( );
	tDelete( m_MiniMapTexture );
}

void SSMiniMap::SetMapSize( int width, int height )
{
	m_MapSizeX = width;
	m_MapSizeY = height;
}

glm::ivec2 SSMiniMap::ConvertToMapCoordinates( const glm::vec3& worldPos )
{
	glm::ivec2 winSize = g_GUI.GetWindowSize( "MiniMapWindow" );
	return glm::ivec2 ( ( worldPos.x / m_MapSizeX ) * winSize.x, ( worldPos.z / m_MapSizeY ) * winSize.y );
}

glm::vec3 SSMiniMap::ConvertToWorldCoordinates(const glm::ivec2& mapPos)
{
	glm::ivec2 winSize = g_GUI.GetWindowSize( "MiniMapWindow" );
	return glm::vec3 ( ( mapPos.x /  (float)winSize.x ) * (float)m_MapSizeX, 0.0f, ( mapPos.y /  (float)winSize.y ) * m_MapSizeY );
}

void SSMiniMap::HandleClicks()
{
	//Clickity stuff
	if( m_BackgroundSprite->GetBoundingBoxRef().Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) && !g_SSPicking.IsBoxSelecting() )
	{		
		bool		sendMission	= false;
		int			missionType	= Squad::MissionType::MISSION_TYPE_NONE;
		glm::ivec2	localPos	= glm::ivec2(	g_Input->GetMousePosX() - m_BackgroundSprite->GetBoundingBoxRef().GetLeft(),
												g_Input->GetMousePosY() - m_BackgroundSprite->GetBoundingBoxRef().GetTop() );

		if( g_Input->MouseDown( MOUSE_BUTTON_LEFT ) )
		{
			if ( g_PlayerData.GetLastActionPressed( ) != ACTION::ACTION_AI_ATTACK && g_PlayerData.GetLastActionPressed( ) != ACTION::ACTION_PING )
			{
				glm::vec3 worldPos = ConvertToWorldCoordinates( localPos );
				g_SSCamera.GetRTSCamera()->LookAtPosition( worldPos );
			}
			g_Input->ConsumeMouseButtons();
		}
		else if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT, true ) )
		{
			if ( g_PlayerData.GetSelectedSquads().size() > 0 && g_PlayerData.GetLastActionPressed() == ACTION::ACTION_AI_ATTACK )
			{
				sendMission = true;
				missionType = Squad::MissionType::MISSION_TYPE_ATTACK_MOVE;
				if ( g_Input->KeyUp( SDL_SCANCODE_LSHIFT ) )
					g_PlayerData.SetLastActionPressed( ACTION::ACTION_SIZE );
			}
			else if ( g_PlayerData.GetLastActionPressed( ) == ACTION::ACTION_PING )
			{
				SendUserPing( ConvertToWorldCoordinates( localPos ) );
			}
			g_Input->ConsumeMouseButtons();
		}
		
		int mouseOverControlPointIndex	= -1;
		int mouseOverResourceSpawnIndex	= -1;

		for( int i = 0; i < m_ControlPointCount; i++ )
		{
			GUI::Sprite* sprite = m_ControlPointSprites[i];
			if( sprite->GetBoundingBoxRef().Intersects( g_Input->GetMousePosX() + ( sprite->GetSpriteDefinitionRef().Width / 2 ),
														g_Input->GetMousePosY() + ( sprite->GetSpriteDefinitionRef().Height / 2 ) ) )
			{
				sprite->GetSpriteDefinitionRef().Colour = sprite->GetSpriteDefinitionRef().Colour + glm::vec4( 0.4f, 0.4f, 0.4f, 0.0f );
				localPos = glm::ivec2( sprite->GetBoundingBoxRef().X, sprite->GetBoundingBoxRef().Y );
				mouseOverControlPointIndex = i;
				break;
			}
		}

		for ( int i = 0; i < m_ResourceSpawnCount; i++ )
		{
			GUI::Sprite* sprite = m_ResourceSpawnSprites[i];
			if( sprite->GetBoundingBoxRef().Intersects( g_Input->GetMousePosX() + ( sprite->GetSpriteDefinitionRef().Width / 2 ),
														g_Input->GetMousePosY() + ( sprite->GetSpriteDefinitionRef().Height / 2 ) ) )
			{
				sprite->GetSpriteDefinitionRef().Colour = sprite->GetSpriteDefinitionRef().Colour + glm::vec4( 0.4f, 0.4f, 0.4f, 0.0f );
				localPos = glm::ivec2( sprite->GetBoundingBoxRef().X, sprite->GetBoundingBoxRef().Y );
				mouseOverResourceSpawnIndex = i;
				break;
			}
		}

		if( g_Input->MouseDownUp( MOUSE_BUTTON_RIGHT ) )
		{
			if( g_PlayerData.GetSelectedSquads().size() > 0 )
			{
				sendMission = true;
				missionType = Squad::MissionType::MISSION_TYPE_MOVE;

				if ( mouseOverResourceSpawnIndex >= 0 )
				{
					sendMission = true;
					missionType = Squad::MissionType::MISSION_TYPE_MINE;
				}
				else if ( mouseOverControlPointIndex >= 0 )
				{
					sendMission = true;
					missionType = Squad::MissionType::MISSION_TYPE_MOVE;
				}
			}
			g_Input->ConsumeMouseButtons();
			g_PlayerData.SetLastActionPressed( ACTION_SIZE );
		}
		
		if( g_Input->MouseUpDown( MOUSE_BUTTON_RIGHT ) )
			g_Input->ConsumeMouseButtons();

		if ( sendMission )
		{
			int					teamID				= g_PlayerData.GetPlayerID();
			const rVector<int>&	squads				= g_PlayerData.GetSelectedSquads();
			const glm::vec3&	targetLocation		= ConvertToWorldCoordinates( localPos );
			Entity				targetEntity		= ENTITY_INVALID;
			bool				overridingCommand	= g_Input->KeyUp( SDL_SCANCODE_LSHIFT );
			
			OrderUnitsMessage message = OrderUnitsMessage( g_GameData.GetFrameCount(), missionType, teamID, squads, targetLocation, targetEntity, overridingCommand );
			g_SSMail.PushToCurrentFrame( message );
		}
	}

	// World click
	if ( g_PlayerData.GetLastActionPressed( ) == ACTION::ACTION_PING && g_Input->MouseDownUp( MOUSE_BUTTON_LEFT ) )
	{
		glm::vec3 position;
		m_Picking.CalcTerrainPosUnderMouse( *g_SSCamera.GetActiveCamera( ), &position );
		SendUserPing( position );

		g_Input->ConsumeMouseButtons( );
	}
}

void SSMiniMap::UpdateCameraArea()
{
	glm::ivec2 mapPos;
	glm::ivec2 screenSize( g_SSCamera.GetActiveCamera()->GetEditableLens().WindowWidth, g_SSCamera.GetActiveCamera()->GetEditableLens().WindowHeight );
	glm::ivec2 minimapSize = g_GUI.GetWindowSize( "MiniMapWindow" );

	int cornersAdded = 0;
	for ( int y = 0; y < 2; ++y )
	{
		for ( int x = y; x < 2 && x >= 0; x = ((y == 0) ? x + 1 : x - 1) )
		{
			m_Picking.ClearUnitSelection();
			m_Picking.PickRay( m_PickingTarget, *g_SSCamera.GetActiveCamera(), screenSize * glm::ivec2( x, y ) );
			mapPos = ConvertToMapCoordinates( m_Picking.GetClickedPosition() );
			m_CameraSprites[cornersAdded++]->GetSpriteDefinitionRef().Position = mapPos;
		}
	}

	for ( int i = 0; i < 4; ++i )
	{
		glm::ivec2 basePos	= m_CameraSprites[i]->GetSpriteDefinitionRef().Position;
		glm::ivec2 diff		= m_CameraSprites[( i + 1 ) % 4]->GetSpriteDefinitionRef().Position - basePos;
		for ( int j = 0; j < 5; ++j )
		{
			m_CameraSprites[4 + i*5 + j]->GetSpriteDefinitionRef().Position = basePos + (diff * (j+1) / 6);
		}
	}

	for ( int i = 0; i < m_CameraSprites.size(); ++i )
	{
		mapPos = m_CameraSprites[i]->GetSpriteDefinitionRef().Position;
		m_CameraSprites[i]->SetVisible( !(mapPos.x < 0 || mapPos.y < 0 || mapPos.x > minimapSize.x || mapPos.y > minimapSize.y) );
	}
}

const rString& SSMiniMap::GetMiniMapTexturePath() const //TODIA: Place this where it should be in this file, couldn't figure it out //DB
{
	return m_MiniMapTexturePath;
}
