/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "Config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include "Logger.h"

using std::ios;
using std::ifstream;
using std::cerr;
using std::endl;
using std::regex;
using std::regex_replace;

Config::Config( )
{
}

Config::~Config( )
{
	for( rMap<rString, ConfigEntry*>::iterator it = m_configs.begin( ); it != m_configs.end( ); ++it )
	{
		if( it->second != nullptr )
		{
			pDelete( it->second );
			it->second = nullptr;
		}
	}
}

bool Config::ReadFile( const rString& path )
{
	ifstream file( path.c_str( ), ios::in );
	rString fileString = "";
	if( file )
	{
		// Load whole file into string
		file.seekg( 0, std::ios::end );
		fileString.resize( file.tellg( ) );
		file.seekg( 0, std::ios::beg );
		file.read( &fileString[0], fileString.size( ) );
		file.close( );

		if( Parse( fileString, &m_configs ) )
		{
			Logger::Log( "Config file: " + path + " successfully read", "Config", LogSeverity::INFO_MSG );
			return true;
		}
		else
		{
			Logger::Log( "Failed to parse config file with path: " + path, "Config", LogSeverity::ERROR_MSG );
			return false;
		}
	}
	else
	{
		// Logger::Log( "Failed to load file: " + path, "Config", LogSeverity::ERROR_MSG );
		return false;
	}
}

void Config::AppendEntry( std::ostream& saveStream, const rString& key, const ConfigEntry& entry, unsigned int indention )
{
	rString indentString = "";
	for( unsigned int i = 0; i < indention; ++i )
		indentString += "\t";

	if ( entry.Comment.size() > 0 )
	{
		rString commentString = "";
		rIStringStream commentStream( entry.Comment );
		rString val;
		while ( getline( commentStream, val, '\n' ) )
		{
            commentString += indentString + ';' + val + "\n";
		}
		saveStream << commentString;
	}

	saveStream << indentString;
	saveStream << key << " : ";
	switch( entry.Type )
	{
		case Config::Type::Double:
		{
			saveStream << entry.Value.DoubleVal;
			// Make sure to write something that will be parsed as a double
			if( entry.Value.DoubleVal == std::floor( entry.Value.DoubleVal ) )
				saveStream << ".0";
		}
		break;
		case Config::Type::Int:
		{
			saveStream << entry.Value.IntVal;
		}
		break;
		case Config::Type::Bool:
		{
			saveStream << ( entry.Value.BoolVal ? "true" : "false" );
		}
		break;
		case Config::Type::String:
		{
			saveStream << "\"" << entry.Value.StringVal << "\"";
		}
		break;
		case Config::Type::Map:
		{
			saveStream << std::endl << "{" << std::endl;
			SaveMap( saveStream, entry.Value.Map, indention + 1 );
			saveStream << "}";
		}
		break;
		case Config::Type::Array:
		{
			saveStream << "[";
			if( entry.Value.Array->size( ) > 0 )
			{
				auto lastComma = std::prev( entry.Value.Array->end( ) );
				for( auto arrayEntry = entry.Value.Array->begin( ); arrayEntry != entry.Value.Array->end( ); ++arrayEntry )
				{
					switch( ( *arrayEntry )->Type )
					{
						case Config::Type::Double:
						{
							saveStream << ( *arrayEntry )->Value.DoubleVal;
							// Make sure to write something that will be parsed as a double
							if( entry.Value.DoubleVal == std::floor( entry.Value.DoubleVal ) )
								saveStream << ".0";
						}
						break;
						case Config::Type::Int:
						{
							saveStream << ( *arrayEntry )->Value.IntVal;
						}
						break;
						case Config::Type::Bool:
						{
							saveStream << ( ( *arrayEntry )->Value.BoolVal ? "true" : "false" );
						}
						break;
						case Config::Type::String:
						{
							saveStream << "\"" << ( *arrayEntry )->Value.StringVal << "\"";
						}
						break;
						// Map and array not supported
						default:
						{
						}
						break;
					}
					if( arrayEntry != lastComma )
						saveStream << ", ";
				}
			}
			saveStream << "]";
		}
		break;
	}
	saveStream << "," << std::endl;
}

