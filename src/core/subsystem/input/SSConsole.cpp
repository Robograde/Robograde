/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSConsole.h"
#include <algorithm>
#include <cctype> // For std::tolower
#include <script/ScriptEngine.h>
#include <utility/Logger.h>
#include <input/Input.h>
#include <profiler/Profiler.h>
#include <network/NetworkInfo.h>
#include "SSInput.h"
#include "../SubsystemManager.h"
#include "../menu/SSGameLobby.h"
#include "../network/SSNetworkController.h"
#include "../utility/SSMemoryDebug.h"
#include "../../utility/GameModeSelector.h"
#include "../input/SSKeyBinding.h"
#include <input/TextInput.h>

SSConsole& SSConsole::GetInstance( )
{
	static SSConsole instance;
	return instance;
}

void SSConsole::Startup( )
{
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	
	//Here be magic numbers!
	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );
	int consoleHeight = 417;
	int consolePosY = 0;
	g_GUI.AddWindow( "ConsoleWindow", GUI::Rectangle( 0, 0, windowSize.x, consoleHeight + 27 ), "RootWindow", true );
	g_GUI.SetWindowClickThrough( "ConsoleWindow", true );
	
	//Set the background
	GUI::SpriteDefinition spriteDef( "", 0, 0, windowSize.x, consoleHeight + 5, glm::vec4( 0.0f, 0.0f, 0.0f, 0.3f ) );
	g_GUI.AddSprite( "ConsoleBg", spriteDef, "ConsoleWindow" );
	
	m_TextBox = g_GUI.AddTextBox( "ConsoleInput", GUI::Rectangle( 15, consoleHeight + 5, windowSize.x - 15, 22 ), "ConsoleWindow" );
	g_GUI.AddText( "ConsoleInputChar", GUI::TextDefinition(">", 5, consoleHeight + 10 ), "ConsoleWindow" );
	
	m_ScrollOffset = 0;
	m_HistoryIndex = 0;
	
	//Set the positions of all the textDefinitions that are going to display the console text
	int posY = consoleHeight - 15;
	int i = 0;
	while( posY > consolePosY )
	{
		GUI::TextDefinition textDef( "", 5, posY, glm::vec4( 1.0f ) );
		GUI::Text* text = g_GUI.AddText( "Text" + rToString( i ), textDef, "ConsoleWindow" );
		m_Texts.push_back( text );
		posY -= 17;
		i++;
	}
	
	InitializeCommands();
	
	PrintLine( "Type [C=BLUE]help[C=WHITE] to see all available commands." );
	PrintLine( "-----" );
	
	g_Script.Register( "ToggleConsole", std::bind( &SSConsole::ToggleSF, this, std::placeholders::_1 )  );

	ReadHistory( );
}

