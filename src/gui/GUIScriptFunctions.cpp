/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "GUIScriptFunctions.h"
#include "GUIEngine.h"

namespace GUI
{
	
	void ScriptFunctions::RegisterFunctions()
	{
		g_Script.Register( "OpenWindow", std::bind( &ScriptFunctions::OpenWindowSF, this, std::placeholders::_1 ) );
		g_Script.Register( "CloseWindow", std::bind( &ScriptFunctions::CloseWindowSF, this, std::placeholders::_1 ) );
		g_Script.Register( "ToggleWindow", std::bind( &ScriptFunctions::ToggleWindowSF, this, std::placeholders::_1 ) );
		
		g_Script.Register( "AddWindow", std::bind( &ScriptFunctions::AddWindowSF, this, std::placeholders::_1 )  );
		g_Script.Register( "AddButton", std::bind( &ScriptFunctions::AddButtonSF, this, std::placeholders::_1 )  );
		g_Script.Register( "AddImageButton", std::bind( &ScriptFunctions::AddImageButtonSF, this, std::placeholders::_1 )  );
		g_Script.Register( "AddToggleButton", std::bind( &ScriptFunctions::AddToggleButtonSF, this, std::placeholders::_1 )  );
		g_Script.Register( "AddToggleImageButton", std::bind( &ScriptFunctions::AddToggleImageButtonSF, this, std::placeholders::_1 )  );
		g_Script.Register( "AddText", std::bind( &ScriptFunctions::AddTextSF, this, std::placeholders::_1 ) );
		
		g_Script.Register( "BringWindowToFront", std::bind( &ScriptFunctions::BringWindowToFrontSF, this, std::placeholders::_1 ) );
		
		g_Script.Register( "ToggleButtonGroup", std::bind( &ScriptFunctions::ToggleButtonGroupSF, this, std::placeholders::_1 ) );
		g_Script.Register( "ToggleWindowGroup", std::bind( &ScriptFunctions::ToggleWindowGroupSF, this, std::placeholders::_1 ) );
		g_Script.Register( "SetButtonToggle", std::bind( &ScriptFunctions::SetButtonToggleSF, this, std::placeholders::_1 ) );
		
		
		g_Script.Register( "UseFont", std::bind( &ScriptFunctions::UseFontSF, this, std::placeholders::_1 )  );
		
		g_Script.Register( "GetWindowSize", std::bind( &ScriptFunctions::GetWindowSizeSF, this, std::placeholders::_1 )  );
		
		g_Script.Register( "SetWindowMoveable", std::bind( &ScriptFunctions::SetWindowMoveableSF, this, std::placeholders::_1 )  );
		
		//Not used atm
		g_Script.Register( "ToggleObject", std::bind( &ScriptFunctions::SetEnabledSF, this, std::placeholders::_1 ) );
		
		//g_Script.Run( "../../../script/gui.lua" );
	}
	
	int ScriptFunctions::AddTextSF( IScriptEngine* scriptEngine )
	{
		//TODOIA: Do prettier later
		int a = scriptEngine->PopInt();
		int b = scriptEngine->PopInt();
		int g = scriptEngine->PopInt();
		int r = scriptEngine->PopInt();
		
		rString parent = scriptEngine->PopString();
		int posY = scriptEngine->PopInt();
		int posX = scriptEngine->PopInt();
		rString text = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		
		GUI::TextDefinition textDef( text.c_str(), posX, posY );
		textDef.Colour = glm::vec4( r, g, b, a );
		g_GUI.AddText( name, textDef, parent );
		return 0;
	}
	
	int ScriptFunctions::AddButtonSF( IScriptEngine* scriptEngine )
	{
		rString parent = scriptEngine->PopString();
		int height = scriptEngine->PopInt();
		int width = scriptEngine->PopInt();
		int posY = scriptEngine->PopInt();
		int posX = scriptEngine->PopInt();
		rString script = scriptEngine->PopString();
		rString text = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		Button* btn = g_GUI.AddButton( name, GUI::Rectangle( posX, posY, width, height ), parent );
		
		//TextDefinition textDef = TextDefinition( text.c_str(), 0, 0 );
		//textDef.FontIdent = g_Graphics2D.GetCurrentFontIdent();
		
		btn->SetText( text.c_str() );
		btn->SetClickScript( script );
		return 0;
	}
	
	int ScriptFunctions::AddToggleButtonSF(IScriptEngine* scriptEngine)
	{
		rString group = scriptEngine->PopString();
		rString parent = scriptEngine->PopString();
		int height = scriptEngine->PopInt();
		int width = scriptEngine->PopInt();
		int posY = scriptEngine->PopInt();
		int posX = scriptEngine->PopInt();
		rString script = scriptEngine->PopString();
		rString text = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		Button* btn = g_GUI.AddButton( name, GUI::Rectangle( posX, posY, width, height ), parent );
		
		//TextDefinition textDef = TextDefinition( text.c_str(), 0, 0 );
		//textDef.FontIdent = g_Graphics2D.GetCurrentFontIdent();
		
		btn->SetText( text.c_str() );
		btn->SetClickScript( script );
		
		if( group != "" )
			btn->SetToggleGroup( group );
		else
		{
			btn->SetToggled( true );
		}
			
		return 0;
	}
	
