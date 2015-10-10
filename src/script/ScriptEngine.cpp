/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#include "ScriptEngine.h"
#include "LuaAllocator.h"
#include <utility/Logger.h>

int LuaLogger( IScriptEngine* scriptEngine )
{
	Logger::GetStream( ) << "[C=PURPLE]$$$ [C=WHITE]";
	ScriptArg val = scriptEngine->Pop( );
	switch ( val.Type )
	{
		case ScriptPrimitive::Int:
			Logger::GetStream( ) << val.asInt;
			break;

		case ScriptPrimitive::Uint:
			Logger::GetStream( ) << val.asUint;
			break;

		case ScriptPrimitive::Bool:
			Logger::GetStream( ) << val.asBool;
			break;

		case ScriptPrimitive::Float:
			Logger::GetStream( ) << val.asFloat;
			break;

		case ScriptPrimitive::Double:
			Logger::GetStream( ) << val.asDouble;
			break;

		case ScriptPrimitive::String:
			Logger::GetStream( ) << val.asString;
			break;

		case ScriptPrimitive::Undefined:
		default:
			break;
	}
	return 0;
}

ScriptEngine& ScriptEngine::GetInstance( ScriptInstance instance )
{
	static ScriptEngine instances[ (size_t) ScriptInstance::Count ];
	return instances[ (size_t) instance];
}

ScriptEngine::ScriptEngine( )
{
}

ScriptEngine::~ScriptEngine( )
{
	if ( m_Lua )
		lua_close( m_Lua );
}

void ScriptEngine::Error( const char* error, const char* extra )
{
	Logger::Log( rString( error ) + rString( extra ), "ScriptEngine", LogSeverity::ERROR_MSG );
}

void ScriptEngine::Initialize( const char* initScript )
{
#if !defined(DISABLE_ALLOCATOR)
	m_Lua = lua_newstate( MemoryAllocator::LuaAllocator, nullptr );
#else
	m_Lua = luaL_newstate( );
#endif
	luaL_openlibs( m_Lua );

	Register( "Log", &LuaLogger );

	if ( initScript )
		Run( initScript );
}

void ScriptEngine::Reload( const char* initScript )
{
	if ( m_Lua )
		lua_close( m_Lua );
	Initialize( initScript );
	
	for ( auto& func : m_Functions )
	{
		lua_pushstring( m_Lua, func.first.c_str( ) );
		lua_pushcclosure( m_Lua, _ScriptCallback, 1 );
		lua_setglobal( m_Lua, func.first.c_str( ) );
	}
}

void ScriptEngine::Run( const char* scriptFile )
{
	if ( luaL_dofile( m_Lua, scriptFile ) != LUA_OK )
		Error( "Failed to run script file. Path: ", scriptFile );
}

void ScriptEngine::Perform( const char* scriptLine )
{
	if ( luaL_dostring( m_Lua, scriptLine ) != LUA_OK )
		Error( "Failed to perform script line: ", scriptLine );
}

void ScriptEngine::Call( const char* functionName, std::initializer_list<ScriptArg> arguments, unsigned int fromStack )
{
	lua_getglobal( m_Lua, functionName );
	if ( !lua_isfunction( m_Lua, STACK_TOP ) )
	{
		Error( "Not a function: ", functionName );
		lua_pop( m_Lua, 1 );
		return;
	}
	
	if ( fromStack )
		lua_insert( m_Lua, -(int)fromStack - 1 );

	for ( auto& arg : arguments )
	{
		switch ( arg.Type )
		{
			case ScriptPrimitive::Int:
				lua_pushinteger( m_Lua, arg.asInt );
				break;

			case ScriptPrimitive::Uint:
				lua_pushunsigned( m_Lua, arg.asUint );
				break;

			case ScriptPrimitive::Bool:
				lua_pushboolean( m_Lua, (int) arg.asBool );
				break;

			case ScriptPrimitive::Float:
				lua_pushnumber( m_Lua, (double) arg.asFloat );
				break;

			case ScriptPrimitive::Double:
				lua_pushnumber( m_Lua, arg.asDouble );
				break;

			case ScriptPrimitive::String:
				lua_pushstring( m_Lua, arg.asString.c_str( ) );
				break;

			case ScriptPrimitive::Undefined:
			default:
				Error( "Type undefined in script argument primitive." );
				break;
		}
	}

	if ( lua_pcall( m_Lua, (int) arguments.size( ) + fromStack, LUA_MULTRET, 0 ) != LUA_OK )
		Error( "Lua function call failed. Function: ", functionName );
}

