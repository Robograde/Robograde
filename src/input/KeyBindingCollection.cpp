/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "KeyBindingCollection.h"
#include <SDL2/SDL_keyboard.h>
#include <utility/Logger.h>
#include "KeyBindings.h"

KeyBindingCollection::KeyBindingCollection( ) 
{
	m_ScancodeToAction.clear( );
}

KeyBindingCollection::~KeyBindingCollection( )
{
}

bool KeyBindingCollection::AddMappingWithName( const rString& keyName, ActionIdentifier action, KeyBindingType keyBindType, bool overwrite, bool clearConflicting, rString* errorString )
{
	SDL_Scancode scanCode = SDL_GetScancodeFromName( keyName.c_str( ) );
	if ( scanCode != SDL_SCANCODE_UNKNOWN )
	{
		return AddMappingWithScancode( scanCode, action, keyBindType, overwrite, clearConflicting, errorString );
	}
	else
	{
		Logger::Log( "Failed to get scancode from name: " + keyName, "KeyBindings", LogSeverity::WARNING_MSG );
		if ( errorString != nullptr )
		{
			*errorString = "Failed to get scancode from name: " + keyName;
		}
		return false;
	}
}

bool KeyBindingCollection::AddMappingWithScancode( SDL_Scancode scancode, ActionIdentifier action, KeyBindingType keyBindType, bool overwrite, bool clearConflicting, rString* errorString )
{
	FillTheVoid( action );
	auto keyIt = m_ScancodeToAction.find( scancode );
	// Warn about overwriting duplicate keybindings
	if ( keyIt != m_ScancodeToAction.end( ) && !clearConflicting )
	{
		Logger::Log( "Can't bind key: \"" + rString( SDL_GetScancodeName( scancode ) ) + "\" to action " +
					 g_KeyBindings.GetDescription( action ) + " because it is already bound to action \"" + g_KeyBindings.GetDescription( keyIt->second ) + "\"",
					 "KeyBindings", LogSeverity::WARNING_MSG );
		if ( errorString != nullptr )
		{
			*errorString = "Can't bind key: \"" + rString( SDL_GetScancodeName( scancode ) ) + "\" to action " +
					 g_KeyBindings.GetDescription( action ) + " because it is already bound to action \"" + g_KeyBindings.GetDescription( keyIt->second ) + "\"";
		}
		return false;
	}
	else
	{
		// Try to key to action
		if ( BindAction( action, scancode, keyBindType, overwrite ) )
		{
			Logger::Log( "Bound key \"" + rString( SDL_GetScancodeName( scancode ) ) + "\" to action \"" + g_KeyBindings.GetDescription( action ) + "\"",
						 "KeyBindings", LogSeverity::DEBUG_MSG );
			if ( errorString != nullptr )
			{
				*errorString = "Bound key \"" + rString( SDL_GetScancodeName( scancode ) ) + "\" to action \"" + g_KeyBindings.GetDescription( action ) + "\"";
			}
			return true;
		}
		else
		{
			Logger::Log( "Can't bind key: \"" + rString( SDL_GetScancodeName( scancode ) ) + "\" to action \"" +
						 g_KeyBindings.GetDescription( action ) + "\" because no free bind slots are avaliable", "KeyBindings", LogSeverity::WARNING_MSG );
			if ( errorString != nullptr )
			{
				*errorString = "Can't bind key: \"" + rString( SDL_GetScancodeName( scancode ) ) + "\" to action \"" +
						 g_KeyBindings.GetDescription( action ) + "\" because no free bind slots are avaliable";
			}
			return false;
		}
	}
}

const rMap<SDL_Scancode, ActionIdentifier>* KeyBindingCollection::GetScancodeToActionMap( ) const
{
	return &m_ScancodeToAction;
}