bool Config::SaveFile( const rString& filePath )
{
	std::ofstream saveStream;
	saveStream.open( filePath.c_str( ) );
	SaveMap( saveStream, &m_configs );
	m_Dirty = false;
	return true;
}

bool Config::SaveMap( std::ostream& saveStream, const rMap<rString, ConfigEntry*>* mapToSave, unsigned int indention )
{
	for( auto& entry : *mapToSave )
	{
		AppendEntry( saveStream, entry.first, *entry.second, indention );
	}
	return true;
}

bool Config::PutEntry( const rString& key, ConfigEntry* entry )
{
	rString keyToUse = key;
	size_t dotPlace = key.find_last_of( "." );
	rMap<rString, ConfigEntry*>* toAddTo = &m_configs;
	if( dotPlace != std::string::npos )
	{
		toAddTo = CreateNeededMaps( key.substr( 0, dotPlace ) );
		if( toAddTo == nullptr )
		{
			Logger::Log( "Failed to put entry with key: " + key, "Config", LogSeverity::ERROR_MSG );
			return false;
		}
		// Strip away map parts
		keyToUse = key.substr( dotPlace + 1, key.size( ) - dotPlace );
	}
	auto mapIt = toAddTo->find( keyToUse );
	if( mapIt == toAddTo->end( ) )
	{
		toAddTo->emplace( keyToUse, entry );
	}
	else
	{
		// Complain about entry already existing
		Logger::Log( "Config entry with key: " + key + " already exists", "Config", LogSeverity::ERROR_MSG );
		return false;
	}
	return true;
}

rMap<rString, Config::ConfigEntry*>* Config::CreateNeededMaps( const rString& keyMapPart )
{
	// Create all needed maps
	rString val;
	rString currentScopePath = "";
	rIStringStream iss( keyMapPart );
	rMap<rString, ConfigEntry*>* toAddTo = &m_configs;
	while( getline( iss, val, '.' ) )
	{
		currentScopePath += val;
		auto mapIt = toAddTo->find( val );
		if( mapIt == toAddTo->end( ) )
		{
			ConfigEntry* ce = pNew( ConfigEntry );

			rMap<rString, ConfigEntry*>* newMap;
// TODODP: h4x!!! Make prettier with a proper new method or macro!!
#if !defined( DISABLE_ALLOCATOR )
#ifdef DEBUG_MEMORY
			newMap = MemoryAllocator::Construct<rMap<rString, ConfigEntry*>>( 1ULL, true, __FILE__, __LINE__ );
#else
			newMap = MemoryAllocator::Construct<rMap<rString, ConfigEntry*>>( 1ULL );
#endif
#else
			newMap = new rMap<rString, ConfigEntry*>;
#endif
			ce->Type = Type::Map;
			ce->Value.Map = newMap;
			toAddTo->emplace( val, ce );
			toAddTo = ce->Value.Map;
		}
		else
		{
			if( mapIt->second->Type == Type::Map )
			{
				toAddTo = mapIt->second->Value.Map;
			}
			else
			{
				Logger::Log( "Entry already exists for " + currentScopePath + " and it is not a scope", "Config", LogSeverity::ERROR_MSG );
				return nullptr;
			}
		}
	}
	return toAddTo;
}

rString Config::GetString( const rString& key, const rString& defaultValue, const rString& comment )
{
	const ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type == Type::String )
		{
			return rString( ce->Value.StringVal );
		}
		else
		{
			Logger::Log( "Value with key: " + key + " is not a string, returning default value: " + defaultValue, "Config", LogSeverity::WARNING_MSG );
			return defaultValue;
		}
	}
	else
	{
		Logger::Log( "Failed to find config entry with key: " + key + ", will attempt to create one with default value: " + defaultValue,
					 "Config", LogSeverity::DEBUG_MSG );

		ConfigEntry* cfgEntry = pNew( ConfigEntry );
		cfgEntry->Type = Type::String;
		cfgEntry->Value.StringVal = pNewArray( char, defaultValue.size( ) + 1 );
		memcpy( cfgEntry->Value.StringVal, defaultValue.data( ), defaultValue.size( ) * sizeof( char ) );
		// Add null terminator
		cfgEntry->Value.StringVal[defaultValue.size( )] = '\0';
		cfgEntry->Comment = comment;

		if( !PutEntry( key, cfgEntry ) )
		{
			Logger::Log( "Failed to add key: " + key + "with value: " + defaultValue + ", returning default value: " + defaultValue, "Config", LogSeverity::WARNING_MSG );
			pDelete( cfgEntry );
		}
		else
			m_Dirty = true;
		return defaultValue;
	}
}

