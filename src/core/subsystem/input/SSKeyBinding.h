/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <SDL2/SDL_scancode.h>
#include <memory/Alloc.h>
#include <utility/CallbackConfig.h>
#include <input/Typedefs.h>
#include "../../input/Action.h"
// Conveniency access function
#define g_SSKeyBinding SSKeyBinding::GetInstance()

class SSKeyBinding : public Subsystem
{
public:
    static SSKeyBinding& GetInstance();
    void Startup( ) override;
    void Shutdown( ) override;

	void AddAction( ACTION action, const rString& name, SDL_Scancode scancode, const rString& description );
	void PrintKeys( ) const;

	const rString GetScancodeNameForAction( ACTION action, KeyBindingType bindType = KeyBindingType::Primary ) const;
	SDL_Scancode GetPrimaryScancodeFromAction( ACTION action );
	SDL_Scancode GetSecondaryScancodeFromAction( ACTION action );
	bool ConsumeFromPressStack( ACTION action ) const;
	bool ActionUpDown( ACTION action, bool ignorePause = false ) const;
	bool ActionDownUp( ACTION action, bool ignorePause = false ) const;
	bool ActionUp( ACTION action, bool ignorePause = false ) const;
	bool ActionDown( ACTION action, bool ignorePause = false ) const;
	const rMap<SDL_Scancode, ActionIdentifier>* GetScancodeToActionMap( ) const;
	const rVector<ACTION>* GetActionToActionIdentifier( ) const;

	ActionIdentifier GetActionIdentifier( ACTION action ) const;
	ACTION GetAction( ActionIdentifier actionIdentifier ) const;

private:
    // No external instancing allowed
    SSKeyBinding ( ) : Subsystem( "Keybinding" ) {}
    SSKeyBinding ( const SSKeyBinding & rhs );
    ~SSKeyBinding ( ) {};
    SSKeyBinding& operator=(const SSKeyBinding & rhs);

	CallbackConfigRegisterID m_ConfigCallbackID = 0;
	rVector<ActionIdentifier> m_ActionIdentifers;
	rVector<ACTION> m_ActionToActionIdentifier;
};