void SSConsole::UpdateUserLayer( const float deltaTime )
{
	//Get the new messages from Logger
	rString line;
	while( std::getline( Logger::GetStream() , line))
	{
		PrintLine( line );
	}
	Logger::GetStream().clear();
	
	//Only do this when shown, tis a hack
	if( g_GUI.IsWindowOpen( "ConsoleWindow" ) )
	{
		
		
		//For speedy console scrolling
		float speedMod = 1.0f;
		int scrollSteps = 1;
		if( g_Input->KeyDown( SDL_SCANCODE_LSHIFT, true ) )
		{
			 speedMod = 0.2f;
			 scrollSteps = 4;
		}
		if( g_Input->IsPaused() )
			m_ScrollOffset += g_Input->GetScrollY( true ) * scrollSteps;
		
		if( m_TimeSinceLastMove >= 0.1f * speedMod )
		{
			//Console scrolling
			if( g_Input->KeyDown( SDL_SCANCODE_PAGEUP, true ) )
			{
				m_ScrollOffset += 1;
				m_TimeSinceLastMove = 0.0f;
			}
			else if( g_Input->KeyDown( SDL_SCANCODE_PAGEDOWN, true ) )
			{
				m_ScrollOffset = ( m_ScrollOffset > 0) ? m_ScrollOffset - 1 : 0;
				m_TimeSinceLastMove = 0.0f;
			}
			
			//History browsing
			int historySize = static_cast<int>( m_History.size() );
			if( m_History.size() > 0 &&  g_Input->IsPaused() )
			{
				if( g_Input->KeyUpDown( SDL_SCANCODE_UP, true ) )
				{
					m_HistoryIndex = ( historySize - m_HistoryIndex > 0 ) ? m_HistoryIndex + 1 : m_HistoryIndex;
					m_TextBox->SetText( ( m_HistoryIndex != 0 ) ? m_History[ historySize - m_HistoryIndex] : "" );
					m_TimeSinceLastMove = 0.0f;
				} 
				else if( g_Input->KeyUpDown( SDL_SCANCODE_DOWN, true ) )
				{
					if( historySize - m_HistoryIndex < historySize ) 
					{
						m_HistoryIndex -= 1;
						m_TextBox->SetText( ( m_HistoryIndex != 0 ) ? m_History[ historySize - m_HistoryIndex] : "" );
					}
					
					m_TimeSinceLastMove = 0.0f;
				}
			}
		}
		m_TimeSinceLastMove += deltaTime;
		
		if( !m_TextBox->IsInputting() && m_TextBox->GetText() == "" )
		{
			
			m_TextBox->GetTextDefinitionRef().Text = "Press {ENTER} to start inputting.";
			m_TextBox->GetTextDefinitionRef().Colour = glm::vec4( 0.0, 0.6, 0.8f, 0.8f );
		}
		
		if( !m_TextBox->IsInputting() )
		{
			if( g_Input->KeyUpDown( SDL_SCANCODE_RETURN ) || g_Input->KeyUpDown( SDL_SCANCODE_KP_ENTER ) )
			{
				m_TextBox->StartInput();
				g_Input->ConsumeKeys();
			}
		}
		else
		{
			if( g_Input->KeyUpDown( SDL_SCANCODE_RETURN, true ) || g_Input->KeyUpDown( SDL_SCANCODE_KP_ENTER, true ) )
			{
				if( m_TextBox->GetText() != "" )
				{
					m_Messages.push_back( "[C=BLUE]-> [C=WHITE]" + m_TextBox->GetText() );
					HandleTextInput();
				}
				
				m_TextBox->StopInput();
				m_TextBox->SetText( "" );

				g_Input->ConsumeKeys();
			}
		}
		
		//Update the text objects
		int i = 0;
		for( GUI::Text* text : m_Texts)
		{
			GUI::TextDefinition& textDef = text->GetTextDefinitionRef();
			unsigned int amount = static_cast<int>( m_Messages.size() );
			
			textDef.Text = ( amount > amount - 1 - i - m_ScrollOffset ) ? 
						m_Messages[ amount - 1 - i - m_ScrollOffset].c_str() : "";
			i++;
		}
	}
	if( g_SSKeyBinding.ActionUpDown( ACTION_TOGGLE_CONSOLE, true ) )
	{
		g_GUI.BringWindowToFront( "ConsoleWindow" );
		Toggle();
	}
	if( g_GUI.IsWindowOpen( "ConsoleWindow" ) )
	{
		if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
		{
			Toggle();
		}
	}
}

void SSConsole::Shutdown( )
{
	SaveHistory( );
	g_GUI.DeleteObject( "ConsoleWindow" );
	m_Texts.clear();
	m_Messages.clear();
	m_History.clear();
	m_Commands.clear();
	m_HistoryIndex = 0;
	m_ScrollOffset = 0;
}

bool SSConsole::Toggle( )
{
	m_TextBox->SetText( "" );
	
	if( !g_GUI.IsWindowOpen( "ConsoleWindow" ) )
	{
		g_GUI.OpenWindow( "ConsoleWindow" );
		m_TextBox->StartInput();
		//g_Input->PauseKeyInput();
		m_TextBox->SetText( "" );
		return true;
	}
	else
	{
		g_GUI.CloseWindow( "ConsoleWindow" );
		m_TextBox->StopInput();
		//g_Input->UnPauseKeyInput();
		return false;
	}
}

