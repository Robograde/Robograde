/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSEditorObjectPlacer.h"
#include <glm/glm.hpp>
#include <input/Input.h>
#include <collisionDetection/ICollisionDetection.h>
#include <utility/Logger.h>
#include <utility/Colours.h>
#include <glm/glm.hpp>
#include "SSEditorToolbox.h"
#include "../../EntityFactory.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameData.h"
#include "../utility/SSMail.h"

using glm::vec3;
using glm::quat;

SSEditorObjectPlacer& SSEditorObjectPlacer::GetInstance()
{
	static SSEditorObjectPlacer instance;
	return instance;
}

void SSEditorObjectPlacer::Startup()
{
	// Set up the subscriber
	m_SimInterests = MessageTypes::PLACE_PROP | MessageTypes::PLACE_RESOUCE | MessageTypes::PLACE_CONTROL_POINT;
	g_SSMail.RegisterSubscriber( this );

	// Set default tool
	m_LastSelectedTool = Tool::MoveTool;

	// Register scripts
	g_Script.Register( "OnPlacePropButtonPressed",					[&] ( IScriptEngine* ) { OnPropButtonPressed();											return 0; } );
	g_Script.Register( "OnPlaceResourceButtonPressed",				[&] ( IScriptEngine* ) { OnResouceButtonPressed();										return 0; } );
	g_Script.Register( "OnPlaceResearchStationButtonPressed",		[&] ( IScriptEngine* ) { OnResearchStationButtonPressed();								return 0; } );
	g_Script.Register( "OnPlacementCategoryOpen",					[&] ( IScriptEngine* ) { OnCategoryOpen();												return 0; } );
	g_Script.Register( "OnPlacementCategoryClose",					[&] ( IScriptEngine* ) { OnCategoryClose();												return 0; } );
	g_Script.Register( "OnPropCatalogueButtonPressed",				[&] ( IScriptEngine* ) { OnPropCatalogueButtonPressed( g_Script.PopInt() );				return 0; } );
	g_Script.Register( "OnResourceCatalogueButtonPressed",			[&] ( IScriptEngine* ) { OnResourceCatalogueButtonPressed( g_Script.PopInt() );			return 0; } );
	g_Script.Register( "OnResearchStationCatalogueButtonPressed",	[&] ( IScriptEngine* ) { OnResearchStationCatalogueButtonPressed( g_Script.PopInt() );	return 0; } );

	// Set the font
	g_GUI.UseFont( FONT_ID_LEKTON_11 );

	// Create and fetch relevant windows
	m_PlacementWindow = g_GUI.GetWindow( "PlacementWindow" );
	m_PlacementWindow->SetOnOpenScript( "OnPlacementCategoryOpen()" );
	m_PlacementWindow->SetOnCloseScript( "OnPlacementCategoryClose()" );

	m_PropPlacementWindow = g_GUI.AddWindow( "PropPlacementWindow", GUI::Rectangle( 0, 0, m_PlacementWindow->GetSize().x, m_PlacementWindow->GetSize().y ), "PlacementWindow" );
	m_PropPlacementWindow->SetToggleGroup( "PlacementTabWindows" );

	m_ResourcePlacementWindow = g_GUI.AddWindow( "ResourcePlacementWindow", GUI::Rectangle( 0, 0, m_PlacementWindow->GetSize().x, m_PlacementWindow->GetSize().y ), "PlacementWindow" );
	m_ResourcePlacementWindow->SetToggleGroup( "PlacementTabWindows" );

	m_ResearchStationPlacementWindow = g_GUI.AddWindow( "ResearchStationPlacementWindow", GUI::Rectangle( 0, 0, m_PlacementWindow->GetSize().x, m_PlacementWindow->GetSize().y ), "PlacementWindow" );
	m_ResearchStationPlacementWindow->SetToggleGroup( "PlacementTabWindows" );

	// Create placement object type tabs
	const int tabWidth	= 128;
	const int tabHeight	= 32;

	GUI::Button* propToggleButton = g_GUI.AddButton( "PropToggleButton", GUI::Rectangle( 0, 0, tabWidth, tabHeight ), "PlacementWindow" );
	propToggleButton->SetToggleGroup( "PlacementTabButtons" );
	propToggleButton->SetClickScript( "ToggleWindowGroup( 'PropPlacementWindow', 'PlacementTabWindows' ); OnPlacePropButtonPressed()" );
	propToggleButton->SetText( "Props" );
	propToggleButton->SetBackgroundImage( "Button_128x32.png" );

	GUI::Button* resourceToggleButton = g_GUI.AddButton( "ResourceToggleButton", GUI::Rectangle( tabWidth, 0, tabWidth, tabHeight ), "PlacementWindow" );
	resourceToggleButton->SetToggleGroup( "PlacementTabButtons" );
	resourceToggleButton->SetClickScript( "ToggleWindowGroup( 'ResourcePlacementWindow', 'PlacementTabWindows' ); OnPlaceResourceButtonPressed()" );
	resourceToggleButton->SetText( "Resources" );
	resourceToggleButton->SetBackgroundImage( "Button_128x32.png" );

	GUI::Button* researchStationToggleButton = g_GUI.AddButton( "ResearchStationToggleButton", GUI::Rectangle( tabWidth * 2, 0, tabWidth, tabHeight ), "PlacementWindow" );
	researchStationToggleButton->SetToggleGroup( "PlacementTabButtons" );
	researchStationToggleButton->SetClickScript( "ToggleWindowGroup( 'ResearchStationPlacementWindow', 'PlacementTabWindows' ); OnPlaceResearchStationButtonPressed()" );
	researchStationToggleButton->SetText( "Research" );
	researchStationToggleButton->SetBackgroundImage( "Button_128x32.png" );

	g_GUI.ToggleWindowGroup( "PropPlacementWindow", "PlacementTabWindows" );

	// Create catalogues //TODODB: Change this so they are no longer allocated on the heap
 	m_PropCatalogue = pNew( Catalogue, "PropCatalogue", GUI::Rectangle( 0, tabHeight, m_PropPlacementWindow->GetBoundingBoxRef().GetSize().x, m_PropPlacementWindow->GetBoundingBoxRef().GetSize().y - tabHeight ), "PropPlacementWindow", "OnPropCatalogueButtonPressed" );
	m_PropCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Rock1" );
	m_PropCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Rock2" );
	m_PropCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Rock3" );
	m_PropCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Rock4" );
	m_PropCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Rock5" );
	m_PropCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Rock6" );

	m_ResourceCatalogue = pNew( Catalogue, "ResourceCatalogue", GUI::Rectangle( 0, tabHeight, m_PropPlacementWindow->GetBoundingBoxRef().GetSize().x, m_PropPlacementWindow->GetBoundingBoxRef().GetSize().y - tabHeight ), "ResourcePlacementWindow", "OnResourceCatalogueButtonPressed" );
	m_ResourceCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "TV" );
	m_ResourceCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Box" );
	m_ResourceCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Wheel" );
	m_ResourceCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Bike" );
	m_ResourceCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Chair" );

	m_ResearchStationCatalogue = pNew( Catalogue, "ResearchStationCatalogue", GUI::Rectangle( 0, tabHeight, m_PropPlacementWindow->GetBoundingBoxRef().GetSize().x, m_PropPlacementWindow->GetBoundingBoxRef().GetSize().y - tabHeight ), "ResearchStationPlacementWindow", "OnResearchStationCatalogueButtonPressed" );
	m_ResearchStationCatalogue->AddButton( "", glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ), "Station1" );

	m_PropModels.push_back( "RockShape1.robo" );
	m_PropModels.push_back( "RockShape2.robo" );
	m_PropModels.push_back( "RockShape3.robo" );
	m_PropModels.push_back( "RockShape4.robo" );
	m_PropModels.push_back( "RockShape5.robo" );
	m_PropModels.push_back( "RockShape6.robo" );

	m_ResourceModels.push_back( "OldTVShape.robo" );
	m_ResourceModels.push_back( "CardboardBoxShape.robo" );
	m_ResourceModels.push_back( "dirtyWheelsShape.robo" );
	m_ResourceModels.push_back( "oldBicycleShape.robo" );
	m_ResourceModels.push_back( "OldChairShape.robo" );

	m_ResearchStationModels.push_back( "ResearchBottomShape.robo" );

	m_SelectedModel = "RockShape1.robo";
}

