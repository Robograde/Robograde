/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "Logger.h"
#include "ConfigManager.h"
#include <mutex>

#define LOG_CFG_PATH_FILE "logging.cfg"

namespace Logger
{
	std::map<std::string, LogType> LogTypes; // Cannot be rMaps since memory allocator is initialized after logger since logger is a namespace only // TODODP: See if anything can be done about this
	std::map<std::string, InterestEntry> InterestedLogTypes;

	bool TimeStamp = false;
	bool HighPrecisionStamp = false;
	unsigned int TreeHeight = 1;

	std::string NameSeparator = ".";
	bool IWantItAllFlag = false;

	Uint64 StartTime = SDL_GetPerformanceCounter();

	rString GetParentString ( const rString& name, unsigned int treeHeight );
	
	std::stringstream OutputStream;
	
	std::stringstream& GetStream ( ) { return OutputStream; }
	
	std::mutex Mutex;
}
/// <summary>
/// Initializes the logger by reading the config file LOG_CFG_PATH_FILE.
/// <para>This should be called after registering types.</para>
/// </summary>
void Logger::Initialize()
{
	CallbackConfig* cfg = g_ConfigManager.GetConfig( LOG_CFG_PATH_FILE );

	HighPrecisionStamp = cfg->GetBool ( "HighPrecisionStamp", false );
	TreeHeight = static_cast<unsigned int> ( cfg->GetInt ( "TreeHeight", 3 ) );
	NameSeparator = cfg->GetString ( "NameSeparator", "." ).c_str( );
	IWantItAllFlag = cfg->GetBool ( "IWantItAll", true );
	if ( IWantItAllFlag )
	{
		IWantItAll();
	}
}

/// <summary>
/// Cleanup logger structures.
/// </summary>
void Logger::Cleanup( )
{
	LogTypes.clear( );
	InterestedLogTypes.clear( );
}

/// <summary>
/// Reads a file containing types of logging messages that the user is interested in.
/// <para>If a path is not supplied it will read the path from the config file
/// defined in LOG_CFG_PATH_FILE.</para>
/// <para>Will not be run if IWantItAll setting is true</para>
/// </summary>
/// <param name="path">Path to the file to be read (optional)</param>
void Logger::RegisterInterestFromFile ( const rString& path )
{
	if ( IWantItAllFlag )
	{
		return;
	}
	rString pathToLoad = path;
	// Read path from file if none was supplied
	if ( path == "" )
	{
		CallbackConfig* cfg = g_ConfigManager.GetConfig( LOG_CFG_PATH_FILE );
		if ( cfg != nullptr )
		{
			pathToLoad = cfg->GetString ( "InterestFile" );
		}
		else
		{
			cerr << Colors::BRed << "Failed to read logging config file: " << LOG_CFG_PATH_FILE << Colors::Reset
				 << endl;
			return;
		}
	}
	CallbackConfig* cfgToLoad = g_ConfigManager.GetConfig( pathToLoad );
	if ( cfgToLoad != nullptr )
	{
		rMap<rString, Config::ConfigEntry*>* m = cfgToLoad->GetScopeMap ( "LogInterests" );
		if ( m )
		{
			// Loop through every map entry in the config
			for ( auto& entry : *m )
			{
				RegisterInterest ( entry.first, entry.second->Value.IntVal );
			}
			Log( "Read logging interest file: " + pathToLoad, "Logger", LogSeverity::INFO_MSG );
		}
		else
		{
			cerr << Colors::BRed << "Failed to read logging interest file: " << path
				 << Colors::Reset << endl;
		}
	}
	else
	{
		std::cerr << "Failed to parse logger interest config file: " << path << std::endl;
	}
}

/// <summary>
/// Registers a type of logging message.
/// <para>This function should be run once for each type of message the programmer creates.</para>
/// <para>If a parent is supplied the supplied <paramref name="name"/> will be a child to this parent. If a parent
/// is registered with <see cref="Logger::RegisterInterest"/> the childs logging messages will
/// also be seen.</para>
/// <para><paramref name="parent"/> can be left empty if a parent is not desired.</para>
/// <remarks>The parent name supplied with <paramref name="parent"/> needs to be registered before
/// any children can be added</remarks>
/// </summary>
/// <param name="name">Name of the logging message</param>
/// <param name="parent">Parent of the supplied name (optional)</param>
void Logger::RegisterType ( const rString& name, const rString& parent )
{
	if ( parent != "" )
	{
		// If a parent is supplied, find it
		rMap<std::string, LogType>::iterator it = LogTypes.find ( parent.c_str( ) );

		// Check if parent was found
		if ( it != LogTypes.end() )
		{
			// Create LogType, emplace it and record the child in the parent.
			LogType lt;
			lt.Parent = parent;
			LogTypes.emplace ( name.c_str( ), lt );
			it->second.Children.push_back ( name );
		}
		else
		{
			cerr << Colors::BRed << "Failed to find parent log type for log type: " << name << Colors::Reset <<
				 endl;
			return;
		}
	}
	else
	{
		// No parent was supplied so create LogType with no parent.
		LogType lt;
		lt.Parent = "";
		LogTypes.emplace ( name.c_str( ), lt );
	}
}

