/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include "IScriptEngine.h"

#include "lua.hpp"

#define SCRIPT_MAIN "../../../script/Main.lua"
#define STACK_TOP		-1

#define g_Script ScriptEngine::GetInstance( )

enum class ScriptInstance : size_t
{
	Default,
	Audio,
	Count
};

class ScriptEngine : public IScriptEngine
{
public:
	~ScriptEngine( );

	// Get the ScriptEngine instance.
	SCRIPT_API static ScriptEngine& GetInstance( ScriptInstance instance = ScriptInstance::Default );

	// DO NOT USE. Lua callback method.
	SCRIPT_API static int			_ScriptCallback( lua_State* lua );

	// Initialize ScriptEninge and run specified script. nullptr = no script.
	SCRIPT_API void Initialize	( const char* initScript = SCRIPT_MAIN );

	// Reinitialize the ScriptEngine and run specified script. nullptr = no script.
	SCRIPT_API void Reload		( const char* initScript = SCRIPT_MAIN );

	// Run script from file.
	void Run		( const char* scriptFile )																					override;

	// Run script from string.
	void Perform	( const char* scriptLine )																					override;

	// Register a C(++) function and name it for internal script access.
	void Register	( const char* functionName, ScriptFunction function )														override;

	// Call script function with (optional) arguments in initializer list. Optionally specify number of arguments already available from the stack.
	void Call		( const char* functionName, std::initializer_list<ScriptArg> arguments = { }, unsigned int fromStack = 0U )	override;

	// Get the number of elements on the script stack.
	int  StackSize	( )																											override;

	// Push

	void			PushInt		( int value )					override;
	void			PushBool	( bool value )					override;
	void			PushString	( const rString& value )		override;
	void			PushFloat	( float value )					override;
	void			PushDouble	( double value )				override;
	void			PushUint	( unsigned int value )			override;
	void			PushNull	( )								override;

	void			Push		( const ScriptArg& value )		override;
	void			PushArray	( ScriptArg* value, int count )	override;

	// Pop

	int				PopInt		( )								override;
	bool			PopBool		( )								override;
	rString			PopString	( )								override;
	float			PopFloat	( )								override;
	double			PopDouble	( )								override;
	unsigned int	PopUint		( )								override;
	void			PopNull		( )								override;

	ScriptArg		Pop			(  )							override;
	int				PopArray	( ScriptArg* value, int count )	override;

	// Set

	void			SetInt		( const char* name, int	value )						override;
	void			SetBool		( const char* name, bool value )					override;
	void			SetString	( const char* name, const rString& value )			override;
	void			SetFloat	( const char* name, float value )					override;
	void			SetDouble	( const char* name, double value )					override;
	void			SetUint		( const char* name, unsigned int value )			override;
	void			SetNull		( const char* name )								override;

	void			Set			( const char* name, const ScriptArg& value )		override;
	void			SetArray	( const char* name, ScriptArg* value, int count )	override;

	// Get

	int				GetInt		( const char* name )								override;
	bool			GetBool		( const char* name )								override;
	rString			GetString	( const char* name )								override;
	float			GetFloat	( const char* name )								override;
	double			GetDouble	( const char* name )								override;
	unsigned int	GetUint		( const char* name )								override;
	void			GetNull		( const char* name )								override;

	ScriptArg		Get			( const char* name )								override;
	int				GetArray	( const char* name, ScriptArg* value, int count )	override;

private:
	/* no type */	ScriptEngine	( );
	/* no type */	ScriptEngine	( const ScriptEngine& );
	ScriptEngine&	operator=		( const ScriptEngine& );

	void Error( const char* error, const char* extra = "" );

	lua_State*								m_Lua = nullptr;
	rMap< rString, ScriptFunction >			m_Functions;
};