int SSConsole::ToggleSF( IScriptEngine* scriptEngine )
{
	scriptEngine->SetBool( "toggle", Toggle() );
	return 0;
}

void SSConsole::PrintLine(rString output)
{
	m_Messages.push_back( output );
	
	if( m_Messages.size() > MAX_CONSOLE_MESSAGES )
		m_Messages.pop_front();
}

void SSConsole::HandleTextInput( )
{
	rString& text = m_TextBox->GetText();
	rString tText = text;
	
	//Keeping old commands
	m_History.push_back( tText );
	m_HistoryIndex = 0;
	m_ScrollOffset = 0;
	
	
	//Getting input and putting it in a vector
	rVector<rString> input;
	int spacePos = static_cast<int>( tText.find( ' ' ) );
	if( spacePos != rString::npos )
	{
		while( spacePos != rString::npos )
		{
			spacePos = static_cast<int>( tText.find( ' ' ) );

			input.push_back( tText.substr( 0, spacePos ) );

			tText = tText.substr( spacePos + 1, tText.size() - 1 );
		}
	}
	else
	{
		input.push_back( tText );
		input.push_back( "" );
	}
	if( input.size() == 1 )
		input.push_back( "" );
	
	//Everything before first space is in input[0] and everything after is parameters
		
	std::transform( input[0].begin(), input[0].end(), input[0].begin(), ::tolower );
	
	bool success = false;
	if( m_Commands.find( input[0] ) != m_Commands.end() )
	{
		HandleCommand( input );
		success = true;
	}
	else
	{
		for( auto commandsIt : m_Commands )
		{
			for( rString alias : commandsIt.second.Aliases )
			{
				if( alias == input[0] )
				{
					input[0] = commandsIt.first;
					HandleCommand( input );
					success = true;
				}
			}
		}
	}
	if( !success )
		g_Script.Perform( text.c_str() );
		//PrintLine( "[C=RED]Did not recognize command \"" + input[0] + "\"" );
		
}

void SSConsole::PrintCommandHelp( const rString& input )
{
	PrintLine( "" );
	
	auto printHelp = [&] ( const rString& name, const Command& cmd )
	{
		PrintLine( "[C=GREEN]------------" );
		PrintLine( "Command: [C=BLUE]" + name );
		//PrintLine( "[C=GREEN]------------" );
		PrintLine( "Description: " + cmd.Desc );
		if( cmd.Usage != "" )
			PrintLine( "Usage: " + cmd.Usage );
		PrintLine( "[C=GREEN]------------" );
		PrintLine( "Args:" );
		for( auto argIt : cmd.Args )
		{
			PrintLine( " [C=PURPLE]" + argIt.first );
			PrintLine( "   Description: " + argIt.second.Desc );
			//PrintLine( "[C=GREEN]------------" );
		}
		
	};

	if( input == "" ) //I want it all!
	{
		PrintLine( "[C=GREEN]------------" );
		PrintLine( "[C=GREEN]Command list" );
		PrintLine( "[C=GREEN]------------" );
		for( auto& commandIt : m_Commands )
		{
			Command cmd = commandIt.second;
			
			PrintLine( "Command: [C=BLUE]" +  commandIt.first );
			PrintLine( "Description: " + cmd.Desc );
			if( cmd.Usage != "" )
				PrintLine( "Usage: " + cmd.Usage );
			
			rString args = "";
			
			for( auto argIt : cmd.Args )
			{
				args += argIt.first + ", ";
			}
			if( cmd.Args.size() > 0 )
					PrintLine( "Args: [C=PURPLE]" + args );
			
			PrintLine( "[C=GREEN]------------" );
			
			//printHelp( commandIt.first, commandIt.second );
		}
		PrintLine( "" );
		PrintLine( "" );
		PrintLine( "[C=GREEN]------------" );
		PrintLine( "[C=WHITE]You can see all LUA commands by typing [C=BLUE]List()" );
		PrintLine( "[C=GREEN]------------" );
	}
	else
	{
		if( m_Commands.find( input ) != m_Commands.end() )
			printHelp( input, m_Commands.at( input ) );
		else
		{
			bool found = false;
			for( auto& command : m_Commands )
			{
				for( rString& alias : command.second.Aliases )
				{
					if( alias == input )
					{
						printHelp( input, command.second );
						found = true;
					}
				}
				
			}
			if( !found )
				PrintLine( "[C=RED]There is no help available for command \"" + input + "\"." );
		}
			
	}
	
	PrintLine( "" );
}