/// <summary>
/// Registers interest in a type of logging message that has been registered with
/// <see cref="Logger::RegisterType"/>.
/// <para>If a parent is registered all its children will also be registered</para>
/// </summary>
/// <param name="name">Path to the file to be read</param>
/// <param name="severityMask">Bitmask of type <see cref="LogSeverity::BitFlag"/></param>
void Logger::RegisterInterest ( const rString& name, int severityMask )
{
	rMap<std::string, LogType>::iterator it = LogTypes.find ( name.c_str( ) );
	if ( it != LogTypes.end() )
	{
		// Create interest entry with supplied severitymask and name.
		InterestEntry ie;
		ie.Parent = it->second.Parent;
		ie.SeverityMask = severityMask;
		InterestedLogTypes.emplace ( name.c_str( ), ie );
		// Add all children as well.
		for ( rVector<rString>::iterator chit = it->second.Children.begin();
			  chit != it->second.Children.end(); ++chit )
		{
			RegisterInterest ( *chit, severityMask );
		}
	}
	else
	{
		cerr 	<< Colors::BRed << "Failed to find log type " << name
				<< " interest not registered." << Colors::Reset << endl;
	}
}

/// <summary>
/// Registers interest in all types of registered logging messages.
/// </summary>
/// <param name="severityMask">Bitmask of type <see cref="LogSeverity::BitFlag"/></param>
void Logger::IWantItAll ( int severityMask )
{
	for ( auto& kv : LogTypes )
	{
		InterestEntry ie;
		ie.Parent = kv.second.Parent;
		ie.SeverityMask = severityMask;
		InterestedLogTypes.emplace ( kv.first, ie );
	}
}

/// <summary>
/// Logs a message in the logger.
/// <para>Logs a message in the logger with the selected type, message and severity.</para>
/// <para>Use <see cref="Logger::RegisterType"/> to register types for use with the logger.</para>
/// <para>Use <see cref="Logger::RegisterInterest"/> to register interest in a type of
/// log message that has been registered.</para>
/// </summary>
/// <param name="message">A string containing the message to be displayed</param>
/// <param name="type">A string defining what type of message it is. (ex. Network or Game)</param>
/// <param name="logSeve">The severity of the message to be logged</param>
void Logger::Log ( const rString& message, const rString& type, LogSeverity::BitFlag logSev )
{
	rMap<std::string, InterestEntry>::const_iterator cit = InterestedLogTypes.find ( type.c_str( ) );
	// Don't log things we are not interested in
	if ( cit == InterestedLogTypes.end() )
	{
		return;
	}
	else if ( ! ( cit->second.SeverityMask & logSev ) )
	{
		return;
	}

	rOStringStream oss;
	rOStringStream oss2; //Used by the other output stream

	if ( TimeStamp )
	{
		// TODOJM, Implement
	}
	// TODOJM dicuss good way to make threadsafe cout printing
	if ( HighPrecisionStamp )
	{
		oss << Colors::Yellow << SDL_GetPerformanceCounter() - StartTime << " ";
		oss2 << "<C=YELLOW>" << SDL_GetPerformanceCounter() - StartTime << " ";
	}

	switch ( logSev )
	{
		case LogSeverity::INFO_MSG:
		{
			oss << Colors::White << "INFO ";
			oss2 << "[C=WHITE]" << "INFO ";
			break;
		}
		case LogSeverity::WARNING_MSG:
		{
			oss << Colors::BPurple << "WARNING ";
			oss2 << "[C=PURPLE]" << "WARNING ";
			break;
		}
		case LogSeverity::ERROR_MSG:
		{
			oss << Colors::BRed << "ERROR ";
			oss2 << "[C=RED]" << "ERROR ";
			break;
		}
		case LogSeverity::DEBUG_MSG:
		{
			oss << Colors::Green << "DEBUG ";
			oss2 << "[C=GREEN]" << "DEBUG ";
			break;
		}
		case LogSeverity::ALL:
		{
			oss << "WRONG_SEVERITY_DO_NOT_USE ";
			oss2 << "WRONG_SEVERITY_DO_NOT_USE ";
			break;
		}
	}
	oss << GetParentString ( cit->first.c_str( ), TreeHeight ) << ": " << message << Colors::Reset << endl;
	cout << oss.str();

	
	Mutex.lock(); //Needed because it doesn't seem to be threadsafe. It crashes randomly at startup on Linux.
	oss2 << GetParentString ( cit->first.c_str( ), TreeHeight ) << ": " << message << endl;
	OutputStream << oss2.str();
	Mutex.unlock();
}

rString Logger::GetParentString ( const rString& name, unsigned int treeHeight )
{
	if ( treeHeight == 0 )
	{
		return "";
	}
	rMap<std::string, LogType>::const_iterator pit = LogTypes.find ( name.c_str( ) );
	if ( pit == LogTypes.end() )
	{
		return name;
	}
	else if ( pit->second.Parent == "" )
	{
		return name;
	}
	return GetParentString ( pit->second.Parent, treeHeight - 1 ) + NameSeparator.c_str( ) + name;
}
