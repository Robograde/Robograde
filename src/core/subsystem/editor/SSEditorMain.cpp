#include "SSEditorMain.h"
#include <gui/GUIEngine.h>
#include "../../utility/GameModeSelector.h"
#include "../input/SSButtonInput.h"
#include "../input/SSKeyBinding.h"
#include "../audio/SSAudio.h"
#include "../network/SSNetworkController.h"
#include "../../utility/GameData.h"
#include "SSEditorToolbox.h"

#include "SSEditorObjectPlacer.h"
#include "SSEditorSFXEmitter.h"
#include "SSEditorParticle.h"
#include "SSEditorEvents.h"

SSEditorMain& SSEditorMain::GetInstance()
{
	static SSEditorMain instance;
	return instance;
}

void SSEditorMain::Startup()
{
	g_GUI.UseFont( FONT_ID_LEKTON_11 );

	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );
	GUI::Window* window = g_GUI.AddWindow( "EditorWindow", GUI::Rectangle( 0, 0, windowSize.x, windowSize.y ), "RootWindow", false );
	GUI::Window* canvasWindow = g_GUI.AddWindow( "EditorCanvasWindow", GUI::Rectangle( 0, 0, windowSize.x, windowSize.y ), "EditorWindow", false );
	window->Open();
	canvasWindow->Open();

	g_GUI.BringWindowToFront( "DebugWindow" );
	
	GUI::Window* bottomWindow = g_GUI.AddWindow( "BottomBarWindow", GUI::Rectangle( m_ToolBoxWindowSize.x, windowSize.y - m_BottomBarHeight, windowSize.x - m_ToolBoxWindowSize.x, m_BottomBarHeight ), "EditorWindow", true );
	bottomWindow->SetBackgroundColour( glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
	bottomWindow->Open();
	
	int y = 80; // Should center later or something
	auto addCategory = [&] ( const rString name, const rString& image, ACTION action, const rString& toolTip )
	{
		GUI::Window* window = g_GUI.AddWindow( name + "Window", GUI::Rectangle( 0, 0, windowSize.x - m_ToolBoxWindowSize.x, m_BottomBarHeight ), "BottomBarWindow" );
		window->SetToggleGroup( "BottomBarGroup" );

		GUI::Button* btn = g_GUI.AddButton( name + "Button", GUI::Rectangle( windowSize.x - 64, y, 64, 64 ), "EditorWindow" );
		btn->SetClickScript( "ToggleWindowGroup( '" + name + "Window', 'BottomBarGroup' )" );
		btn->SetToggleGroup( "CategoryTabButtons" );
		btn->GetBackgroundRef().BorderSize = 0;
		btn->SetHighLightColour( glm::vec4( 0.32f, 0.78f, 1.0f, 1.0f ) );
		btn->SetDisabledColour( glm::vec4( 0.0f ) );
		btn->SetBackgroundImage( "Button_64.png" );
		btn->SetImagePath( image );
		btn->SetToolTipText( toolTip );

		g_SSButtonInput.AddButtonBinding( btn, name, action, "" );

		y += 64 + 16;
	};

	// Category buttons
	g_GUI.UseFont( FONT_ID_LEKTON_8 );
	addCategory( "TerrainEdit",		"icons/TerrainEdit.png", ACTION::ACTION_SELECT_CONTROL_GROUP_1, "Edit the terrain" );
	addCategory( "Placement",		"icons/Placement.png", ACTION::ACTION_SELECT_CONTROL_GROUP_2, "Place props on the terrain" );
	addCategory( "Particles",		"icons/Particles.png",	ACTION::ACTION_SELECT_CONTROL_GROUP_3, "Create and place particle systems" );
	addCategory( "SoundEmitters",	"icons/SoundEmitter.png", ACTION::ACTION_SELECT_CONTROL_GROUP_4, "Create and place Sound emitters" );
	addCategory( "Events",			"icons/Event.png",		ACTION::ACTION_SELECT_CONTROL_GROUP_5, "Create and manage events" );
	addCategory( "CameraPaths",		"icons/Camera.png", ACTION::ACTION_SELECT_CONTROL_GROUP_6, "Create and edit camera paths" );

	g_GUI.ToggleWindowGroup( "TerrainEditWindow", "BottomBarGroup" );
	g_GUI.ToggleButtonGroup( "TerrainEditButton", "CategoryTabButtons" );

	// Toolbox
	GUI::Window* toolboxWindow = g_GUI.AddWindow( "ToolboxWindow", GUI::Rectangle( 0, windowSize.y - m_ToolBoxWindowSize.y, m_ToolBoxWindowSize.x, m_ToolBoxWindowSize.y ), "EditorWindow", true );
	toolboxWindow->SetBackgroundColour( glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
	toolboxWindow->Open();

	int x = 0;
	auto addToolboxItem = [&] ( const rString name, const rString& image, ACTION action, const rString& script, const rString& toolTip, glm::vec4& iconColour ) mutable -> GUI::Button*
	{
		GUI::Button* btn = g_GUI.AddButton( name, GUI::Rectangle( x, 0, m_ToolboxIconSize, m_ToolboxIconSize ), "ToolboxWindow" );
		btn->SetBackgroundImage( "Button_32.png" );
		btn->SetImagePath( image );
		btn->SetImageColour( iconColour );
		btn->SetHighLightColour( glm::vec4( 0.32f, 0.78f, 1.0f, 1.0f ) );
		btn->SetClickScript( script );
		btn->SetToolTipText( toolTip );

		x += m_ToolboxIconSize;

		return btn;
	};

	m_MoveButton =		addToolboxItem( "", "icons/Move.png",	ACTION::ACTION_CONTEXT_1, "GE_EditorMove()", "Move something",  glm::vec4( 1.0f ) );
	m_AddButton =		addToolboxItem( "", "icons/Add.png",	ACTION::ACTION_CONTEXT_1, "GE_EditorAdd()", "Add something", glm::vec4( 0.0f, 1.0f, 0.2f, 1.0f ) );
	m_RemoveButton =	addToolboxItem( "", "icons/Delete.png", ACTION::ACTION_CONTEXT_1, "GE_EditorRemove()", "", glm::vec4( 1.0f, 0.0f, 0.2f, 1.0f ) );
	m_UndoButton =		addToolboxItem( "", "icons/Undo.png",	ACTION::ACTION_CONTEXT_1, "", "", glm::vec4( 1.0f ) );
	m_RedoButton =		addToolboxItem( "", "icons/Redo.png",	ACTION::ACTION_CONTEXT_1, "", "", glm::vec4( 1.0f ) );
	m_NoteButton =		addToolboxItem( "", "icons/Note.png",	ACTION::ACTION_CONTEXT_1, "", "", glm::vec4( 1.0f ) );
	m_SaveButton =		addToolboxItem( "", "icons/Save.png",	ACTION::ACTION_CONTEXT_1, "", "", glm::vec4( 1.0f ) );

	g_Script.Register( "GE_EditorMove", std::bind( &SSEditorMain::EditorMove, this, std::placeholders::_1 )  );
	g_Script.Register( "GE_EditorAdd", std::bind( &SSEditorMain::EditorAdd, this, std::placeholders::_1 )  );
	g_Script.Register( "GE_EditorRemove", std::bind( &SSEditorMain::EditorRemove, this, std::placeholders::_1 ) );

	// ParticipantWindow
	{
		g_GUI.UseFont( FONT_ID_LEKTON_20 );

		int offsetY = 4;
		int offsetX = 4;
		// Create the window
		glm::ivec2 participantBoxSize = glm::ivec2( 32, 32 );
		const rMap<short, NetworkPlayer>& participants = g_SSNetworkController.GetNetworkedPlayers();
		GUI::Window* participantsWindow = g_GUI.AddWindow( "ParticipantsWindow", GUI::Rectangle( 0, 0, offsetX + (participantBoxSize.x + offsetX ) * static_cast<int>( participants.size() ), participantBoxSize.y + offsetY * 2 ), "EditorWindow", true );
		participantsWindow->SetBackgroundColour( glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
		participantsWindow->Open();

		// Create a button for each user
		int x = offsetX;
		int participantIndex = 0;
		for ( auto participant : participants )
		{
			GUI::Button* participantButton = g_GUI.AddButton( "Participant" + rToString( participantIndex ) + "Button", GUI::Rectangle( x, offsetY, participantBoxSize.x, participantBoxSize.y ), "ParticipantsWindow" );
			participantButton->SetText( participant.second.Name.substr( 0, 1 ) ); // TODODB: Up the font size here so that the initial letter is readable
			participantButton->SetColour( g_GameData.GetPlayerColour( participant.first ) );
			participantButton->SetToolTipDelay( 0.0f );
			participantButton->SetToolTipText( participant.second.Name );
			// TODODB: Set clickscript to a script that moves the players camera position to the clicked players camera position

			x += participantBoxSize.x + offsetX;
			++participantIndex;
		}
	}

	// Dont play music in editor-mode
	MusicEvent musicEvent;
	musicEvent.Type = MusicEventType::STOP_ALL;
	g_SSAudio.PostEventMusic(musicEvent);
}

void SSEditorMain::UpdateUserLayer( const float deltaTime )
{
	if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
	{
		g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
	}

	for ( auto& action : *g_SSKeyBinding.GetActionToActionIdentifier( ) )
	{
		if ( g_SSKeyBinding.ActionUpDown( action ) ) // Check if the key has been triggered // TODODB: Add support for other checks than keyUpDown
		{
			g_SSButtonInput.HandleAction( action );
		}
	}
}

void SSEditorMain::Shutdown()
{
	g_GUI.DeleteObject( "EditorWindow" );
	g_GUI.ClearWindowGroup( "BottomBarGroup" );
	g_GUI.ClearButtonGroup( "CategoryTabButtons" );
}

int SSEditorMain::EditorMove( IScriptEngine* scriptEngine )
{
	g_SSEditorToolbox.SelectTool( Tool::MoveTool );
	return 0;
}

int SSEditorMain::EditorAdd( IScriptEngine* scriptEngine )
{
	if( g_GUI.IsWindowOpen( "TerrainEditWindow" ) )
		g_SSEditorToolbox.SelectTool( Tool::None );
	else if ( g_GUI.IsWindowOpen( "PlacementWindow" ) )
		g_SSEditorToolbox.SelectTool( g_SSEditorObjectPlacer.GetLastSelectedTool() );
	else if ( g_GUI.IsWindowOpen( "ParticlesWindow" ) )
		g_SSEditorToolbox.SelectTool( Tool::None ); //Particle placement tool here later
	else if ( g_GUI.IsWindowOpen( "SoundEmittersWindow" ) )
		g_SSEditorToolbox.SelectTool( Tool::PlaceSFXEmitter );
	else if ( g_GUI.IsWindowOpen( "EventsWindow" ) )
		g_SSEditorToolbox.SelectTool( Tool::None );
	else if ( g_GUI.IsWindowOpen( "CameraPathsWindow" ) )
		g_SSEditorToolbox.SelectTool( Tool::None );
	return 0;
}

int SSEditorMain::EditorRemove( IScriptEngine* scriptEngine )
{

	return 0;
}