void SSEditorObjectPlacer::Shutdown()
{
	m_SimInterests = 0;
	g_SSMail.UnregisterSubscriber( this );
	pDelete( m_PropCatalogue );
	pDelete( m_ResourceCatalogue );
	pDelete( m_ResearchStationCatalogue );
}

void SSEditorObjectPlacer::UpdateUserLayer( const float deltaTime )
{
	// Place object
	if ( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
	{
		if ( g_SSEditorToolbox.IsToolSelected( Tool::PlaceProp ) )
		{
			const vec3& pickedPosition = g_SSEditorToolbox.GetPickedPosition();
			PlacePropMessage placePropMessage( g_GameData.GetFrameCount() + 1, pickedPosition.x, pickedPosition.z, vec3( 1.0f ), quat( 1.0f, 0.0f, 0.0f, 0.0f ), true, m_SelectedModel, 1, false );
			g_SSMail.PushToNextFrame( placePropMessage );
		}
		else if ( g_SSEditorToolbox.IsToolSelected( Tool::PlaceResouce ) )
		{
			const vec3& pickedPosition = g_SSEditorToolbox.GetPickedPosition();
			PlaceResourceMessage placeResourceMessage( g_GameData.GetFrameCount() + 1, pickedPosition.x, pickedPosition.z, vec3( 1.0f ), quat( 1.0f, 0.0f, 0.0f, 0.0f ), m_SelectedModel );
			g_SSMail.PushToNextFrame( placeResourceMessage );
		}
		else if ( g_SSEditorToolbox.IsToolSelected( Tool::PlaceControlPoint ) )
		{
			const vec3& pickedPosition = g_SSEditorToolbox.GetPickedPosition();
			PlaceControlPointMessage placeControlPointMessage( g_GameData.GetFrameCount() + 1, pickedPosition.x, pickedPosition.z, vec3( 1.0f ), quat( 1.0f, 0.0f, 0.0f, 0.0f ), m_SelectedModel );
			g_SSMail.PushToNextFrame( placeControlPointMessage );
		}
	}
}

void SSEditorObjectPlacer::UpdateSimLayer( const float timeStep )
{
	for ( int i = 0; i < m_SimMailbox.size(); ++i )
	{
		const Message* message = m_SimMailbox[i];
		switch ( m_SimMailbox[i]->Type )
		{
			case MessageTypes::PLACE_PROP:
			{
				const PlacePropMessage* propMessage = static_cast<const PlacePropMessage*>( message );
				EntityFactory::CreateProp( propMessage->PosX, propMessage->PosZ, propMessage->Scale, propMessage->Orientation, propMessage->BlockPath, propMessage->ModelPath, propMessage->Radius, propMessage->IsTree );
			} break;

			case MessageTypes::PLACE_RESOUCE:
			{
				const PlaceResourceMessage* resourceMessage = static_cast<const PlaceResourceMessage*>( message );
				EntityFactory::CreateResource( resourceMessage->PosX, resourceMessage->PosZ, resourceMessage->Scale, resourceMessage->Orientation, resourceMessage->ModelFileName );
			} break;

			case MessageTypes::PLACE_CONTROL_POINT:
			{
				const PlaceControlPointMessage* controlPointMessage = static_cast<const PlaceControlPointMessage*>( message );
				EntityFactory::CreateControlPoint( controlPointMessage->PosX, controlPointMessage->PosZ, controlPointMessage->Scale, controlPointMessage->Orientation, -1, controlPointMessage->ModelFileName );
			} break;

			default:
				Logger::Log( "Received unknown message type", "EditorObjectPlacer", LogSeverity::WARNING_MSG );
				break;
		}
	}
}

void SSEditorObjectPlacer::OnPropButtonPressed()
{
	g_SSEditorToolbox.SelectTool( Tool::PlaceProp );
	m_LastSelectedTool = g_SSEditorToolbox.GetSelectedTool();
}

void SSEditorObjectPlacer::OnResouceButtonPressed()
{
	g_SSEditorToolbox.SelectTool( Tool::PlaceResouce );
	m_LastSelectedTool = g_SSEditorToolbox.GetSelectedTool();
}

void SSEditorObjectPlacer::OnResearchStationButtonPressed()
{
	g_SSEditorToolbox.SelectTool( Tool::PlaceControlPoint );
	m_LastSelectedTool = g_SSEditorToolbox.GetSelectedTool();
}

void SSEditorObjectPlacer::OnCategoryOpen()
{
	g_SSEditorToolbox.SelectTool( m_LastSelectedTool );
	m_LastSelectedTool = g_SSEditorToolbox.GetSelectedTool();
}

void SSEditorObjectPlacer::OnCategoryClose()
{
	m_LastSelectedTool = g_SSEditorToolbox.GetSelectedTool();
	g_SSEditorToolbox.SelectTool( Tool::None );
}

Tool SSEditorObjectPlacer::GetLastSelectedTool()
{
	return m_LastSelectedTool;
}

void SSEditorObjectPlacer::OnPropCatalogueButtonPressed( int buttonIndex )
{
	m_SelectedModel = m_PropModels[buttonIndex];
	g_SSEditorToolbox.SetLocalPlacementGhostModel( m_PropModels[buttonIndex] );
	g_SSEditorToolbox.SelectTool( Tool::PlaceProp );
}

void SSEditorObjectPlacer::OnResourceCatalogueButtonPressed( int buttonIndex )
{
	m_SelectedModel = m_ResourceModels[buttonIndex];
	g_SSEditorToolbox.SetLocalPlacementGhostModel( m_ResourceModels[buttonIndex] );
	g_SSEditorToolbox.SelectTool( Tool::PlaceResouce );
}

void SSEditorObjectPlacer::OnResearchStationCatalogueButtonPressed( int buttonIndex )
{
	m_SelectedModel = m_ResearchStationModels[buttonIndex];
	g_SSEditorToolbox.SetLocalPlacementGhostModel( m_ResearchStationModels[buttonIndex] );
	g_SSEditorToolbox.SelectTool( Tool::PlaceControlPoint );
}