// ---------- Don't touch above this ----------- \\

void SSConsole::InitializeCommands()
{
	Command help = Command( "Display help text", "help, help <command>" );
	help.Aliases.push_back( "h" );
	m_Commands.emplace( "help", help );
	
	Command clear = Command( "Clears the console", "clear" );
	clear.Aliases.push_back( "cls" );
	clear.Aliases.push_back( "c" );
	m_Commands.emplace( "clear", clear );
	
	Command quit = Command( "Shuts down the game" );
	quit.Aliases.push_back( "exit" );
	quit.Aliases.push_back( "shutdown" );
	quit.Aliases.push_back( "q" );
	m_Commands.emplace ( "quit", quit );
	
	Command lua = Command( "Runs lua code", "lua <Lua>" );
	m_Commands.emplace( "lua", lua );
	
	Command luaf = Command( "Runs a lua file", "lua <filename>" );
	m_Commands.emplace( "luaf", luaf );
	
	Command time = Command( "Prints profiler data." );
	time.Args.emplace( "avg", CommandArg( "Prints the average time of different systems." ) );
	time.Args.emplace( "max", CommandArg( "Prints the maximum time of different systems." ) );
	m_Commands.emplace( "time", time );
	
	Command mem = Command( "Shows the memory usage" );
	m_Commands.emplace( "mem", mem );
	
	Command host = Command( "Makes this machine host" );
	host.Args.emplace( "<port>", CommandArg( "Hosts a game using the inputed port" ) );
	host.Aliases.push_back( "host" );
	m_Commands.emplace( "host", host );
	
	Command connect = Command( "Establishes network connection" );
	connect.Args.emplace( "<IP Address>", CommandArg( "Connects to the specific IP Address" ) );
	host.Args.emplace( "<port>", CommandArg( "Connects to a game using the inputed port" ) );
	m_Commands.emplace( "connect", connect );

	Command startSS = Command( "Start subsystem" );
	startSS.Args.emplace( "<Name>", CommandArg( "Starts the subsystem with the inputed name" ) );
	m_Commands.emplace( "startss", startSS );

	Command gameMode = Command( "Switch to inputed game mode" );
	gameMode.Args.emplace( "<Game Mode>", CommandArg( "Switches to the inputed game mode" ) );
	gameMode.Aliases.push_back( "m" );
	gameMode.Aliases.push_back( "mdoe" ); // GOD DAMNIT!
	m_Commands.emplace( "mode", gameMode );
	
	//Add new commands here first
}

