/**************************************************
2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include <input/KeyBindings.h>
#include "../../input/Action.h"

#define g_SSButtonInput SSButtonInput::GetInstance()

struct ButtonKeyBinding
{
	ACTION Action;
	rString Name;
	GUI::Button* Button = nullptr;
	rVector<ButtonKeyBinding> Children;
	
	
	bool TryAdd( ButtonKeyBinding& newChild, const rString& parent )
	{
		if( Name == parent )
		{
			for( ButtonKeyBinding& child : Children )
			{
				if( child.Action == newChild.Action )
				{
					return false;
				}
			}
			Children.push_back( newChild );
			return true;
		}
		for( ButtonKeyBinding& child : Children )
		{
			if( child.TryAdd( newChild, parent ) )
				return true;
		}
		return false;
	}
};

class SSButtonInput : public Subsystem
{
public:
	static SSButtonInput& GetInstance( );
	
	void Startup();
	void UpdateUserLayer( const float deltaTime )	override;
	void Shutdown();
	
	bool AddButtonBinding( GUI::Button* button, const rString& name, ACTION action, const rString& parent );
	
	bool HandleAction( ACTION action );

private:
	// No external instancing allowed
	SSButtonInput( ) : Subsystem( "ButtonInput" ) { }
	SSButtonInput( const SSButtonInput& rhs );
	~SSButtonInput( ) { }
	SSButtonInput& operator=(const SSButtonInput& rhs);
	
	ButtonKeyBinding m_RootLevel;
	ButtonKeyBinding* m_CurrentLevel;

	float m_TimeoutTime;
	float m_TimeOutCountDown;
};