double Config::GetDouble( const rString& key, double defaultValue, const rString& comment )
{
	const ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type == Type::Double )
		{
			return ce->Value.DoubleVal;
		}
		else
		{
			Logger::Log( "Value with key: " + key + " is not a double or float, returning default value: " + rToString( defaultValue ),
						 "Config", LogSeverity::ERROR_MSG );
			return defaultValue;
		}
	}
	else
	{
		Logger::Log( "Failed to find config entry with key: " + key + ", will attempt to create one with default value: " + rToString( defaultValue ),
					 "Config", LogSeverity::DEBUG_MSG );

		ConfigEntry* cfgEntry = pNew( ConfigEntry );
		cfgEntry->Type = Type::Double;
		cfgEntry->Value.DoubleVal = defaultValue;
		cfgEntry->Comment = comment;

		if( !PutEntry( key, cfgEntry ) )
		{
			Logger::Log( "Failed to add key: " + key + "with value: " + rToString(defaultValue) + ", returning default value: " + rToString(defaultValue), "Config", LogSeverity::WARNING_MSG );
			pDelete( cfgEntry );
		}
		else
			m_Dirty = true;
		return defaultValue;
	}
}

float Config::GetFloat( const rString& key, float defaultValue, const rString& comment )
{
	return static_cast<float>( GetDouble( key, defaultValue, comment ) );
}

int Config::GetInt( const rString& key, int defaultValue, const rString& comment )
{
	const ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type == Type::Int )
		{
			return ce->Value.IntVal;
		}
		else
		{
			Logger::Log( "Value with key: " + key + " is not an integer, returning default value: " + rToString( defaultValue ), "Config",
						 LogSeverity::ERROR_MSG );
			return defaultValue;
		}
	}
	else
	{
		Logger::Log( "Failed to find config entry with key: " + key + ", will attempt to create one with default value: " + rToString( defaultValue ),
					 "Config", LogSeverity::DEBUG_MSG );

		ConfigEntry* cfgEntry = pNew( ConfigEntry );
		cfgEntry->Type = Type::Int;
		cfgEntry->Value.IntVal = defaultValue;
		cfgEntry->Comment = comment;

		if( !PutEntry( key, cfgEntry ) )
		{
			Logger::Log( "Failed to add key: " + key + "with value: " + rToString( defaultValue ) + ", returning default value: " + rToString( defaultValue ), "Config", LogSeverity::WARNING_MSG );
			pDelete( cfgEntry );
		}
		else
			m_Dirty = true;
		return defaultValue;
	}
}

bool Config::GetBool( const rString& key, bool defaultValue, const rString& comment )
{
	const ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type == Type::Bool )
		{
			return ce->Value.BoolVal;
		}
		else
		{
			Logger::Log( "Value with key: " + key + " is not a bool, returning default value: " + rToString( defaultValue ), "Config",
						 LogSeverity::ERROR_MSG );
			return defaultValue;
		}
	}
	else
	{
		Logger::Log( "Failed to find config entry with key: " + key + ", will attempt to create one with default value: " + rToString( defaultValue ),
					 "Config", LogSeverity::DEBUG_MSG );

		ConfigEntry* cfgEntry = pNew( ConfigEntry );
		cfgEntry->Type = Type::Bool;
		cfgEntry->Value.BoolVal = defaultValue;
		cfgEntry->Comment = comment;

		if( !PutEntry( key, cfgEntry ) )
		{
			Logger::Log( "Failed to add key: " + key + "with value: " + rToString( defaultValue ) + ", returning default value: " + rToString( defaultValue ), "Config", LogSeverity::WARNING_MSG );
			pDelete( cfgEntry );
		}
		else
			m_Dirty = true;
		return defaultValue;
	}
}

