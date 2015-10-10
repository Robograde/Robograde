/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "GUIEngine.h"
#include <GL/glew.h>
#include <utility/Logger.h>

namespace GUI
{
	GUIEngine& GUIEngine::GetInstance()
	{
		static GUIEngine input;
		return input;
	}

	GUIEngine::GUIEngine()
	{
	}

	GUIEngine::~GUIEngine()
	{
		pDelete( m_RootWindow );
	}

	void GUIEngine::Initialize( int windowWidth, int windowHeight )
	{
		g_Graphics2D.Initialize();

		m_RootWindow = pNew( Window, "RootWindow", "", Rectangle( 0, 0, windowWidth, windowHeight ) );
		m_Windows.emplace( "RootWindow", m_RootWindow );
		m_RootWindow->Open();
		//InitializeRoot( windowWidth, windowHeight );

		m_ScriptFunctions.RegisterFunctions();
		
		m_CurrentFontID = FONT_ID_DEFAULT_12;
	}
	
	void GUIEngine::InitializeRoot(int windowWidth, int windowHeight)
	{
		if( !m_RootWindow )
		{
			m_RootWindow = pNew( Window, "RootWindow", "", Rectangle( 0, 0, windowWidth, windowHeight ) );
			m_Windows.emplace( "RootWindow", m_RootWindow );
			m_RootWindow->Open();
		}
		else
		{
			m_RootWindow->SetSize( windowWidth, windowHeight );
		}
	}

	void GUIEngine::Render( int windowWidth, int windowHeight )
	{
		m_RootWindow->SetSize( windowWidth, windowHeight );
		m_RootWindow->Render( glm::ivec2 ( 0, 0 ) );
		g_Graphics2D.Render( windowWidth, windowHeight );
	}

	void GUIEngine::RenderNoGUI( int windowWidth, int windowHeight )
	{
		g_Graphics2D.Render( windowWidth, windowHeight );
	}

	void GUIEngine::Update( float deltaTime )
	{
		m_RootWindow->Update( deltaTime, glm::ivec2 ( 0, 0 ) );
	}

	Window* GUIEngine::GetWindow( const rString& name ) const 
	{
		auto windowIt = m_Windows.find( name );
		if ( windowIt != m_Windows.end() )
		{
			return windowIt->second;
		}
		else
		{
			Logger::Log( "Couldn't find the window: " + name + ", so you can't get it.", "GUIEngine", LogSeverity::WARNING_MSG );
			return nullptr;
		}
	}

	glm::ivec2 GUIEngine::GetWindowSize( const rString& name )
	{
		auto windowIt = m_Windows.find( name );
		if ( windowIt != m_Windows.end() )
		{
			return windowIt->second->GetSize();
		}
		else
		{
			Logger::Log( "Couldn't find the window: " + name + ", so you can't get its size.", "GUIEngine", LogSeverity::WARNING_MSG );
			return glm::ivec2( 0, 0 );
		}
	}
	
	glm::ivec2 GUIEngine::GetWindowPos( const rString& name )
	{
		auto windowIt = m_Windows.find( name );
		if ( windowIt != m_Windows.end() )
		{
			return glm::ivec2( windowIt->second->GetBoundingBoxRef().X, windowIt->second->GetBoundingBoxRef().Y );
		}
		else
		{
			Logger::Log( "Couldn't find the window: " + name + ", so you can't get its position.", "GUIEngine", LogSeverity::WARNING_MSG );
			return glm::ivec2( 0, 0 );
		}
	}
	
	FONT_ID GUIEngine::GetCurrentFont()
	{
		return m_CurrentFontID;
	}

	void GUIEngine::EnqueueText( TextDefinition* textDefinition )
	{
		g_Graphics2D.EnqueueText( textDefinition );
	}

	void GUIEngine::EnqueueSprite( SpriteDefinition* spriteDefinition )
	{
		g_Graphics2D.EnqueueSprite( spriteDefinition );
	}

	void GUIEngine::UseFont( FONT_ID fontID )
	{
		m_CurrentFontID = fontID;
	}
	
	Window* GUIEngine::AddWindow( rString name, Rectangle boundingBox, rString parent , bool border )
	{
		Window* newWindow = pNew( Window, name, parent, boundingBox, border );
		bool success = false;
		for( auto& windowIt : m_Windows )
		{
			if( parent == windowIt.second->GetName() )
				success = windowIt.second->AddChild( name, newWindow );
		}
		
		if( success )
		{
			m_Windows.emplace( name, newWindow );
			return newWindow;
		}
		else
		{
			Logger::Log( "Failed to add window: " + name, "GUIEngine", LogSeverity::ERROR_MSG );
			pDelete( newWindow );
			return nullptr;
		}
	}

