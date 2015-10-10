/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include <SDL2/SDL_events.h>
#include "Input.h"
#include <memory/Alloc.h>

#define g_TextInput TextInput::GetInstance()

class TextInput {
public:
	INPUT_API static TextInput& GetInstance( );

	INPUT_API void					Initialize	( );
	INPUT_API void					HandleEvents( const SDL_Event& event );

	// Starts inputting and resets the input string.
	INPUT_API void					StartInput	( unsigned int id );
	// Stops inputting and returns the inputted string.
	INPUT_API const rString&		StopInput	( );
	INPUT_API bool					IsInputting ( unsigned int id ) const;

	INPUT_API void					ResetString ( unsigned int id );

	INPUT_API const	rString&		GetString( unsigned int id ) const;
	INPUT_API void					SetString( unsigned int id, const rString& input );
	
	INPUT_API const unsigned int	GetTextCursor( unsigned int id ) const;
	
	INPUT_API void					MoveCursor( unsigned int id, int direction );
	
	INPUT_API unsigned int			ReserveAndGetInputID();

private:
	//rString							m_InpurString = "";
	rString							m_Composition;
	bool							m_IsInputting = false;
	
	const rString					m_EmptyString = "";
	pVector<rString>				m_InputStrings;
	pVector<unsigned int>			m_TextCursors;
	
	unsigned int					m_CurrentInputString;
};