	int ScriptFunctions::AddImageButtonSF(IScriptEngine* scriptEngine)
	{
		rString parent = scriptEngine->PopString();
		int height = scriptEngine->PopInt();
		int width = scriptEngine->PopInt();
		int posY = scriptEngine->PopInt();
		int posX = scriptEngine->PopInt();
		rString script = scriptEngine->PopString();
		rString text = scriptEngine->PopString();
		rString imagePath = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		Button* btn = g_GUI.AddButton( name, GUI::Rectangle( posX, posY, width, height ), parent );
		
		//TextDefinition textDef = TextDefinition( text.c_str(), 0, 0 );
		//textDef.FontIdent = g_Graphics2D.GetCurrentFontIdent();
		
		btn->SetBackgroundImage( imagePath );
		btn->SetText( text.c_str() );
		btn->SetClickScript( script );
		btn->SetColour ( glm::vec4( 1.0f ) );
		return 0;
	}
	
	int ScriptFunctions::AddToggleImageButtonSF(IScriptEngine* scriptEngine)
	{
		rString group = scriptEngine->PopString();
		rString parent = scriptEngine->PopString();
		int height = scriptEngine->PopInt();
		int width = scriptEngine->PopInt();
		int posY = scriptEngine->PopInt();
		int posX = scriptEngine->PopInt();
		rString script = scriptEngine->PopString();
		rString text = scriptEngine->PopString();
		rString imagePath = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		Button* btn = g_GUI.AddButton( name, GUI::Rectangle( posX, posY, width, height ), parent );
		
		//TextDefinition textDef = TextDefinition( text.c_str(), 0, 0 );
		//textDef.FontIdent = g_Graphics2D.GetCurrentFontIdent();
		
		btn->SetBackgroundImage( imagePath );
		btn->SetText( text.c_str() );
		btn->SetClickScript( script );
		btn->SetToggleGroup( group );
		btn->SetColour( glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		//btn->SetHighLightColour( glm::vec4( 0.5f, 1.0f, 1.0f, 1.0f ) );
		return 0;
	}
	
	int ScriptFunctions::AddWindowSF( IScriptEngine* scriptEngine )
	{
		bool border = scriptEngine->PopBool();
		rString parent = scriptEngine->PopString();
		int height = scriptEngine->PopInt();
		int width = scriptEngine->PopInt();
		int posY = scriptEngine->PopInt();
		int posX = scriptEngine->PopInt();
		rString name = scriptEngine->PopString();
		
		g_GUI.AddWindow( name, GUI::Rectangle( posX, posY, width, height ), parent, border );
		return 0;
	}
	
	int ScriptFunctions::GetWindowSizeSF( IScriptEngine* scriptEngine )
	{
		rString name = scriptEngine->PopString();
		
		scriptEngine->PushInt( g_GUI.GetWindowSize( name ).x );
		scriptEngine->PushInt( g_GUI.GetWindowSize( name ).y );
		return 2;
	}
	
	int ScriptFunctions::SetWindowMoveableSF( IScriptEngine* scriptEngine )
	{
		bool moveable = scriptEngine->PopBool();
		rString name = scriptEngine->PopString();
		
		g_GUI.SetWindowMoveable( name, moveable );
		return 0;
	}
	
	int ScriptFunctions::OpenWindowSF( IScriptEngine* scriptEngine )
	{
		rString name = scriptEngine->PopString();
		
	 	g_GUI.OpenWindow( name );
		return 0;
	}
	
	int ScriptFunctions::CloseWindowSF( IScriptEngine* scriptEngine )
	{
		rString name = scriptEngine->PopString();
		
	 	g_GUI.CloseWindow( name );
		return 0;
	}
	
	int ScriptFunctions::ToggleWindowSF( IScriptEngine* scriptEngine )
	{
		rString name = scriptEngine->PopString();
		
		bool isOpen = false;
		
		if ( g_GUI.IsWindowOpen( name ) )
			g_GUI.CloseWindow( name );
		else
		{
			g_GUI.OpenWindow( name );
			isOpen = true;
		}
		
		scriptEngine->SetBool( "toggle", isOpen );
		
		return 0;
	}
	
	int ScriptFunctions::BringWindowToFrontSF(IScriptEngine* scriptEngine)
	{
		rString name = scriptEngine->PopString();
		g_GUI.BringWindowToFront( name );
		return 0;
	}
	
	int ScriptFunctions::UseFontSF( IScriptEngine* scriptEngine )
	{
		int fontSize = scriptEngine->PopInt();
		rString fontName = scriptEngine->PopString();
		
		//g_GUI.UseFont( fontName, fontSize );
		
		return 0;
	}
	
	int ScriptFunctions::ToggleButtonGroupSF( IScriptEngine* scriptEngine )
	{
		rString group = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		g_GUI.ToggleButtonGroup( name, group );
		return 0;
	}
	
	int ScriptFunctions::ToggleWindowGroupSF(IScriptEngine* scriptEngine)
	{
		rString group = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		g_GUI.ToggleWindowGroup( name, group );
		return 0;
	}
	
	int ScriptFunctions::SetButtonToggleSF(IScriptEngine* scriptEngine)
	{
		bool toggledOn = scriptEngine->PopBool();
		rString parent = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		g_GUI.SetButtonToggle( name, parent, toggledOn );
		return 0; 
	}
	
	int ScriptFunctions::SetEnabledSF( IScriptEngine* scriptEngine )
	{
		bool enabled = scriptEngine->PopBool();
		rString parent = scriptEngine->PopString();
		rString name = scriptEngine->PopString();
		
		g_GUI.SetEnabled( name, parent, enabled );
		
		return 0;
	}
}
