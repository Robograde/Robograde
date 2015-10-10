#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSEditorMain SSEditorMain::GetInstance( )

class SSEditorMain : public Subsystem
{
public:

	static SSEditorMain& GetInstance();

	void Startup() override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown() override;

private:
	// No external instancing allowed
	SSEditorMain() : Subsystem( "EditorMain" ) {}
	SSEditorMain( const SSEditorMain& rhs );
	~SSEditorMain() {};
	SSEditorMain& operator=( const SSEditorMain& rhs );

	int EditorMove  ( IScriptEngine* scriptEngine );
	int EditorAdd	( IScriptEngine* scriptEngine );
	int EditorRemove ( IScriptEngine* scriptEngine );

	glm::ivec2		m_ToolBoxWindowSize = glm::ivec2( 336, 160 );
	int				m_BottomBarHeight = 160;
	int				m_ToolboxIconSize = 48;


	GUI::Button*	m_MoveButton;
	GUI::Button*	m_AddButton;
	GUI::Button*	m_RemoveButton;
	GUI::Button*	m_UndoButton;
	GUI::Button*	m_RedoButton;
	GUI::Button*	m_NoteButton;
	GUI::Button*	m_SaveButton;
};