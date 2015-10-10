/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSReplaySelectMenu.h"
#include <glm/glm.hpp>
#include <utility/FileUtility.h>
#include "../replay/SSReplayPlayer.h"
#include "../input/SSKeyBinding.h"
#include "../../utility/GameModeSelector.h"

SSReplaySelectMenu& SSReplaySelectMenu::GetInstance()
{
	static SSReplaySelectMenu instance;
	return instance;
}

void SSReplaySelectMenu::Startup()
{
	g_GUI.UseFont( FONT_ID_LEKTON_16 );

	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );

	g_GUI.AddWindow( m_ReplaySelectWindowName, GUI::Rectangle( 0, 0, windowSize.x, windowSize.y ), "RootWindow" );

	// Lambda to make sure buttons all get the same design.
	auto setButtonDesign = [] ( GUI::Button* btn )
	{
		btn->SetBackgroundImage( "Button.png" );
		btn->SetColour( glm::vec4( 1.0f ) );
		btn->SetHighLightColour( glm::vec4( 0.8f, 1.0f, 1.0f, 1.0f ) );
		btn->GetBackgroundRef().BorderSize = 0;
	};

	int btnWidth	= 256;
	int btnHeight	= 64;

	GUI::Button* backBtn = g_GUI.AddButton( "", GUI::Rectangle( 5, windowSize.y - btnHeight - 5, btnWidth, btnHeight ), m_ReplaySelectWindowName );
	backBtn->SetText( "Back" );
	backBtn->SetClickScript( "OpenWindow('MainMenu'); CloseWindow( 'ReplaySelectWindow' )" );
	setButtonDesign( backBtn );

	auto startReplay =[&] (IScriptEngine*) -> int
	{
		g_SSReplayPlayer.SetReplayIndex( m_ReplaySelector->GetNumItems() - 1 - m_ReplaySelector->GetSelectedIndex() );
		g_GameModeSelector.SwitchToGameMode( GameModeType::Replay );
		return 0;
	};
	g_Script.Register( "GE_StartReplay", startReplay );

	GUI::Button* startButton = g_GUI.AddButton( "", GUI::Rectangle( windowSize.x - 5 - btnWidth, windowSize.y - btnHeight - 5, btnWidth, btnHeight ), m_ReplaySelectWindowName );
	startButton->SetText( "Start" );
	startButton->SetClickScript( "GE_StartReplay();" );
	setButtonDesign( startButton );

	int comboBoxWidth	= 256;
	int comboBoxHeight	= 32;

	m_ReplaySelector = g_GUI.AddComboBox( "", GUI::Rectangle( windowSize.x / 2 - comboBoxWidth / 2, 64, comboBoxWidth, comboBoxHeight ), m_ReplaySelectWindowName );
	m_ReplaySelector->SetBackgroundImage( "Button_128x32.png" );
	m_ReplaySelector->SetItemBackgroundImage( "Button_128x32.png" );

	rVector<rString> replayFileContent;
	FileUtility::GetListOfContentInDirectory( m_ReplayFolderPath.c_str(), replayFileContent );
	
	//TODODB: Protect against crap in the folder

	rVector<int> replayIndices;
	for ( int i = 0; i < replayFileContent.size(); ++i )
	{
		size_t first = replayFileContent[i].find_first_of( "0123456789" );
		size_t last = replayFileContent[i].find_last_of( '.' );

		replayIndices.push_back( std::stoi( replayFileContent[i].substr( first, last ).c_str() ) );
	}
	std::sort( replayIndices.begin(), replayIndices.end() );

	for ( int i = static_cast<int>( replayIndices.size() ) - 1; i >= 0; --i )
		m_ReplaySelector->AddItem( "Replay " + rToString( replayIndices[i] ) );
}

void SSReplaySelectMenu::UpdateUserLayer( float deltaTime )
{
	if ( g_GUI.IsWindowOpen( m_ReplaySelectWindowName ) && g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
	{ 
		g_Script.Perform( "OpenWindow( 'MainMenu' ); CloseWindow( 'ReplaySelectWindow' )" );
	}
}

void SSReplaySelectMenu::Shutdown()
{
	g_GUI.DeleteObject( m_ReplaySelectWindowName );
}

bool SSReplaySelectMenu::HasReplays() const
{
	return m_ReplaySelector->GetNumItems() > 0;
}