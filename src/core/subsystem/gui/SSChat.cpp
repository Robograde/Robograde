/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSChat.h"
#include <cctype>
#include <messaging/GameMessages.h>
#include "../utility/SSMail.h"
#include "../network/SSNetworkController.h"
#include <input/Input.h>
#include <input/KeyBindings.h>
#include "../../utility/GameData.h"
#include "../../utility/Alliances.h"
#include "../input/SSKeyBinding.h"

SSChat& SSChat::GetInstance()
{
	static SSChat instance;
	return instance;
}

void SSChat::Startup()
{
	g_SSMail.RegisterSubscriber( this );
	m_UserInterests = MessageTypes::CHAT;
	m_Messages.clear();

	if( m_ChatState != CHATSTATE_LOBBY )
	{
		int height = 256 + 128;
		int width = 384;
		InitializeChatWindow( 0, g_GameData.GetWindowHeight() - 256 - 32 - height, width, height, 40, CHATSTATE::CHATSTATE_INGAME );
	}
}

void SSChat::InitializeChatWindow( const int x, const int y, const int width, const int height, const int maxChars, CHATSTATE chatstate )
{
	g_GUI.DeleteObject( "Chat" );
	m_Texts.clear();
	m_Messages.clear();
	m_Window = g_GUI.AddWindow( "Chat", GUI::Rectangle( x, y, width, height ), "RootWindow", true );
	m_Window->SetPosition( x, y );
	m_Window->SetSize( width, height );
	m_Window->SetClickThrough( true );
	m_Window->Open();

	g_GUI.UseFont( FONT_ID_LEKTON_11 );

	int amount = ( height - m_TextBoxSize.y ) / 16;
	int yPos = height - m_TextBoxSize.y - 16;
	for( int i = 0; i < amount; i++ )
	{
		m_Texts.push_back( g_GUI.AddText( "", GUI::TextDefinition( "Test", 5, yPos, width, m_TextSize.y ), "Chat" ) );
		yPos -= m_TextSize.y;
	}

	g_GUI.UseFont( FONT_ID_LEKTON_11 );

	int offsetX = 0;
	if( chatstate != CHATSTATE_LOBBY )
	{
		offsetX = 64;
		m_Target = g_GUI.AddText( "", GUI::TextDefinition( "ALL", 0, height - m_TextBoxSize.y, offsetX, m_TextBoxSize.y ), "Chat" );
		m_Target->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
	}
	else
	{
		offsetX = 48;
		g_GUI.UseFont( FONT_ID_LEKTON_16 );
		m_LobbyTitle = g_GUI.AddText( "", GUI::TextDefinition( "Chat", 0, height - m_TextBoxSize.y, offsetX, m_TextBoxSize.y ), "Chat" );
		m_LobbyTitle->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_LEFT );
		g_GUI.UseFont( FONT_ID_LEKTON_11 );
		m_ChatTarget = CHAT_TARGET_ALL;
	}

	m_TextBox = g_GUI.AddTextBox( "", GUI::Rectangle( offsetX, height - m_TextBoxSize.y, width - offsetX, m_TextBoxSize.y ), "Chat" );

	m_ChatState = chatstate;

	if( m_ChatState == CHATSTATE_LOBBY )
		m_KeepOpen = true;
	else
		m_KeepOpen = false;

	if( m_ChatState == CHATSTATE_INGAME_NOINPUT )
	{
		m_TextBox->SetEnabled( false );
	}

	m_IsInitialized = true;

	m_MaxChars = maxChars;
}