rMap<rString, Config::ConfigEntry*>* Config::GetScopeMap( const rString& scopes ) const
{
	rString val;
	rIStringStream iss( scopes );
	getline( iss, val, '.' );
	rMap<rString, ConfigEntry*>::const_iterator cit = m_configs.find( val );
	if( cit == m_configs.end( ) )
	{
		Logger::Log( "Failed to find scope with key: " + val, "Config", LogSeverity::ERROR_MSG );
		return nullptr;
	}
	if( cit->second->Type != Type::Map )
	{
		Logger::Log( "Key: " + val + " is not a scope", "Config", LogSeverity::ERROR_MSG );
		return nullptr;
	}
	rMap<rString, ConfigEntry*>* mp = cit->second->Value.Map;
	while( getline( iss, val, '.' ) )
	{
		cit = mp->find( val );
		if( cit == m_configs.end( ) )
		{
			Logger::Log( "Failed to find scope with key: " + val, "Config", LogSeverity::ERROR_MSG );
			return nullptr;
		}
		if( cit->second->Type != Type::Map )
		{
			Logger::Log( "Key: " + val + " is not a scope", "Config", LogSeverity::ERROR_MSG );
			return nullptr;
		}
		mp = cit->second->Value.Map;
	}
	return mp;
}

rVector<Config::ConfigEntry*>* Config::GetArray( const rString& key )
{
	const ConfigEntry* ce = AssertFind( key );
	if( ce == nullptr )
	{
		Logger::Log( "Failed to get array with key: " + key + ", attempting to return empty array", "Config", LogSeverity::DEBUG_MSG );
		ConfigEntry* arrayEntry = pNew( ConfigEntry );
		arrayEntry->Type = Type::Array;
		arrayEntry->Value.Array = pNew(rVector<ConfigEntry*>);
		if ( !PutEntry( key, arrayEntry ) )
		{
			pDelete( arrayEntry->Value.Array );
			pDelete( arrayEntry );
			Logger::Log( "Failed to create array entry with key: " + key, "Config", LogSeverity::ERROR_MSG );
			return nullptr;
		}
		else
		{
			return arrayEntry->Value.Array;
		}
	}
	else if( ce->Type != Type::Array )
	{
		Logger::Log( "Value with key: " + key + " is not an array", "Config", LogSeverity::ERROR_MSG );
		return nullptr;
	}
	return ce->Value.Array;
}

void Config::SetString( const rString& key, const rString& value )
{
	ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type != Config::Type::String )
			Logger::Log( "Config entry with key: " + key + " is not of type string, value: " + value + "  will not be set", "Config",
						 LogSeverity::WARNING_MSG );
		else
		{
			pDelete( ce->Value.StringVal );
			ce->Value.StringVal = pNewArray( char, value.size( ) + 1 );
			memcpy( ce->Value.StringVal, &value[0], value.size( ) );
			// Add null terminator
			ce->Value.StringVal[value.size( )] = '\0';
			m_Dirty = true;
		}
	}
	else
		Logger::Log( "Failed to set string with key: " + key + " and value: " + value, "Config", LogSeverity::WARNING_MSG );
}

void Config::SetDouble( const rString& key, double value )
{
	ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type != Config::Type::Double )
		{
			Logger::Log( "Config entry with key: " + key + " is not of type double, value: " + rToString( value ) + "  will not be set",
						 "Config", LogSeverity::WARNING_MSG );
		}
		else
		{
			ce->Value.DoubleVal = value;
			m_Dirty = true;
		}
	}
	else
		Logger::Log( "Failed to set double with key: " + key + " and value: " + rToString( value ), "Config", LogSeverity::WARNING_MSG );
}