bool KeyBindingCollection::BindAction( ActionIdentifier action, SDL_Scancode scancode, KeyBindingType keyBindType, bool overwrite )
{
	FillTheVoid( action );
	auto freePrevious = [this, action, scancode] ( bool primary )
	{
		SDL_Scancode previousCode = (primary ? m_ActionToScancodePrimary : m_ActionToScancodeSecondary)[static_cast<int>(action)];
		if ( previousCode != SDL_SCANCODE_UNKNOWN )
		{
			auto it = m_ScancodeToAction.find( previousCode );
			if ( it != m_ScancodeToAction.end() )
			{
				m_ScancodeToAction.erase( it );
			}
		}
	};
	auto addPrimaryBinding = [this, action, scancode, &freePrevious]( )
	{
		freePrevious( true );
		m_ActionToScancodePrimary[static_cast<int>( action )] = scancode;
		m_ScancodeToAction[scancode] = action;
	};
	auto addSecondaryBinding = [this, action, scancode, &freePrevious]( )
	{
		freePrevious( false );
		m_ActionToScancodeSecondary[static_cast<int>( action )] = scancode;
		m_ScancodeToAction[scancode] = action;
	};

	if ( overwrite )
	{
		if ( keyBindType == KeyBindingType::Primary )
			addPrimaryBinding( );
		else if ( keyBindType == KeyBindingType::Secondary )
			addSecondaryBinding( );
		else if ( keyBindType == KeyBindingType::Any )
		{
			// Check for any free bindings
			if ( m_ActionToScancodePrimary[static_cast<int>( action )] == SDL_SCANCODE_UNKNOWN )
				addPrimaryBinding( );
			else if ( m_ActionToScancodeSecondary[static_cast<int>( action )] == SDL_SCANCODE_UNKNOWN )
				addSecondaryBinding( );
			// Always overwrite secondary if none was specified or free
			else
				addSecondaryBinding( );
		}
		return true;
	}
	// No overwriting
	else
	{
		// Try to bind primary first
		if ( keyBindType == KeyBindingType::Primary || keyBindType == KeyBindingType::Any )
		{
			// Only bind unbound action
			if ( m_ActionToScancodePrimary[static_cast<int>( action )] == SDL_SCANCODE_UNKNOWN )
			{
				addPrimaryBinding( );
				return true;
			}
		}
		// Try secondary after primary
		if ( keyBindType == KeyBindingType::Secondary || keyBindType == KeyBindingType::Any )
		{
			// Only bind unbound action
			if ( m_ActionToScancodeSecondary[static_cast<int>( action )] == SDL_SCANCODE_UNKNOWN )
			{
				addSecondaryBinding( );
				return true;
			}
		}
		// Failed to bind
		return false;
	}
}

const rVector<SDL_Scancode>& KeyBindingCollection::GetPrimaryBindings() const
{
	return m_ActionToScancodePrimary;
}

const rVector<SDL_Scancode>& KeyBindingCollection::GetSecondaryBindings() const
{
	return m_ActionToScancodeSecondary;
}

ActionIdentifier KeyBindingCollection::GetGetActionFromScancode( SDL_Scancode scancode ) const
{
	auto it = m_ScancodeToAction.find( scancode );
	if ( it != m_ScancodeToAction.end( ) )
		return it->second;
	else
		return ActionIdentifier( );
}

const rString KeyBindingCollection::GetScancodeNameForAction( ActionIdentifier action, KeyBindingType bindType ) const
{
	return rString( SDL_GetScancodeName( ( bindType == KeyBindingType::Primary ? m_ActionToScancodePrimary : m_ActionToScancodeSecondary ).at( static_cast<int>( action ) ) ) );
}

SDL_Scancode KeyBindingCollection::GetPrimaryScancodeFromAction( ActionIdentifier action ) const
{
	assert( static_cast<int>( action ) < m_ActionToScancodePrimary.size( ) );
	return m_ActionToScancodePrimary.at( static_cast<int>( action ) );
}

SDL_Scancode KeyBindingCollection::GetSecondaryScancodeFromAction( ActionIdentifier action ) const
{
	assert( static_cast<int>( action ) < m_ActionToScancodeSecondary.size( ) );
	return m_ActionToScancodeSecondary.at( static_cast<int>( action ) );
}

void KeyBindingCollection::FillTheVoid( ActionIdentifier action )
{
	assert( static_cast<int>( action ) < mc_OverflowLimit );
	auto fillVoid = []( ActionIdentifier action, rVector<SDL_Scancode>& vec )
	{
		if ( static_cast<int>( action ) >= vec.size( ) )
		{
			size_t prevSize = vec.size( );
			vec.resize( static_cast<int>(action) + 1 );
			std::fill( vec.begin( ) + prevSize, vec.end( ), SDL_SCANCODE_UNKNOWN );
		}
	};
	fillVoid( action, m_ActionToScancodePrimary );
	fillVoid( action, m_ActionToScancodeSecondary );
}