int ScriptEngine::StackSize( )
{
	return lua_gettop( m_Lua );
}

void ScriptEngine::Register( const char* functionName, ScriptFunction function )
{
	lua_pushstring( m_Lua, functionName );
	lua_pushlightuserdata( m_Lua, this );
	lua_pushcclosure( m_Lua, _ScriptCallback, 2 );
	lua_setglobal( m_Lua, functionName );
	m_Functions[functionName] = function;
}

int ScriptEngine::_ScriptCallback( lua_State* lua )
{
	const char*		name	= lua_tostring( lua, lua_upvalueindex( 1 ) );
	ScriptEngine*	engine	= (ScriptEngine*) lua_topointer( lua, lua_upvalueindex( 2 ) );
	auto func = engine->m_Functions.find( name );
	if ( func != engine->m_Functions.end( ) )
		return func->second( engine );
	return 0;
}

#pragma region PUSH, POP, SET, GET
#pragma region SCRIPTARG
void ScriptEngine::Push( const ScriptArg& value )
{
	switch ( value.Type )
	{
		case ScriptPrimitive::Int:
			PushInt( value.asInt );
			break;

		case ScriptPrimitive::Uint:
			PushUint( value.asUint );
			break;

		case ScriptPrimitive::Bool:
			PushBool( value.asBool );
			break;

		case ScriptPrimitive::Float:
			PushFloat( value.asFloat );
			break;

		case ScriptPrimitive::Double:
			PushDouble( value.asDouble );
			break;

		case ScriptPrimitive::String:
			PushString( value.asString );
			break;

		case ScriptPrimitive::Undefined:
		default:
			Error( "Push argument type unsupported." );
			break;
	}
}

ScriptArg ScriptEngine::Pop( )
{
	int type = lua_type( m_Lua, STACK_TOP );
	switch ( type )
	{
		case LUA_TNUMBER:
			return ScriptArg( PopDouble( ) );

		case LUA_TBOOLEAN:
			return ScriptArg( PopBool( ) );

		case LUA_TSTRING:
			return ScriptArg( PopString( ) );

		case LUA_TNIL:
			PopNull( );
			return ScriptArg( );

		case LUA_TTABLE:
		case LUA_TTHREAD:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
		default:
			PopNull( );
			Error( "Unsupported type to pop. Popped anyway..." );
			return ScriptArg( );
	}
}

void ScriptEngine::Set( const char* name, const ScriptArg& value )
{
	Push( value );
	lua_setglobal( m_Lua, name );
}

ScriptArg ScriptEngine::Get( const char* name )
{
	lua_getglobal( m_Lua, name );
	return Pop( );
}
#pragma endregion

#pragma region ARRAY
void ScriptEngine::PushArray( ScriptArg* value, int count )
{
	lua_createtable( m_Lua, count, 0 );
	for ( int i = 0; i < count; ++i )
	{
		PushInt( i + 1 );
		Push( value[i] );
		lua_settable( m_Lua, -3 );
	}
}

int ScriptEngine::PopArray( ScriptArg* value, int count )
{
	int i = 0;
	int initialSize = StackSize( );

	lua_pushnil( m_Lua );
	while ( i < count && lua_next( m_Lua, -2 ) )
			value[i++] = Pop( );

	lua_pop( m_Lua, StackSize( ) - initialSize + 1 );
	return i;
}

void ScriptEngine::SetArray( const char* name, ScriptArg* value, int count )
{
	PushArray( value, count );
	lua_setglobal( m_Lua, name );
}

int ScriptEngine::GetArray( const char* name, ScriptArg* value, int count )
{
	lua_getglobal( m_Lua, name );
	return PopArray( value, count );
}
#pragma endregion

#pragma region INT
void ScriptEngine::PushInt( int value )
{
	lua_pushinteger( m_Lua, value );
}

int ScriptEngine::PopInt( )
{
	int isInt = 0;
	int i = (int) lua_tointegerx( m_Lua, STACK_TOP, &isInt );
	if ( !isInt )
		Error( "Popping non-integer value in PopInt( )." );
	lua_pop( m_Lua, 1 );
	return i;
}

void ScriptEngine::SetInt( const char* name, int value )
{
	PushInt( value );
	lua_setglobal( m_Lua, name );
}

int ScriptEngine::GetInt( const char* name )
{
	lua_getglobal( m_Lua, name );
	return PopInt( );
}
#pragma endregion