void Config::SetFloat( const rString& key, float value )
{
	ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type != Config::Type::Double )
		{
			Logger::Log( "Config entry with key: " + key + " is not of type float, value: " + rToString( value ) + "  will not be set",
						 "Config", LogSeverity::WARNING_MSG );
		}
		else
		{
			ce->Value.DoubleVal = static_cast<double>( value );
			m_Dirty = true;
		}
	}
	else
		Logger::Log( "Failed to set float with key: " + key + " and value: " + rToString( value ), "Config", LogSeverity::WARNING_MSG );
}

void Config::SetInt( const rString& key, int value )
{
	ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type != Config::Type::Int )
		{
			Logger::Log( "Config entry with key: " + key + " is not of type int, value: " + rToString( value ) + " will not be set",
						 "Config", LogSeverity::WARNING_MSG );
		}
		else
		{
			ce->Value.IntVal = value;
			m_Dirty = true;
		}
	}
	else
		Logger::Log( "Failed to set int with key: " + key + " and value: " + rToString( value ), "Config", LogSeverity::WARNING_MSG );
}

void Config::SetBool( const rString& key, bool value )
{
	ConfigEntry* ce = AssertFind( key );
	if( ce != nullptr )
	{
		if( ce->Type != Config::Type::Bool )
		{
			Logger::Log( "Config entry with key: " + key + " is not of type bool, value: " + rToString( value ) + "  will not be set",
						 "Config", LogSeverity::WARNING_MSG );
		}
		else
		{
			ce->Value.BoolVal = value;
			m_Dirty = true;
		}
	}
	else
		Logger::Log( "Failed to set bool with key: " + key + " and value: " + rToString( value ), "Config", LogSeverity::WARNING_MSG );
}