	Button* GUIEngine::AddButton( const rString& name, Rectangle boundingBox, const rString& parent )
	{
		Button* btn = pNew( Button, name, parent, boundingBox );
		btn->GetTextDefinitionRef().FontID = m_CurrentFontID;
		AddChild( name, btn, parent );
		return btn;
	}
	
	ComboBox* GUIEngine::AddComboBox( const rString& name, Rectangle boundingBox, const rString& parent )
	{
		ComboBox* cmboBox = pNew( ComboBox, name, parent, boundingBox );
		AddChild( name, cmboBox, parent );
		return cmboBox;
	}
	
	Slider* GUIEngine::AddSlider(const rString& name, Rectangle boundingBox, const rString& parent)
	{
		Slider* slider = pNew( Slider, name, parent, boundingBox );
		AddChild( name, slider, parent );
		return slider;
	}
	
	ProgressBar* GUIEngine::AddProgressBar( const rString& name, Rectangle boundingBox, const rString& parent )
	{
		ProgressBar* progressBar = pNew( ProgressBar, name, parent, boundingBox );
		AddChild( name, progressBar, parent );
		return progressBar;
	}
	
	Text* GUIEngine::AddText( const rString& name, TextDefinition textDefinition, const rString& parent )
	{
		textDefinition.FontID = m_CurrentFontID;
		
		Rectangle boundingBox;
		
		if( textDefinition.BoundsSize.x == -1 )
		{
			glm::ivec2 windowSize = g_GUI.GetWindowSize( parent );
			boundingBox.Width = windowSize.x;
			boundingBox.Height = windowSize.y;
		}
		else
		{
			boundingBox.Width = textDefinition.BoundsSize.x;
			boundingBox.Height = textDefinition.BoundsSize.y;
		}
		
		Text* txt = pNew( Text, name, textDefinition, parent, boundingBox );
		AddChild( name, txt, parent );
		return txt;
	}

	TextBox* GUIEngine::AddTextBox( const rString& name, Rectangle boundingBox, const rString& parent )
	{
		TextBox* txtBox = pNew( TextBox, name, parent, boundingBox );
		txtBox->GetTextDefinitionRef().FontID = m_CurrentFontID;
		AddChild( name, txtBox, parent );
		return txtBox;
	}

	Sprite*GUIEngine::AddSprite( const rString& name, SpriteDefinition spriteDefinition, const rString& parent )
	{
		Sprite* sprite = pNew( Sprite, name, spriteDefinition, parent );
		AddChild( name, sprite, parent );
		return sprite;
	}	

