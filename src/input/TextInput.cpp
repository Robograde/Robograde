/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "TextInput.h"
#include <SDL2/SDL_clipboard.h>

TextInput& TextInput::GetInstance()
{
	static TextInput textInput;
	return textInput;
}

void TextInput::Initialize()
{
	// Default to not start entering text
	SDL_StopTextInput();
	
	m_InputStrings.reserve( 20 );
	m_CurrentInputString = -1;
}

void TextInput::HandleEvents( const SDL_Event& event )
{
	switch ( event.type )
	{
		case SDL_KEYDOWN:
			if ( event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE )
			{
				if( m_CurrentInputString != -1 )
				{
					if ( m_InputStrings[m_CurrentInputString].size( ) > 0 )
					{
						//m_InputStrings[m_CurrentInputString].pop_back( );
						if ( m_TextCursors[m_CurrentInputString] > 0 )
						{
							m_InputStrings[m_CurrentInputString] = m_InputStrings[m_CurrentInputString].erase( m_TextCursors[m_CurrentInputString] - 1, 1 );
							--m_TextCursors[m_CurrentInputString];
						}
					}
				}
			}
			if ( event.key.keysym.scancode == SDL_SCANCODE_LEFT )
			{
				if ( m_CurrentInputString != -1 )
					MoveCursor( m_CurrentInputString, -1 );
			}
			if ( event.key.keysym.scancode == SDL_SCANCODE_RIGHT )
			{
				if ( m_CurrentInputString != -1 )
					MoveCursor( m_CurrentInputString, 1 );
			}
			//TODOIA check if needed
//			if ( event.key.keysym.scancode == SDL_SCANCODE_RETURN  ) {
//				StopInput();
//			}
//			if ( event.key.keysym.scancode == SDL_SCANCODE_GRAVE  ) {
//				StopInput();
//			}
			//Paste
			if ( event.key.keysym.scancode == SDL_SCANCODE_V && SDL_GetModState( ) & KMOD_CTRL ) 
				if ( m_CurrentInputString != -1 )
				{
					//m_InputStrings[m_CurrentInputString] += SDL_GetClipboardText( );

					size_t strSize = m_InputStrings[m_CurrentInputString].size( );

					m_InputStrings[m_CurrentInputString].insert( m_TextCursors[m_CurrentInputString], SDL_GetClipboardText( ) );

					MoveCursor( m_CurrentInputString, (int)( m_InputStrings[m_CurrentInputString].size( ) - strSize ) );
				}
			//Copy
			if( event.key.keysym.scancode == SDL_SCANCODE_C && SDL_GetModState() & KMOD_CTRL )
				if(m_CurrentInputString != -1 )
					SDL_SetClipboardText( m_InputStrings[m_CurrentInputString].c_str() );
			//Cut
			if( event.key.keysym.scancode == SDL_SCANCODE_X && SDL_GetModState() & KMOD_CTRL )
			{
				if(m_CurrentInputString != -1 )
				{
					SDL_SetClipboardText( m_InputStrings[m_CurrentInputString].c_str() );
					m_InputStrings[m_CurrentInputString] = "";
					m_TextCursors[m_CurrentInputString] = 0;
				}
			}
			
			break;
		case SDL_TEXTINPUT:
			if ( m_CurrentInputString != -1 )
			{
				//m_InputStrings[m_CurrentInputString] += event.text.text;

				m_InputStrings[m_CurrentInputString].insert( m_TextCursors[m_CurrentInputString], event.text.text );

				++m_TextCursors[m_CurrentInputString];
			}
			
			//m_InpurString = m_InpurString.substr( 0, m_TextCursorIndex ) + event.text.text + m_InpurString.substr( m_TextCursorIndex, m_InpurString.size() - m_TextCursorIndex ) ;
			
			break;
		case SDL_TEXTEDITING:
			m_Composition = event.edit.text;
			
			if ( m_CurrentInputString != -1 )
			{
				m_InputStrings[m_CurrentInputString] += event.edit.text;
				m_TextCursors[m_CurrentInputString] = event.edit.start;
			}
			break;
	}
}

void TextInput::StartInput( unsigned int id )
{
	if( id < m_InputStrings.size() )
	{
		m_CurrentInputString = id;
		
		g_Input->PauseKeyInput();
		SDL_StartTextInput();
		m_IsInputting = true;
	}
}

const rString& TextInput::StopInput()
{
	g_Input->UnPauseKeyInput();
	SDL_StopTextInput();
	m_IsInputting = false;
	return GetString( m_CurrentInputString );
}

void TextInput::ResetString( unsigned int id )
{
	if( id < m_InputStrings.size() )
		m_InputStrings[id] = "";
}

bool TextInput::IsInputting( unsigned int id ) const
{
	if( id < m_InputStrings.size() )
		return m_IsInputting && m_CurrentInputString == id ;
	else
		return false;
}

const rString& TextInput::GetString( unsigned int id ) const
{
	if( id < m_InputStrings.size() )
		return m_InputStrings[id];
	else
		return m_EmptyString;
}

void TextInput::SetString( unsigned int id, const rString& input )
{
	if ( id < m_InputStrings.size( ) )
	{
		m_InputStrings[id]	= input;
		m_TextCursors[id]	= (unsigned int) input.size( );
	}
}

const unsigned int TextInput::GetTextCursor( unsigned int id ) const
{
	if ( id != -1 )
		return m_TextCursors[id];
	return 0;
}

void TextInput::MoveCursor( unsigned int id, int direction )
{
	if ( id != -1 )
	{
		if ( m_TextCursors[id] + direction >= 0 && m_TextCursors[id] + direction <= m_InputStrings[id].length( ) )
			m_TextCursors[id] += direction;
	}
}

unsigned int TextInput::ReserveAndGetInputID()
{
	m_TextCursors.push_back( 0 );
	m_InputStrings.push_back( "" );
	return static_cast<int>( m_InputStrings.size() ) - 1;
}