bool Config::Parse( const rString& strToParse, rMap<rString, ConfigEntry*>* mapToAddTo )
{
	rString key = "";
	rString strValue = "";

	Stage stage = Stage::Key;
	Type type = Type::Int;
	char c;
	rIStringStream iss( strToParse );
	bool failed = false;
	rVector<ConfigEntry*>* vec = nullptr;
	rMap<rString, ConfigEntry*>* mp = nullptr;
	rString comment = "";
	while( !iss.eof( ) )
	{
		if( stage == Stage::Key )
		{
			comment = "";
			getline( iss, key, ':' );
			size_t firstNonWhitespace = key.find_first_not_of( " \t\n" );
			if ( firstNonWhitespace == std::string::npos )
				break;
			if ( key.at( firstNonWhitespace ) == ';' )
			{
				size_t newLine = key.find_first_of( "\n", firstNonWhitespace );
				size_t currentPos = firstNonWhitespace;
				while ( newLine != std::string::npos )
				{
					size_t startOfComment = key.find_first_not_of(" \t\n", currentPos) + 1;
					comment += key.substr( startOfComment, newLine - startOfComment );
					currentPos = newLine;
					newLine = key.find_first_of( "\n", newLine + 1 );
					if ( newLine != std::string::npos )
						comment += "\n";
				}
				key = key.substr(currentPos + 1, key.size() - currentPos);
			}
			// Strip whitespaces
			key.erase( remove_if( key.begin( ), key.end( ), isspace ), key.end( ) );
			// key = regex_replace(key, regex("\\s+"), "");
			stage = Stage::Value;
			auto it = mapToAddTo->find( key );
			if( it != mapToAddTo->end( ) )
			{
				pDelete( it->second );
				mapToAddTo->erase( it );
			}
			continue;
		}

		c = iss.peek( );

		// Ignore whitespaces
		if( c == ' ' || c == '\n' || c == '\r' || c == '\t' )
		{
			iss.get( );
			continue;
		}
		// Scope/Map
		else if( c == '{' )
		{
			iss.get( );
			rString sst;
			int scount = 1;
			char sc = ' ';
			while( scount > 0 && !iss.eof( ) )
			{
				sc = iss.get( );
				if( sc == '{' )
				{
					scount++;
				}
				else if( sc == '}' )
				{
					scount--;
				}
				sst += sc;
			}
			sst.pop_back( );
// TODODP: h4x!!! Make prettier with a proper new method or macro!!
#if !defined( DISABLE_ALLOCATOR )
#ifdef DEBUG_MEMORY
			mp = MemoryAllocator::Construct<rMap<rString, ConfigEntry*>>( 1ULL, true, __FILE__, __LINE__ );
#else
			mp = MemoryAllocator::Construct<rMap<rString, ConfigEntry*>>( 1ULL, true );
#endif
#else
			mp = new rMap<rString, ConfigEntry*>;
#endif
			stage = Stage::Key;

			auto it = mapToAddTo->find( key );
			if( it != mapToAddTo->end( ) )
			{
				pDelete( it->second );
				mapToAddTo->erase( it );
			}

			if( Parse( sst, mp ) )
			{
				type = Type::Map;
				stage = Stage::Value;
			}
			else
			{
				pDelete( mp );
				Logger::Log( "Failed to interpret scope: " + sst + " with key: " + key, "Config", LogSeverity::ERROR_MSG );
				return false;
			}
			continue;
		}
		else if( c == '[' )
		{
			iss.get( );
			rString array;
			getline( iss, array, ']' );
			vec = pNew( rVector<ConfigEntry*> );

			auto it = mapToAddTo->find( key );
			if( it != mapToAddTo->end( ) )
			{
				pDelete( it->second );
				mapToAddTo->erase( it );
			}

			if( ParseArray( array, vec ) )
			{
				type = Type::Array;
			}
			else
			{
				pDelete( vec );
				Logger::Log( "Failed to interpret array: " + array + " with key:" + key, "Config", LogSeverity::ERROR_MSG );
				return false;
			}
			continue;
		}
		// String
		else if( c == '"' )
		{
			iss.get( );
			getline( iss, strValue, '"' );
			type = Type::String;
			continue;
		}
		// Bool or numerical value
		else if( c != ',' )
		{
			getline( iss, strValue, ',' );
			strValue.erase( remove_if( strValue.begin( ), strValue.end( ), isspace ), strValue.end( ) );
			if( strValue == "true" || strValue == "false" )
			{
				type = Type::Bool;
			}
			else
			{
				type = Type::Int;
			}
			c = ',';
		}
		// , marks end of value
		if( c == ',' )
		{
			iss.get( );
			ConfigEntry* ce = pNew( ConfigEntry );

			// Default to int when not string
			if( type == Type::Int )
			{
				// Try converting to int or double
				try
				{
					// Check for "."
					if( strValue.find( '.' ) == rString::npos )
					{
						ce->Value.IntVal = std::stoi( strValue.c_str( ) );
					}
					// Contained a "."; hopefully a double.
					else
					{
						double value = std::stod( strValue.c_str( ) );
						ce->Value.DoubleVal = value;
						type = Type::Double;
					}
				}
				// Not a valid value :(
				catch( ... )
				{
					Logger::Log( "Failed to parse value: " + strValue, "Config", LogSeverity::ERROR_MSG );
					failed = true;
				}
			}
			// It is a bool
			else if( type == Type::Bool )
			{
				ce->Value.BoolVal = strValue == "true" ? true : false;
			}
			else if( type == Type::String )
			{
				// Strip the " in the front and back, at the same time make new copy
				// so that it doesn't get destructed.
				ce->Value.StringVal = pNewArray( char, strValue.size( ) + 1 );
				memcpy( ce->Value.StringVal, &strValue[0], strValue.size( ) );
				// Add null terminator
				ce->Value.StringVal[strValue.size( )] = '\0';
			}
			else if( type == Type::Array )
			{
				ce->Value.Array = vec;
			}
			else if( type == Type::Map )
			{
				ce->Value.Map = mp;
			}

			// Don't add invalid entry
			if( !failed )
			{
				ce->Type = type;
				auto it = mapToAddTo->find( key );
				if( it != mapToAddTo->end( ) )
				{
					pDelete( it->second );
					mapToAddTo->erase( it );
				}
				ce->Comment = comment;
				mapToAddTo->emplace( key, ce );
			}
			else
			{
				pDelete( ce );
			}

			// Reset stuff
			key = "";
			strValue = "";
			stage = Stage::Key;
			type = Type::Int;
			ce = nullptr;
		}
	}
	// TODOJM: Make sure to tell people that they might have a missing ,
	return true;
}

