/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#pragma once
#include <initializer_list>
#include <functional>
#include <memory/Alloc.h>

#ifdef _WIN32
	#ifdef SCRIPT_DLL_EXPORT // This is the define you defines in the CMakeLists.txt file
		#define SCRIPT_API __declspec(dllexport) // Exports symbols
	#else
		#define SCRIPT_API __declspec(dllimport) // Imports symbols
	#endif
#else
	#define SCRIPT_API // Unix is not annoying :D
#endif

enum class ScriptPrimitive
{
	Undefined,

	Int,
	Uint,
	Bool,
	Float,
	Double,
	String,
};


struct ScriptArg
{
	ScriptPrimitive Type;
	rString			asString;
	union
	{
		union
		{
			int				asInt;
			unsigned int	asUint;
			bool			asBool;
			float			asFloat;
			double			asDouble;
		};
	};

	// Default constructor
	ScriptArg( )						{ Type = ScriptPrimitive::Undefined; }

	// Constructors
	ScriptArg( int i )					{ this->asInt		= i;	Type = ScriptPrimitive::Int;	}
	ScriptArg( bool b )					{ this->asBool		= b;	Type = ScriptPrimitive::Bool;	}
	ScriptArg( char* s )				{ this->asString	= s;	Type = ScriptPrimitive::String; }
	ScriptArg( const rString& s )		{ this->asString	= s;	Type = ScriptPrimitive::String; }
	ScriptArg( float f )				{ this->asFloat		= f;	Type = ScriptPrimitive::Float;	}
	ScriptArg( double d )				{ this->asDouble	= d;	Type = ScriptPrimitive::Double;	}
	ScriptArg( unsigned int u )			{ this->asUint		= u;	Type = ScriptPrimitive::Uint;	}

	// Assignment constructors
	void operator=( int i )					{ this->asInt		= i;	Type = ScriptPrimitive::Int; }
	void operator=( bool b )				{ this->asBool		= b;	Type = ScriptPrimitive::Bool; }
	void operator=( char* s )				{ this->asString	= s;	Type = ScriptPrimitive::String; }
	void operator=(const rString& s)		{ this->asString	= s;	Type = ScriptPrimitive::String; }
	void operator=( float f )				{ this->asFloat		= f;	Type = ScriptPrimitive::Float; }
	void operator=( double d )				{ this->asDouble	= d;	Type = ScriptPrimitive::Double; }
	void operator=( unsigned int u )		{ this->asUint		= u;	Type = ScriptPrimitive::Uint; }
};


typedef std::function<int( class IScriptEngine* )> ScriptFunction;


class SCRIPT_API IScriptEngine
{
public:
	// Run script from file.
	virtual void Run( const char* )																= 0;
	// Run script from string.
	virtual void Perform( const char* )															= 0;
	// Call script function with (optional) arguments in initializer list. Optionally specify number of arguments already available from the stack.
	virtual void Call( const char*, std::initializer_list<ScriptArg> = { }, unsigned int = 0U )	= 0;
	// Register a C(++) function and name it for internal script access.
	virtual void Register( const char*, ScriptFunction )										= 0;
	// Get the number of elements on the script stack.
	virtual int  StackSize( )																	= 0;
	
	// Push

	virtual void			PushInt		( int )					= 0;
	virtual void			PushBool	( bool )				= 0;
	virtual void			PushString	( const rString& )		= 0;
	virtual void			PushFloat	( float )				= 0;
	virtual void			PushDouble	( double )				= 0;
	virtual void			PushUint	( unsigned int )		= 0;
	virtual void			PushNull	( )						= 0;

	virtual void			Push		( const ScriptArg& )	= 0;
	virtual void			PushArray	( ScriptArg*, int )		= 0;

	// Pop

	virtual int				PopInt		( )					= 0;
	virtual bool			PopBool		( )					= 0;
	virtual rString			PopString( )					= 0;
	virtual float			PopFloat	( )					= 0;
	virtual double			PopDouble	( )					= 0;
	virtual unsigned int	PopUint		( )					= 0;
	virtual void			PopNull		( )					= 0;

	virtual ScriptArg		Pop			( )					= 0;
	virtual int				PopArray	( ScriptArg*, int )	= 0;

	// Set

	virtual void			SetInt		( const char*, int )				= 0;
	virtual void			SetBool		( const char*, bool)				= 0;
	virtual void			SetString	( const char*, const rString& )		= 0;
	virtual void			SetFloat	( const char*, float )				= 0;
	virtual void			SetDouble	( const char*, double )				= 0;
	virtual void			SetUint		( const char*, unsigned int )		= 0;
	virtual void			SetNull		( const char* )						= 0;

	virtual void			Set			( const char*, const ScriptArg& )	= 0;
	virtual void			SetArray	( const char*, ScriptArg*, int )	= 0;

	// Get

	virtual int				GetInt		( const char* )						= 0;
	virtual bool			GetBool		( const char* )						= 0;
	virtual rString			GetString	( const char* )						= 0;
	virtual float			GetFloat	( const char* )						= 0;
	virtual double			GetDouble	( const char* )						= 0;
	virtual unsigned int	GetUint		( const char* )						= 0;
	virtual void			GetNull		( const char* )						= 0;

	virtual ScriptArg		Get			( const char* )	= 0;
	virtual int				GetArray	( const char*, ScriptArg*, int )	= 0;
};