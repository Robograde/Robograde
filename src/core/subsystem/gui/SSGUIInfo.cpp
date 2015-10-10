/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSGUIInfo.h"

SSGUIInfo& SSGUIInfo::GetInstance()
{
	static SSGUIInfo instance;
	return instance;
}

void SSGUIInfo::Startup()
{
	glm::ivec2 winSize = g_GUI.GetWindowSize( "InGameWindow" );
	
	m_InfoWindow1 = g_GUI.AddWindow( "InfoWindow1", GUI::Rectangle( 0, winSize.y - 128 - m_InfoWindow1Size.y, m_InfoWindow1Size.x, m_InfoWindow1Size.y ), "InGameWindow", true );
	m_InfoWindow2 = g_GUI.AddWindow( "InfoWindow2", GUI::Rectangle( winSize.x - 256 - m_InfoWindow2Size.x, winSize.y - 48 - 48 - 32 - m_InfoWindow2Size.y , m_InfoWindow2Size.x, m_InfoWindow2Size.y ), "InGameWindow", true );
	
	m_InfoWindow1->SetBackgroundColour( glm::vec4( 0.0f, 0.0f, 0.0f, 0.8f ) );
	m_InfoWindow2->SetBackgroundColour( glm::vec4( 0.0f, 0.0f, 0.0f, 0.8f ) );

	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	m_InfoText1 = g_GUI.AddText( "", GUI::TextDefinition( "Temp text", 5, 0 ), "InfoWindow1" );
	m_InfoText1->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_LEFT );
	m_InfoText1->GetTextDefinitionRef().LineSpacing = GUI::LINESPACING_HALF;
	
	m_InfoText2 = g_GUI.AddText( "", GUI::TextDefinition( "Temp text", 5, 0 ), "InfoWindow2" );
	m_InfoText2->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_LEFT );
	m_InfoText2->GetTextDefinitionRef().LineSpacing = GUI::LINESPACING_HALF;
	
	g_Script.Register( "GE_SetInfoText", [this](IScriptEngine* scriptEngine) -> int 
	{
		int  pos = scriptEngine->PopInt();
		rString window = scriptEngine->PopString();
		
		if( window == "InfoWindow1" )
		{
			m_InfoText1->SetText( scriptEngine->PopString() );
			m_InfoWindow1->Open();
		} else if ( window == "InfoWindow2" )
		{
			m_InfoText2->SetText( scriptEngine->PopString() );
			m_InfoWindow2->Open();
			m_InfoWindow2->SetPosition( m_InfoWindow2->GetBoundingBoxRef().Origin.x + pos, m_InfoWindow2->GetBoundingBoxRef().Y );
		}
		else //Should not happen
		{
			m_InfoText1->SetText( scriptEngine->PopString() );
			m_InfoWindow1->Open();
		}
		
		m_TimeLeft = m_TimeOut;
		
		m_WantToOpen = true;
		
		return 0; 
	} );
	
	g_Script.Register( "GE_TryCloseInfo", [this](IScriptEngine* scriptEngine) -> int 
	{ 
		m_WantToClose = true;
		m_TimeLeft = m_TimeOut;
		return 0; 
	} );


	int posX = 0;
	int posY = winSize.y - 256;
	int shadowDist = 2;

	m_PlayerGenericHepWindow = g_GUI.AddWindow( "PlayerGenericHelp", GUI::Rectangle( 0, posY, winSize.x, 32 ), "InGameWindow", true );

	g_GUI.UseFont( FONT_ID_LEKTON_14 );
	m_PlayerGenericHelpTextShadow = g_GUI.AddText( "PlayerGenericHelpTextShadow", GUI::TextDefinition( "", shadowDist, shadowDist, glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ), "PlayerGenericHelp" );
	m_PlayerGenericHelpText = g_GUI.AddText( "PlayerGenericHelpText", GUI::TextDefinition( "", 0, 0, glm::vec4( 1.0f ) ), "PlayerGenericHelp" );

	m_PlayerGenericHelpTextShadow->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	m_PlayerGenericHelpText->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	
}

void SSGUIInfo::Shutdown()
{
	g_GUI.DeleteObject( "InfoWindow1" );
	g_GUI.DeleteObject( "InfoWindow2" );
	g_GUI.DeleteObject( "PlayerGenericHelp" );
}

void SSGUIInfo::UpdateUserLayer( const float deltaTime )
{
	if( m_DisplayHelpMsg )
	{
		m_MsgTimerTimer += deltaTime;
		m_PlayerGenericHepWindow->GetBackgroundRef().Colour.a = 1.0f - m_MsgTimerTimer / 5.0f;
		m_PlayerGenericHelpTextShadow->GetTextDefinitionRef().Colour.a = 1.0f - m_MsgTimerTimer / 5.0f;
		m_PlayerGenericHelpText->GetTextDefinitionRef().Colour.a = 1.0f - m_MsgTimerTimer / 5.0f;
		if( m_MsgTimerTimer > m_MsgTimerTotalTime )
		{
			m_DisplayHelpMsg = false;
			m_PlayerGenericHepWindow->Close();
			
		}
	}


	if( m_WantToOpen )
		m_WantToClose = false;
		
	if( m_TimeLeft > 0.0f )
	{
		m_TimeLeft -= deltaTime;
	}
	else if( m_WantToClose )
	{
		m_InfoWindow1->Close();
		m_InfoWindow2->Close();
	}
	
	m_WantToOpen = false;
}

void SSGUIInfo::DisplayMessage( const rString& text, float time )
{
	m_PlayerGenericHelpText->SetText( text );
	m_PlayerGenericHelpTextShadow->SetText( text );
	m_DisplayHelpMsg = true;
	m_MsgTimerTimer = 0;
	m_MsgTimerTotalTime = time;
	m_PlayerGenericHepWindow->Open();
}