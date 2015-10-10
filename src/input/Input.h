/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_events.h>
#include "InputLibraryDefine.h"
#include <memory/Alloc.h>

#define g_Input Input::GetInstance()

enum INPUT_API MOUSE_BUTTON
{
	MOUSE_BUTTON_LEFT	= SDL_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE	= SDL_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT	= SDL_BUTTON_RIGHT,
	MOUSE_BUTTON_4		= SDL_BUTTON_X1,
	MOUSE_BUTTON_5		= SDL_BUTTON_X2,
};

class Input
{
public:
	static INPUT_API Input*		GetInstance			( );

	INPUT_API					~Input				( );

	INPUT_API void 				Initialize			( );
	INPUT_API void 				Update				( );

	INPUT_API void 				HandleEvent			( SDL_Event& event );

								// Returns true if keyboard key was released previous frame and pressed this frame.
	INPUT_API bool 				KeyUpDown			( SDL_Scancode key, bool ignorePause = false ) const;
								// Returns true if keyboard key was pressed previous frame and released this frame.
	INPUT_API bool 				KeyDownUp			( SDL_Scancode key, bool ignorePause = false ) const;
								// Returns true if keyboard key is pressed this frame.
	INPUT_API bool 				KeyDown				( SDL_Scancode key, bool ignorePause = false ) const;
								// Returns true if keyboard key is released this frame.
	INPUT_API bool 				KeyUp				( SDL_Scancode key, bool ignorePause = false ) const;

								// Returns true if mouse key was released previous frame and pressed this frame. Use MOUSE_KEY enum for input.
	INPUT_API bool 				MouseUpDown			( unsigned int button, bool ignoreConsume = false );
								// Returns true if mouse key was pressed previous frame and released this frame. Use MOUSE_KEY enum for input.
	INPUT_API bool 				MouseDownUp			( unsigned int button, bool ignoreConsume = false );
								// Returns true if mouse key is pressed this frame. Use MOUSE_KEY enum for input.
	INPUT_API bool 				MouseDown			( unsigned int button );
								// Returns true if mouse key is released this frame. Use MOUSE_KEY enum for input.
	INPUT_API bool 				MouseUp				( unsigned int button );
	
	INPUT_API void				ConsumeMouseButtons( );
	INPUT_API void				ConsumeKeys( );
	//void						ConsumeMouseDown( unsigned int button );

								// Change in mouse position along x-axis from previous frame to this frame.
	INPUT_API int 				GetMouseDeltaX		( ) const;
								// Change in mouse position along y-axis from previous frame to this frame.
	INPUT_API int 				GetMouseDeltaY		( ) const;
	INPUT_API int 				GetMousePosX		( ) const;
	INPUT_API int 				GetMousePosY		( ) const;
	INPUT_API int 				GetScrollX			( bool ignorePause = false ) const;
	INPUT_API int 				GetScrollY			( bool ignorePause = false ) const;
	INPUT_API bool				GetMouseInsideWindow( ) const;
	
	INPUT_API void				PauseKeyInput		( );
	INPUT_API void				UnPauseKeyInput		( );
	INPUT_API bool				IsPaused			( ) const;

	INPUT_API const rVector<SDL_Scancode>& GetPressStack() const;
	INPUT_API rVector<SDL_Scancode>& GetEditablePressStack();
	INPUT_API const rVector<SDL_Scancode>& GetReleaseStack() const;
	INPUT_API rVector<SDL_Scancode>& GetEditableReleaseStack();

private:
	const Uint8* 		m_CurrentKeys 		= nullptr;
	Uint8* 				m_PrevKeys 			= nullptr;

	int 				m_KeyArrayLength	= 0;
	int 				m_MouseDeltaX		= 0;
	int 				m_MouseDeltaY		= 0;
	int 				m_MousePosX			= 0;
	int 				m_MousePosY			= 0;
	Uint32				m_CurrentMouseMask	= 0;
	Uint32				m_PrevMouseMask		= 0;
	int 				m_ScrollY 			= 0;
	int 				m_ScrollX 			= 0;
	bool				m_MouseInsideWindow	= true;
	
	bool				m_Paused = false;
	
	bool				m_ConsumedMouse = false;
	bool				m_ConsumedKeys = false;

	rVector<SDL_Scancode> m_PressStack;
	rVector<SDL_Scancode> m_ReleaseStack;
};