bool Config::ParseArray( const rString& arrayStr, rVector<ConfigEntry*>* vec )
{
	rIStringStream issa( arrayStr );
	rString val;
	while( getline( issa, val, ',' ) )
	{
		// Trim whitespaces
		val.erase( 0, val.find_first_not_of( " \t\r\n" ) );
		val.erase( val.find_last_not_of( " \t\r\n" ) + 1 );
		ConfigEntry* ce = pNew( ConfigEntry );

		try
		{
			// String?
			if( val.find_first_of( '"' ) != rString::npos )
			{
				if( val.find_last_of( '"' ) != rString::npos )
				{
					// Create char pointer
					ce->Value.StringVal = pNewArray( char, val.size( ) - 1 );
					memcpy( ce->Value.StringVal, &val[1], val.size( ) - 2 );
					// Add null terminator
					ce->Value.StringVal[val.size( ) - 2] = '\0';
					ce->Type = Type::String;
				}
				else
				{
					throw; // I don't want this phony string, throw it away
				}
			}
			// Possible double?
			else if( val.find_first_of( "." ) != rString::npos )
			{
				ce->Value.DoubleVal = std::stod( val.c_str( ) );
				ce->Type = Type::Double;
			}
			// Bool?
			else if( val == "true" || val == "false" )
			{
				ce->Value.BoolVal = val == "true" ? true : false;
				ce->Type = Type::Bool;
			}
			// Hopefully an int
			else
			{
				ce->Value.IntVal = std::stoi( val.c_str( ) );
				ce->Type = Type::Int;
			}

			vec->push_back( ce );
		}
		catch( ... )
		{
			pDelete( ce );
			return false;
		}
	}
	return true;
}
Config::ConfigEntry* Config::AssertFind( const rString& key ) const
{
	size_t p = key.find_first_of( '.' );

	rMap<rString, ConfigEntry*>::const_iterator it;

	// Not in scope
	if( p == rString::npos )
	{
		size_t startSquareBracket = key.find_first_of( '[' );
		size_t endSquareBracket = key.find_first_of( ']' );

		// Check if trying to access array
		if( startSquareBracket != rString::npos && endSquareBracket != rString::npos )
		{
			rString newKey = key.substr( 0, startSquareBracket );
			rString strIndex = key.substr( startSquareBracket + 1, 1 );
			int index = std::stoi( strIndex.c_str( ) );

			it = m_configs.find( newKey );
			if( it == m_configs.end( ) )
				return nullptr;
			return it->second->Value.Array->at( index );
		}
		// Was not trying to access array
		else
		{
			it = m_configs.find( key );
			if( it == m_configs.end( ) )
				return nullptr;
			return it->second;
		}
	}
	// In scope
	else
	{
		rString scope = key.substr( 0, p );
		rString skey = key.substr( p + 1, p - key.size( ) );
		rMap<rString, ConfigEntry*>* mp = GetScopeMap( scope );
		if( mp == nullptr )
			return nullptr;

		size_t startSquareBracket = skey.find_first_of( '[' );
		size_t endSquareBracket = skey.find_first_of( ']' );

		// Check if trying to access array
		if( startSquareBracket != rString::npos && endSquareBracket != rString::npos )
		{
			rString newKey = skey.substr( 0, startSquareBracket );
			rString strIndex = skey.substr( startSquareBracket + 1, 1 );
			int index = std::stoi( strIndex.c_str( ) );

			it = mp->find( newKey );
			return it->second->Value.Array->at( index );
		}
		// Was not trying to access array
		else
		{
			it = mp->find( skey );
			if( it == mp->end( ) )
				return nullptr;
			return it->second;
		}
	}
}

bool Config::IsDirty( ) const
{
	return m_Dirty;
}
