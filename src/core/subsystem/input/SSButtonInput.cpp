/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSButtonInput.h"
#include <SDL2/SDL_keyboard.h>
#include <utility/Logger.h>
#include "../input/SSKeyBinding.h"

SSButtonInput& SSButtonInput::GetInstance( )
{
	static SSButtonInput instance;
	return instance;
}

void SSButtonInput::Startup()
{
	m_RootLevel.Name = "";
	m_CurrentLevel = &m_RootLevel;

	m_TimeoutTime = 1.0f;
	m_TimeOutCountDown = 0.0f;
}

void SSButtonInput::UpdateUserLayer( const float deltaTime )
{
	if( m_TimeOutCountDown > 0.0f )
		m_TimeOutCountDown -= deltaTime;
	else
	{
		for( ButtonKeyBinding& child : m_CurrentLevel->Children )
		{
			if( child.Button )
				child.Button->SetSecondaryText( "" );
		}
		
		m_CurrentLevel  = &m_RootLevel;

		for( ButtonKeyBinding& child : m_CurrentLevel->Children )
		{
			if( child.Button && child.Button->IsEnabled() )
				child.Button->SetSecondaryText( SDL_GetScancodeName( g_SSKeyBinding.GetPrimaryScancodeFromAction( child.Action ) ) );
		}
	}
	
	if( m_CurrentLevel != &m_RootLevel )
	{
		for( ButtonKeyBinding& child : m_RootLevel.Children )
		{
			if ( child.Button && child.Button->IsEnabled( ) && child.Children.size( ) == 0 )
				child.Button->SetSecondaryText( SDL_GetScancodeName( g_SSKeyBinding.GetPrimaryScancodeFromAction( child.Action ) ) );
		}
	}
}

void SSButtonInput::Shutdown()
{
	m_RootLevel.Children.clear();
}

bool SSButtonInput::AddButtonBinding( GUI::Button* button, const rString& name, ACTION action, const rString& parent )
{
	ButtonKeyBinding newBtnKeyBinding;
	newBtnKeyBinding.Name = name;
	newBtnKeyBinding.Button = button;
	newBtnKeyBinding.Action = action;
	
	if( m_RootLevel.TryAdd( newBtnKeyBinding, parent ) )
		return true;
	else
	{
		Logger::Log( "Couldn't bind action " + rToString( action ) + " to button.", "SSButtonInput", LogSeverity::ERROR_MSG );
		return false;
	}
}

bool SSButtonInput::HandleAction( ACTION action )
{
	ButtonKeyBinding* lastLevel;

	bool handledAction = false;
	for( ButtonKeyBinding& child : m_CurrentLevel->Children )
	{
		if( child.Action == action )
		{
			if( child.Button )
			{
				if( child.Button->IsEnabled() )
					child.Button->SimulateClick();
			}
			lastLevel = m_CurrentLevel;

			m_CurrentLevel = &child;
			
			if( child.Children.size() == 0 )
			{
				m_TimeOutCountDown = 0.0f;
			}
			else
			{
				m_TimeOutCountDown = m_TimeoutTime;
				for( ButtonKeyBinding& child2 : child.Children )
				{
					if( child2.Button && child2.Button->IsEnabled() )
					{
						SDL_Scancode scancode = g_SSKeyBinding.GetPrimaryScancodeFromAction( child2.Action );
						child2.Button->SetSecondaryText( SDL_GetScancodeName( scancode ) );
					}
				}
			}
			
			handledAction = true;
		}
	}
	if( handledAction )
	{
		for( ButtonKeyBinding& child : lastLevel->Children )
		{
			if( child.Button )
				child.Button->SetSecondaryText( "" );
		}
	}
	else
	{
		m_TimeOutCountDown = 0.0f;
		
		
		for( ButtonKeyBinding& child : m_RootLevel.Children )
		{
			if( child.Action == action )
			{
				if( child.Button )
					child.Button->SimulateClick();
//				lastLevel = m_CurrentLevel;
				
//				m_CurrentLevel = &child;
				
			}
		}
	}
	
	return handledAction;
}
