/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "LoadingScreen.h"
#include <gfx/GraphicsEngine.h>
#include "GameData.h"

LoadingScreen& LoadingScreen::GetInstance()
{
	static LoadingScreen instance;
	return instance;
}

LoadingScreen::LoadingScreen()
{
}

void LoadingScreen::Render( const rString& name, int loaded, int total )
{
	glm::ivec2 winSize = glm::ivec2( g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight() );

	GUI::SpriteDefinition background = GUI::SpriteDefinition( "", 0, 0, winSize.x, winSize.y, glm::vec4( 0.0f, 0.0f, 0.1f, 1.0f ) );
	GUI::SpriteDefinition image = GUI::SpriteDefinition( "Menu_bg.png", (winSize.x / 2) - 256, (winSize.y / 2) - 256, 512, 512 );
	glm::ivec2 progressBarSize = glm::ivec2(  winSize.x - 64, m_ProgressBarHeight );

	//-- Progressbar start
	float progressbarWidthMult = loaded / (float)total;
	glm::ivec2 progressBarPos = glm::ivec2( (winSize.x / 2) - (progressBarSize.x / 2), winSize.y - (progressBarSize.y * 2) );
	GUI::SpriteDefinition progressBarBack = GUI::SpriteDefinition( "Button_256x32.png", progressBarPos.x, progressBarPos.y, 
		progressBarSize.x, progressBarSize.y, 
		glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

	GUI::SpriteDefinition progressBarFront = GUI::SpriteDefinition( "",  progressBarPos.x + 1, progressBarPos.y + 1, 
		static_cast<int>( (progressBarSize.x - 2 ) * progressbarWidthMult ), progressBarSize.y - 2, 
		glm::vec4( 0.0f, 0.9f, 0.1f, 0.3f ) );
	//-- Progressbar end

	rString text = name;
	GUI::TextDefinition loadingtext = GUI::TextDefinition( text.c_str(), progressBarPos.x, progressBarPos.y, progressBarSize.x, progressBarSize.y );
	loadingtext.Alignment = GUI::ALIGNMENT_MIDDLE_CENTER;

	//-- Render
	g_GUI.EnqueueSprite( &background );
	g_GUI.EnqueueSprite( &image );
	g_GUI.EnqueueSprite( &progressBarBack );
	g_GUI.EnqueueSprite( &progressBarFront );
	g_GUI.EnqueueText( &loadingtext );

	g_GUI.RenderNoGUI( winSize.x, winSize.y );
	gfx::g_GFXEngine.Swap();
}