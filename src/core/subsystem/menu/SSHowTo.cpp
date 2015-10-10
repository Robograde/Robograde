/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSHowTo.h"
#include <gfx/Texture.h>
#include "../input/SSKeyBinding.h"
#include "../../utility/GameModeSelector.h"

SSHowTo& SSHowTo::GetInstance()
{
	static SSHowTo instance;
	return instance;
}

void SSHowTo::Startup()
{
	g_GUI.BringWindowToFront( "DebugWindow" );

	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );
	g_GUI.AddWindow( m_WindowName, GUI::Rectangle( 0, 0, windowSize.x, windowSize.y ), "RootWindow" );
	//g_GUI.OpenWindow( m_WindowName );

	g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, windowSize.x, windowSize.y, glm::vec4( 0.05f, 0.05f, 0.1f, 1.0f ) ), m_WindowName );

	LoadTextures();

	m_Image = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", ( windowSize.x / 2 ) - (m_BackgroundSize.x / 2), ( windowSize.y / 2 ) - (m_BackgroundSize.y / 2), 
		m_BackgroundSize.x, m_BackgroundSize.y ), m_WindowName );
	
	if( m_Textures.size() > 0 )
		m_Image->GetSpriteDefinitionRef().TextureHandle = m_Textures[0]->GetHandle();

	g_GUI.UseFont( FONT_ID_LEKTON_20 );
	GUI::Button* back = g_GUI.AddButton( "", GUI::Rectangle( 5, windowSize.y - 64 - 5, 256, 64 ), m_WindowName );
	
	m_ButtonPrevious = g_GUI.AddButton( "", GUI::Rectangle( (windowSize.x / 2) - 256, windowSize.y - 64 - 5, 256, 64 ), m_WindowName );
	m_ButtonNext = g_GUI.AddButton( "", GUI::Rectangle( (windowSize.x / 2) , windowSize.y - 64 - 5, 256, 64 ), m_WindowName );

	back->SetClickScript( "GE_BackFromHowTo()" );
	m_ButtonPrevious->SetClickScript( "GE_PreviousImage()" );
	m_ButtonNext->SetClickScript( "GE_NextImage()" );

	back->SetText( "Back");
	m_ButtonPrevious->SetText( "Previous");
	m_ButtonNext->SetText( "Next");

	back->SetBackgroundImage( "Button.png" );
	m_ButtonPrevious->SetBackgroundImage( "Button.png" );
	m_ButtonNext->SetBackgroundImage( "Button.png" );

	
	//Script functions that are to be called by buttons
	g_Script.Register( "GE_BackFromHowTo", [this] ( IScriptEngine* ) -> int 
	{
		//g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
		g_Script.Perform( "CloseWindow('HowTo'); OpenWindow('MainMenu')" );

		return 0; 
	} );
	g_Script.Register( "GE_PreviousImage", [this] ( IScriptEngine* ) -> int 
	{ 
		m_HowToIndex--;
		if( m_HowToIndex < 0 )
			m_HowToIndex = 0;
		return 0; 
	
	} );

	g_Script.Register( "GE_NextImage", [this] ( IScriptEngine* ) -> int 
	{ 
		m_HowToIndex++;
		if( m_HowToIndex >= m_Textures.size() )
			m_HowToIndex = static_cast<int>( m_Textures.size() ) - 1;
		return 0; 
	
	} );
	
	
}

void SSHowTo::Shutdown()
{
	g_GUI.DeleteObject( m_WindowName );

	for( gfx::Texture* texture : m_Textures )
		tDelete( texture );
	m_Textures.clear();
}

void SSHowTo::UpdateUserLayer( const float deltaTime )
{
	if ( g_GUI.IsWindowOpen( m_WindowName ) )
	{
		m_Image->GetSpriteDefinitionRef().TextureHandle = m_Textures[m_HowToIndex]->GetHandle();

		m_ButtonPrevious->SetEnabled( m_HowToIndex != 0 );
		m_ButtonNext->SetEnabled( static_cast<size_t>(m_HowToIndex) != m_Textures.size() - 1 );

		if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
		{
			g_Script.Perform( "GE_BackFromHowTo()" );
			//g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
		}
	}
}

void SSHowTo::LoadTextures()
{
	const rString path = "../../../asset/gui/tutorial/";
	gfx::Texture* texture;
	
	texture = tNew( gfx::Texture );
	texture->Init( ( path + "tutorial0.png" ).c_str(), gfx::TEXTURE_2D );
	m_Textures.push_back( texture );

	texture = tNew( gfx::Texture );
	texture->Init( ( path + "tutorial1.png" ).c_str(), gfx::TEXTURE_2D );
	m_Textures.push_back( texture );

	texture = tNew( gfx::Texture );
	texture->Init( ( path + "tutorial2.png" ).c_str(), gfx::TEXTURE_2D );
	m_Textures.push_back( texture );

	texture = tNew( gfx::Texture );
	texture->Init( ( path + "tutorial3.png" ).c_str(), gfx::TEXTURE_2D );
	m_Textures.push_back( texture );

	texture = tNew( gfx::Texture );
	texture->Init( ( path + "tutorial4.png" ).c_str(), gfx::TEXTURE_2D );
	m_Textures.push_back( texture );
}