void SSChat::UpdateUserLayer( const float deltaTime )
{
	for ( int i = 0; i < m_UserMailbox.size(); ++i )
	{
		if( !g_NetworkInfo.AmIHost() && !m_UserMailbox[i]->CreatedFromPacket ) 
			continue;

		const ChatMessage* chatMessage = static_cast<const ChatMessage*>(m_UserMailbox[i]);

		rString name = g_SSNetworkController.GetPlayerName( chatMessage->SenderID );

		if( chatMessage->TargetID == g_PlayerData.GetPlayerID()/* || chatMessage->SenderID == g_PlayerData.GetPlayerID()*/ || chatMessage->TargetID == -1 )
		{
			rString messageColour;
			if( chatMessage->TargetID == -1 )
				messageColour = "[C=WHITE]";
			else
				messageColour = "[C=GREEN]";

			rString messageText = chatMessage->MessageText;
			
			int nameSize = static_cast<int>( name.size() ) + 2;
			rString nameSpacing;
			nameSpacing.resize( nameSize, ' ' );

			rVector<rString> messageParts;
			int lineCount = 0;
			while( messageText.size() > m_MaxChars )
			{
				if( lineCount == 0)
					messageParts.push_back( messageText.substr( 0, m_MaxChars ) );
				else
					messageParts.push_back( nameSpacing + messageText.substr( 0, m_MaxChars ) );

				messageText = messageText.substr( m_MaxChars, messageText.size() - m_MaxChars );
				lineCount++;
			}

			if( messageParts.size() > 0 )
			{
				messageParts.push_back( nameSpacing +  messageText.substr( 0, m_MaxChars ) );
				messageText = messageParts[0];
			}


			rString message = "[C=BLUE]" + name + "[C=WHITE]: " + messageColour + messageText;


			m_Messages.push_back( message );


			for( int i = 1; i < messageParts.size(); i++ )
				m_Messages.push_back( messageColour + messageParts[i] );
			
			if( !m_TextBox->IsInputting() )
			{
				m_CurrentFadeTime = m_RecieveFadeTime;
				m_RecievedFade = true;
			}
		}
		else
		{
			//Recieved a chat message not intended for you! :o
		}
	}

	
	if( !m_IsInitialized )
		return;

	if( m_TextBox->IsInputting() )
	{
		if( g_Input->KeyUpDown( SDL_SCANCODE_RETURN, true ) || g_Input->KeyDownUp( SDL_SCANCODE_KP_ENTER, true ) )
		{
			if( m_TextBox->GetText() != "" )
			{
				rString& text = m_TextBox->GetText();
				int pos1 = static_cast<int>( text.find( '/' ) );
				int pos2 = static_cast<int>( text.find( ' ' ) );

				tString messageText = "";
				if( pos1 != tString::npos && pos2 != tString::npos )
				{
					if( m_ChatState != CHATSTATE_LOBBY )
					{
						tString target = "";
						target = text.substr( pos1 + 1, pos2 - (pos1+1) ).c_str();
						std::transform( target.begin(), target.end(), target.begin(), ::tolower );
						if( target == "all" )
							m_ChatTarget = CHAT_TARGET_ALL;
						else if( target == "allied" )
							m_ChatTarget = CHAT_TARGET_ALLIED;

						messageText = text.substr( pos2 + 1, text.size() - pos2 ).c_str();
					}
				}
				else
					messageText = m_TextBox->GetText().c_str();

				if( m_ChatTarget == CHAT_TARGET_ALLIED )
				{
					for( int ally : g_Alliances.GetAllies( g_PlayerData.GetPlayerID() ) )
					{
						ChatMessage message = ChatMessage( messageText.c_str(), g_PlayerData.GetPlayerID(), ally );
						g_SSMail.PushToUserLayer( message ); 
					}
				}
				else //ALL
				{
					ChatMessage message = ChatMessage( messageText.c_str(), g_PlayerData.GetPlayerID() );
					g_SSMail.PushToUserLayer( message ); 
				}
			}

			m_TextBox->SetText( "" );
			
			if( m_ChatState != CHATSTATE_LOBBY ) 
			{
				m_TextBox->StopInput();
				m_TextBox->SetEnabled( false );
				m_CurrentFadeTime = m_FadeTime;
				m_KeepOpen = false;
				m_RecievedFade = false;
			}
		}
	}
	else
	{
		if( g_SSKeyBinding.ActionUpDown( ACTION_OPEN_CHAT ) )
		{
			if( m_ChatState != CHATSTATE_LOBBY )
			{
				if( g_Alliances.GetAllies( g_PlayerData.GetPlayerID() ).size() > 1 )
				{
					if( g_Input->KeyDown( SDL_SCANCODE_LSHIFT ) || g_Input->KeyDown( SDL_SCANCODE_RSHIFT ) )
						m_ChatTarget = CHAT_TARGET_ALL;
					else
						m_ChatTarget = CHAT_TARGET_ALLIED;
				}
				else
					m_ChatTarget = CHAT_TARGET_ALL;
			}
			else
				m_ChatTarget = CHAT_TARGET_ALL;

			m_TextBox->StartInput();
			m_KeepOpen = true;
			m_RecievedFade = false;
			if( m_ChatState != CHATSTATE_LOBBY )
				m_TextBox->SetEnabled( true );
		}
		
	}
	ManageFading( deltaTime );

	for( int i = 0; i < m_Texts.size(); i++ )
	{
		if( m_Messages.size() > i )
		{
			m_Texts[i]->SetText( m_Messages[m_Messages.size() - 1 - i] );
		}
		else
		{
			m_Texts[i]->SetText( "" );
		}
	}

	if( m_ChatState != CHATSTATE_LOBBY )
	{
		if( m_ChatTarget == CHAT_TARGET_ALL )
		{
			m_Target->SetText( "ALL " );
			m_Target->GetTextDefinitionRef().Colour = glm::vec4( 1.0f );
		}
		else if( m_ChatTarget == CHAT_TARGET_ALLIED )
		{
			m_Target->SetText( "ALLIED " );
			m_Target->GetTextDefinitionRef().Colour = glm::vec4( 0.0f, 0.4f, 0.8f, 1.0f );
		}
	}
}

void SSChat::Shutdown()
{
	g_SSMail.UnregisterSubscriber( this );
	g_GUI.DeleteObject( "Chat" );
	m_Texts.clear();
}

void SSChat::ManageFading( const float deltaTime )
{
	float opacity = 1.0f;
	if( !m_KeepOpen )
	{
		
		if( m_CurrentFadeTime > 0 )
		{
			if( m_RecievedFade )
				opacity = m_CurrentFadeTime / m_RecieveFadeTime;
			else
				opacity = m_CurrentFadeTime / m_FadeTime;

			m_CurrentFadeTime -= deltaTime;
		}
		else
		{
			opacity = 0.0f;
			m_RecievedFade = false;
		}
	}

	for( GUI::Text* text : m_Texts )
	{
		text->SetOpacity( opacity );
	}

	if( m_RecievedFade && m_ChatState !=  CHATSTATE_LOBBY )
	{
		m_Window->SetOpacity( 0 );
		m_TextBox->SetOpacity( 0 );
		m_Target->SetOpacity( 0 );
	}
	else
	{
		m_Window->SetOpacity( opacity );
		m_TextBox->SetOpacity( opacity );

		if( m_ChatState != CHATSTATE_LOBBY )
			m_Target->SetOpacity( opacity );
	}

		
	
}

 void SSChat::SetChatState( CHATSTATE chatState )
 {
	 m_ChatState = chatState;
 }
