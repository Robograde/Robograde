#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSEditorEvents SSEditorEvents::GetInstance( )

class SSEditorEvents : public Subsystem
{
public:

	static SSEditorEvents& GetInstance();

	void Startup() override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown() override;

private:
	// No external instancing allowed
	SSEditorEvents() : Subsystem( "EditorEvents" ) {}
	SSEditorEvents( const SSEditorEvents& rhs );
	~SSEditorEvents() {};
	SSEditorEvents& operator=( const SSEditorEvents& rhs );


	const rString m_WindowName = "EventsWindow";
	GUI::ComboBox* m_EventTypeComboBox;
	GUI::ComboBox* m_EventEffectComboBox;


	//"Dynamic"
	GUI::Window* m_TimeInputWindow;
	GUI::TextBox* m_TimeInput;

	GUI::Button* m_ColourResult;
};
