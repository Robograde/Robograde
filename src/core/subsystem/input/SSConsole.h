/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include <gui/object/Text.h>

#define g_SSConsole SSConsole::GetInstance()

#define MAX_CONSOLE_MESSAGES 1000

struct CommandArg
{
	CommandArg( rString desc )
	{
		Desc = desc;
	}

	rString Desc;
};

struct Command
{
	Command( rString desc, rString usage )
	{
		Desc = desc;
		Usage = usage;
	}
	
	Command( rString desc )
	{
		Desc = desc;
	}

	rString Desc;
	rString Usage;
	rMap<rString, CommandArg> Args;
	
	rVector<rString> Aliases;
	
};



class SSConsole : public Subsystem
{
public:
	static SSConsole& GetInstance( );
	void Startup() override;
	
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown() override;
	
	bool Toggle( );
	
	void PrintLine( rString output );
	void SaveHistory( ) const;
	void ReadHistory( );
	
private:
	SSConsole( ) : Subsystem( "Console" ) {}
	SSConsole( const SSConsole& rhs );
	~SSConsole( ) {}
	SSConsole& operator = (const SSConsole& rhs);
	
	int ToggleSF( IScriptEngine* scriptEngine );
	
	void			HandleTextInput();
	void			PrintCommandHelp( const rString& input );
	void			InitializeCommands();
	void			HandleCommand( rVector<rString>& input );

	const rString	m_HistoryFileName = "CommandHistory.txt";
	const int		m_MaximumHistoryEntriesInFile = 1000;
	
	pDeque<rString> m_Messages;
	
	pVector<GUI::Text*> m_Texts;
	
	pVector<rString> m_History;
	
	pMap<rString, Command> m_Commands;
	
	GUI::TextBox*	m_TextBox;
	
	//bool			m_Shown = false;
	unsigned int	m_ScrollOffset = 0;
	
	int				m_HistoryIndex = 0;
	
	float			m_TimeSinceLastMove = 1.0f;
};
