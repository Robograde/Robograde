/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once

#include "graphics2D/Graphics2D.h"
#include "object/Window.h"
#include "object/TextBox.h"
#include "object/Text.h"
#include "object/Sprite.h"
#include "object/Button.h"
#include "object/ComboBox.h"
#include "object/Slider.h"
#include "object/ProgressBar.h"
#include "object/CircularSlider.h"
#include <script/ScriptEngine.h>

#include "GUIScriptFunctions.h"

#ifdef _WIN32
	#ifdef GUI_DLL_EXPORT
		#define GUI_API __declspec(dllexport) // Exports symbols
	#else
		#define GUI_API __declspec(dllimport) // Imports symbols
	#endif
#else
	#define GUI_API // Unix is not annoying :D
#endif


#define g_GUI GUI::GUIEngine::GetInstance()

namespace GUI 
{
	class GUIEngine
	{
	public:
		GUI_API static GUIEngine&	GetInstance();
		
		GUI_API				GUIEngine		();
							~GUIEngine		();
		GUI_API void		Initialize		( int windowWidth, int windowHeight );
		GUI_API void		InitializeRoot	( int windowWidth, int windowHeight );
		GUI_API void		Render			( int windowWidth, int windowHeight );
		GUI_API void		RenderNoGUI		( int windowWidth, int windowHeight );
		GUI_API void		Update			( float deltaTime );
							
		GUI_API Window*		GetWindow 		( const rString& name ) const;
		GUI_API glm::ivec2	GetWindowSize	( const rString& name );
		GUI_API glm::ivec2	GetWindowPos	( const rString& name );
		GUI_API FONT_ID		GetCurrentFont	();
							
		GUI_API void 		EnqueueText		( TextDefinition* textDefinition );
		GUI_API void 		EnqueueSprite	( SpriteDefinition* spriteDefinition );
		GUI_API void 		UseFont			( FONT_ID fontID );
		
		//Add objects
		GUI_API Window*		AddWindow		( rString name, Rectangle boundingBox, rString parent, bool border = false );
		GUI_API Button*		AddButton		( const rString& name, Rectangle boundingBox, const rString& parent );
		GUI_API ComboBox*	AddComboBox		( const rString& name, Rectangle boundingBox, const rString& parent );
		GUI_API Slider*		AddSlider		( const rString& name, Rectangle boundingBox, const rString& parent );
		GUI_API ProgressBar*AddProgressBar	( const rString& name, Rectangle boundingBox, const rString& parent );
		GUI_API CircularSlider* AddCircularSlider	( const rString& name, glm::ivec2 position, const rString& parent );
		
		//Add a text. Width and Height are optional, will align text to window if not set
		GUI_API Text*		AddText			( const rString& name, TextDefinition textDefinition, const rString& parent );
		
		GUI_API TextBox*	AddTextBox		( const rString& name, Rectangle boundingBox, const rString& parent );
		GUI_API Sprite*		AddSprite		( const rString& name, SpriteDefinition spriteDefinition, const rString& parent );
		
		
		//Window management
		GUI_API void 		OpenWindow			( const rString& name );
		GUI_API void 		CloseWindow			( const rString& name );
		GUI_API void 		ToggleWindow		( const rString& name );
		GUI_API bool 		IsWindowOpen		( const rString& name );
		GUI_API void 		BringWindowToFront	( const rString& name );
							
		GUI_API void		DeleteObject		( const rString& name );
				void		EraseWindow			( const rString& name );
							
		GUI_API void		SetWindowMoveable		( const rString& name, bool moveable );
		GUI_API void		SetWindowClickThrough	( const rString& name , bool clickThrough );
							
							
		GUI_API void 		AddButtonToGroup	( Button* btn, const rString& group );
		GUI_API void 		ClearButtonGroup	( const rString& group );
		GUI_API void 		ToggleButtonGroup	( const rString& buttonName, const rString& group );
		GUI_API void 		SetEnabled			( const rString& objectName, const rString& windowName, bool enabled );
							
		GUI_API void 		AddWindowToGroup	( Window* window, const rString& group );
		GUI_API void 		ClearWindowGroup	( const rString& group );
		GUI_API void 		ToggleWindowGroup	( const rString& windowName, const rString& group );
							
		GUI_API void 		SetButtonToggle		( const rString& name, const rString& parent, bool value );
							
		GUI_API void 		ToggleInput			( bool enableInput );
		GUI_API bool 		IsInputEnabled		( );
		
		GUI_API Object*		GetObject			(  const rString& name, const rString& parent );

		GUI_API void		SetToolTipText		( const rString& text );
		
		
	private:
		void				AddChild			( const rString& name, Object* child, const rString& parent );
		Object*				GetChild			( const rString& name, const rString& parent );

		void				RenderToolTip		();
		
		tMap<rString, Window*>			m_Windows;
		Window*							m_RootWindow = nullptr;
		
		ScriptFunctions					m_ScriptFunctions;
		
		tMap<rString, tVector<Button*>>	m_ToggleButtonGroups;
		tMap<rString, tVector<Window*>>	m_ToggleWindowGroups;
		
		FONT_ID							m_CurrentFontID;
		
		bool							m_InputEnabled = true;
		int								m_ObjectNameIndex = 0;

		TextDefinition					m_ToolTipText;
		SpriteDefinition				m_ToolTipBackground;
	};
}
