/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <SDL2/SDL_scancode.h>
#include "InputLibraryDefine.h"
#include "Typedefs.h"

class KeyBindingCollection
{
public:
	INPUT_API KeyBindingCollection();
	INPUT_API ~KeyBindingCollection();

	INPUT_API bool AddMappingWithName( const rString& keyName, ActionIdentifier action,
			KeyBindingType keyBindType = KeyBindingType::Any, bool overwrite = false,
			bool clearConflicting = false, rString* errorString = nullptr );

	INPUT_API bool AddMappingWithScancode( SDL_Scancode scancode, ActionIdentifier action,
			KeyBindingType keyBindType = KeyBindingType::Any, bool overwrite = false,
			bool clearConflicting = false, rString* errorString = nullptr );

	INPUT_API const rMap<SDL_Scancode, ActionIdentifier>* GetScancodeToActionMap( ) const;
	INPUT_API const rVector<SDL_Scancode>& 		GetPrimaryBindings				( ) const;
	INPUT_API const rVector<SDL_Scancode>& 		GetSecondaryBindings			( ) const;
	INPUT_API ActionIdentifier 					GetGetActionFromScancode( SDL_Scancode scancode ) const;
	INPUT_API const rString 					GetScancodeNameForAction( ActionIdentifier action, KeyBindingType bindType = KeyBindingType::Primary ) const;
	INPUT_API SDL_Scancode						GetPrimaryScancodeFromAction( ActionIdentifier action ) const;
	INPUT_API SDL_Scancode						GetSecondaryScancodeFromAction( ActionIdentifier action ) const;

	INPUT_API bool BindAction( ActionIdentifier action, SDL_Scancode scancode, KeyBindingType keyBindType, bool overwrite );

private:
	void FillTheVoid( ActionIdentifier action );

	rMap<SDL_Scancode, ActionIdentifier> m_ScancodeToAction;
	rVector<SDL_Scancode> m_ActionToScancodePrimary;
	rVector<SDL_Scancode> m_ActionToScancodeSecondary;

	static const size_t mc_OverflowLimit = 200;
};