void SSConsole::HandleCommand( rVector<rString>& input )
{
	//Inportant notes: input will always have at least input[0] = command 
	//	and input[1] = "" (Unless you use a command that have parameters, then those will be put in input[1] and onwards)

	//Put your own stuff here
	if( input[0] == "help" )
	{
		PrintCommandHelp( input[1] );
	}
	else if( input[0] == "clear" )
	{
		m_Messages.clear();
		m_ScrollOffset = 0;
	}
	else if( input[0] == "quit" )
	{
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent ( &event );
	}
	else if( input[0] == "lua" )
	{
		if( input.size() > 1 )
			g_Script.Perform( input[1].c_str() );
	}
	else if ( input[0] == "luaf" )
	{
		if( input.size() > 1 )
			g_Script.Run( input[1].c_str() );
	}
	else if ( input[0] == "mem" )
	{
		Logger::GetStream( ) << g_SSMemoryDebug.GetMemoryUsagePerFile( );
	}
	else if ( input[0] == "time" )
	{
		if ( input[1] == "" || input[1] == "avg" )
			Logger::GetStream( ) << g_Profiler.GetFormattedAveragesMilliSeconds( );
		else if ( input[1] == "max" )
			Logger::GetStream( ) << g_Profiler.GetFormattedMaxMilliSeconds( );
		else
			Logger::GetStream() << "Invalid argument for command profile. Only avg or max is accepted";
	}
	else if ( input[0] == "host")
	{
		unsigned short listeningPort;
		if ( input[1] != "" )
		{
			listeningPort = NetworkUtility::GetPortFromString( input[1] );
			if ( listeningPort == 0 )
			{
				Logger::Log( "Invalid port supplied to host command", "SSConsole", LogSeverity::WARNING_MSG );
				return;
			}
		}
		else
			listeningPort = 0;

		if ( g_SSNetworkController.MakeHost( listeningPort ) )
			Logger::Log( "Host started", "SSConsole", LogSeverity::INFO_MSG );
		else
			Logger::Log( "Failed to start host", "SSConsole", LogSeverity::ERROR_MSG );
	}
	else if ( input[0] == "connect" )
	{
		if ( !g_NetworkInfo.AmIHost() )
		{
			rString IPAddress = "";
			unsigned short port = INVALID_PORT;

			if ( input.size() == 3 )
			{
				IPAddress	= NetworkUtility::GetIPFromString( input[1] );
				port		= NetworkUtility::GetPortFromString( input[2] );
			}
			else
				NetworkUtility::GetIPAndPortFromString( input[1], IPAddress, port );

			if ( IPAddress == "" )
				IPAddress = LOCALHOST_IP;
			if ( port == INVALID_PORT )
				port = g_SSNetworkController.GetDefaultListeningPort();

			if ( g_SSNetworkController.ConnectToGame( IPAddress, port ) )
				Logger::Log( "Connection request added", "SSConsole", LogSeverity::INFO_MSG );
			else
				Logger::Log( "Failed to add connection request", "SSConsole", LogSeverity::ERROR_MSG );
		}
		else
			Logger::Log( "Invalid port supplied to connect command", "SSConsole", LogSeverity::WARNING_MSG );
	}
	else if ( input[0] == "startss" )
	{
		if (input[1] == "all")
			g_SubsystemManager.StartAllUnstarted( );
		else if ( input[1] != "" )
			g_SubsystemManager.StartSubsystem( input[1] );
		else
			Logger::Log( "No parameter supplied to startss command", "SSConsole", LogSeverity::WARNING_MSG );
	}
	else if ( input[0] == "mode" )
	{
		if ( input[1] != "" )
		{
			rString lowerCaseParameter = input[1];
			std::transform( lowerCaseParameter.begin(), lowerCaseParameter.end(), lowerCaseParameter.begin(), ::tolower );
			g_GameModeSelector.SwitchToGameModebyString(  lowerCaseParameter );
		}
		else
			Logger::Log( "No parameter supplied to mode command", "SSConsole", LogSeverity::WARNING_MSG );
	}
}

void SSConsole::SaveHistory( ) const
{
	std::ofstream history( m_HistoryFileName.c_str(), std::ios::out );
	if ( history.is_open( ) )
	{
		int c = 0;
		// Only save maximum number of entries
		for ( pVector<rString>::const_iterator it = m_History.end( ) - std::min(static_cast<int>(m_History.size()), m_MaximumHistoryEntriesInFile);
			it != m_History.end( ) && c < m_MaximumHistoryEntriesInFile; ++it )
		{
			history << *it << std::endl;
			c++;
		}
		history.close( );
	}
	else
	{
		Logger::Log( "Failed to save history file: " + m_HistoryFileName + ".", "SSConsole", LogSeverity::WARNING_MSG );
	}
}

void SSConsole::ReadHistory( )
{
	std::ifstream history( m_HistoryFileName.c_str( ), std::ios::in );
	rString line;
	if ( history.is_open() )
	{
		while ( history.good() && !history.eof() && getline( history, line ) )
		{
			m_History.push_back( line );
		}
	}
	history.close( );
}