#pragma region UINT
void ScriptEngine::PushUint( unsigned int value )
{
	lua_pushunsigned( m_Lua, value );
}

unsigned int ScriptEngine::PopUint( )
{
	// TODODP: Check if integer instead of number ?
	int isNum = lua_isnumber( m_Lua, STACK_TOP );
	unsigned int u = (unsigned int) lua_tounsigned( m_Lua, STACK_TOP );
	if ( !isNum )
		Error( "Popping non-number value in PopUint( )." );
	lua_pop( m_Lua, 1 );
	return u;
}

void ScriptEngine::SetUint( const char* name, unsigned int value )
{
	PushUint( value );
	lua_setglobal( m_Lua, name );
}

unsigned int ScriptEngine::GetUint( const char* name )
{
	lua_getglobal( m_Lua, name );
	return PopUint( );
}
#pragma endregion

#pragma region BOOL
void ScriptEngine::PushBool( bool value )
{
	lua_pushboolean( m_Lua, (int) value );
}

bool ScriptEngine::PopBool( )
{
	int isBool = lua_isboolean( m_Lua, STACK_TOP );
	bool b = lua_toboolean( m_Lua, STACK_TOP ) != 0;
	if ( !isBool )
		Error( "Popping non-boolean value in PopBool( )." );
	lua_pop( m_Lua, 1 );
	return b;
}

void ScriptEngine::SetBool( const char* name, bool value )
{
	PushBool( value );
	lua_setglobal( m_Lua, name );
}

bool ScriptEngine::GetBool( const char* name )
{
	lua_getglobal( m_Lua, name );
	return PopBool( );
}
#pragma endregion

#pragma region FLOAT
void ScriptEngine::PushFloat( float value )
{
	lua_pushnumber( m_Lua, (double) value );
}

float ScriptEngine::PopFloat( )
{
	int isNum = 0;
	float f = (float) lua_tonumberx( m_Lua, STACK_TOP, &isNum );
	if ( !isNum )
		Error( "Popping non-number value in PopFloat( )." );
	lua_pop( m_Lua, 1 );
	return f;
}

void ScriptEngine::SetFloat( const char* name, float value )
{
	PushFloat( value );
	lua_setglobal( m_Lua, name );
}

float ScriptEngine::GetFloat( const char* name )
{
	lua_getglobal( m_Lua, name );
	return PopFloat( );
}
#pragma endregion

#pragma region DOUBLE
void ScriptEngine::PushDouble( double value )
{
	lua_pushnumber( m_Lua, (double) value );
}

double ScriptEngine::PopDouble( )
{
	int isNum = 0;
	double d = lua_tonumberx( m_Lua, STACK_TOP, &isNum );
	if ( !isNum )
		Error( "Popping non-number value in PopDouble( )." );
	lua_pop( m_Lua, 1 );
	return d;
}

void ScriptEngine::SetDouble( const char* name, double value )
{
	PushDouble( value );
	lua_setglobal( m_Lua, name );
}

double ScriptEngine::GetDouble( const char* name )
{
	lua_getglobal( m_Lua, name );
	return PopDouble( );
}
#pragma endregion

#pragma region STRING
void ScriptEngine::PushString( const rString& value )
{
	lua_pushstring( m_Lua, value.c_str( ) );
}

rString ScriptEngine::PopString( )
{
	rString s;
	if ( lua_isstring( m_Lua, STACK_TOP ) )
		s = lua_tostring( m_Lua, STACK_TOP );
	else
		Error( "Popping non-string value in PopString( )." );
	lua_pop( m_Lua, 1 );
	return s;
}

void ScriptEngine::SetString( const char* name, const rString& value )
{
	PushString( value );
	lua_setglobal( m_Lua, name );
}

rString ScriptEngine::GetString( const char* name )
{
	lua_getglobal( m_Lua, name );
	return PopString( );
}
#pragma endregion

#pragma region NULL
void ScriptEngine::PushNull( )
{
	lua_pushnil( m_Lua );
}

void ScriptEngine::PopNull( )
{
	lua_pop( m_Lua, 1 );
}

void ScriptEngine::SetNull( const char* name )
{
	PushNull( );
	lua_setglobal( m_Lua, name );
}

void ScriptEngine::GetNull( const char* name )
{
	Error( "Is User A Goat? ", "GetNull is not supported." );
}
#pragma endregion
#pragma endregion