	void GUIEngine::OpenWindow( const rString& name )
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
			windowIt->second->Open();
		else
			Logger::Log( "Couldn't open window: " + name + ", it doesn't exist.", "GUIEngine", LogSeverity::ERROR_MSG  );
	}

	void GUIEngine::CloseWindow( const rString& name )
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
			windowIt->second->Close();
		else
			Logger::Log( "Couldn't close window: " + name + ", it doesn't exist.", "GUIEngine", LogSeverity::WARNING_MSG  );
	}

	void GUIEngine::ToggleWindow( const rString& name )
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
			windowIt->second->ToggleOpen();
		else
			Logger::Log( "Couldn't toggle window: " + name + ", it doesn't exist.", "GUIEngine", LogSeverity::ERROR_MSG  );
	}
	
	bool GUIEngine::IsWindowOpen( const rString& name)
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
			return windowIt->second->IsOpen();
		else
			Logger::Log( "Couldn't get window: " + name + ", it doesn't exist.", "GUIEngine", LogSeverity::ERROR_MSG  );
		
		return false;
	}
	
	void GUIEngine::BringWindowToFront( const rString& name )
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
			return windowIt->second->BringToFront();
		else
			Logger::Log( "Couldn't bring window window: " + name + " to front, it doesn't exist.", "GUIEngine", LogSeverity::ERROR_MSG  );
	}
	
	void GUIEngine::DeleteObject( const rString& name )
	{
		m_RootWindow->DeleteChild( name );
	}

	void GUIEngine::EraseWindow( const rString& name )
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
		{
			windowIt->second = nullptr;
			m_Windows.erase( windowIt );
		}
	}
	
	void GUIEngine::SetWindowMoveable( const rString& name, bool moveable)
	{
		for( auto& windowIt : m_Windows )
		{
			if( name == windowIt.second->GetName() )
				windowIt.second->SetMoveable( moveable );
		}
	}
	
	void GUIEngine::SetWindowClickThrough( const rString& name, bool clickThrough )
	{
		auto windowIt = m_Windows.find( name ) ;
		if( windowIt != m_Windows.end() )
			windowIt->second->SetClickThrough( clickThrough );
		else
			Logger::Log( "Couldn't get window: " + rString( name.c_str()) + ", it doesn't exist.", "GUIEngine", LogSeverity::ERROR_MSG  );
	}

	void GUIEngine::AddButtonToGroup( Button* btn, const rString& group )
	{
		if( group != "" && btn != nullptr)
		{
			for ( Button* btntemp : m_ToggleButtonGroups[group] )
			{
				if ( btntemp->GetName() == btn->GetName() )
					return;
			}
			m_ToggleButtonGroups[group].push_back( btn );
		}
	}
	
	void GUIEngine::ClearButtonGroup( const rString& group )
	{
		auto buttonGroupIt = m_ToggleButtonGroups.find( group );
		if( buttonGroupIt != m_ToggleButtonGroups.end() )
			m_ToggleButtonGroups.erase( buttonGroupIt );
	}
	
	void GUIEngine::ToggleButtonGroup( const rString& buttonName, const rString& group )
	{
		auto buttonGroupIt = m_ToggleButtonGroups.find( group ) ;
		if( buttonGroupIt != m_ToggleButtonGroups.end() )
		{
			tVector<Button*>& buttons = buttonGroupIt->second;
			
			for( Button* button : buttons )
			{
				if( button->GetName() == buttonName )
					button->SetToggled( true );
				else
					button->SetToggled( false );
			}
		}
	}
	
	void GUIEngine::SetEnabled( const rString& objectName, const rString& windowName, bool enabled )
	{
		GetChild( objectName, windowName )->SetEnabled( enabled );
	}
	
	void GUIEngine::AddWindowToGroup( Window* window, const rString& group )
	{
		if( group != "" && window != nullptr)
		{
			for ( Window* windowTemp : m_ToggleWindowGroups[group] )
			{
				if ( windowTemp->GetName() == window->GetName() )
					return;
			}
			m_ToggleWindowGroups[group].push_back( window );
		}
	}
	
	void GUIEngine::ClearWindowGroup( const rString& group )
	{
		auto buttonGroupIt = m_ToggleWindowGroups.find( group );
		if( buttonGroupIt != m_ToggleWindowGroups.end() )
			m_ToggleWindowGroups.erase( buttonGroupIt );
	}
	
	void GUIEngine::ToggleWindowGroup( const rString& windowName, const rString& group )
	{
		auto windowGroupIt = m_ToggleWindowGroups.find( group ) ;
		if( windowGroupIt != m_ToggleWindowGroups.end() )
		{
			for( Window* window : windowGroupIt->second )
			{
				if( window->GetName() == windowName )
					window->Open();
				else
					window->Close();
			}
		}
	}
	
	void GUIEngine::SetButtonToggle( const rString& name, const rString& parent, bool value )
	{
		Button* btn = (Button*)GetChild( name, parent );
		if( btn )
			btn->SetToggled( value );
	}
	
	void GUIEngine::ToggleInput( bool enableInput )
	{
		m_InputEnabled = enableInput;
	}
	
	//TODOIA Seems to be unused, remove?
	bool GUIEngine::IsInputEnabled()
	{
		return m_InputEnabled;
	}
	
	//Private
	
	void GUIEngine::AddChild( const rString& name, Object* child, const rString& parent )
	{
		rString newName = name;
		if( name == "" )
			newName = "ChildObject" + rToString( m_ObjectNameIndex ); 
		
		bool success = false;
		for( auto& windowIt : m_Windows )
		{
			if( parent == windowIt.second->GetName() )
				success = windowIt.second->AddChild( newName, child );
		}
		if( !success )
		{
			Logger::Log( "Failed to add child object: " + newName, "GUIEngine", LogSeverity::ERROR_MSG );
			pDelete( child );
		}
		else
			m_ObjectNameIndex++;
	}
	
	Object*GUIEngine::GetChild( const rString& name, const rString& parent )
	{
		for( auto& windowIt : m_Windows )
		{
			if( parent == windowIt.second->GetName() )
				return windowIt.second->GetChild( name );
		}
		
		Logger::Log( "Failed to find window: " + parent, "GUIEngine", LogSeverity::ERROR_MSG );
		return nullptr;
	}
}
