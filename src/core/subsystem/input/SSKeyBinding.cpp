/**************************************************
2015 Johan Melin
***************************************************/

#include "SSKeyBinding.h"
#include "../../CompileFlags.h"
#include <SDL2/SDL_keyboard.h>
#include <input/KeyBindings.h>
#include <utility/ConfigManager.h>
#include <utility/Logger.h>
#include "../utility/SSMail.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/PlayerData.h"
#include "../../input/Action.h"

SSKeyBinding& SSKeyBinding::GetInstance( )
{
	static SSKeyBinding instance;
	return instance;
}

void SSKeyBinding::Startup( )
{
	m_ActionIdentifers.resize( ACTION_SIZE );
	m_ActionToActionIdentifier.resize( ACTION_SIZE );
	CallbackConfig* cfg = g_ConfigManager.GetConfig( g_KeyBindings.GetConfigPath( ) );
	assert( cfg );
	m_ConfigCallbackID = cfg->RegisterInterest( []( CallbackConfig* )
		{
			g_KeyBindings.ReloadConfig( );
		} );

	DEV(
		AddAction( ACTION_HEST,						"hest",						SDL_SCANCODE_SEMICOLON,		"Hest" );
		AddAction( ACTION_INCREASE_SIMULATION_RATE, "increasesimulationrate",	SDL_SCANCODE_KP_PLUS,		"Multiplies the current simulation rate by 2" );
		AddAction( ACTION_DECREASE_SIMULATION_RATE, "decreasesimulationrate",	SDL_SCANCODE_KP_MINUS,		"Divides the current simulation rate by 2" );
		AddAction( ACTION_TOGGLE_USER_LAYER,		"toggleuserlayer",			SDL_SCANCODE_F4,			"Toggle user layer" );
		AddAction( ACTION_TOGGLE_MEMORY_INFO,		"togglememoryinfo",			SDL_SCANCODE_M,				"Toggle memory usage overlay" );
		AddAction( ACTION_TOGGLE_CAMERA,			"togglecamera",				SDL_SCANCODE_B,				"Toggle between RTS camera and FPS camera" );
		AddAction( ACTION_PRINT_ENTITY_MASKS,		"printentitymasks",			SDL_SCANCODE_F9,			"Print masks of all entites." );
		AddAction( ACTION_RESET_MAX_PROFILER_TIMES, "resetmaxprofilertimes",	SDL_SCANCODE_O,				"Reset max profiler times" );
		AddAction( ACTION_SWITCH_PROFILER_SORT,		"switchprofilersort",		SDL_SCANCODE_J,				"Switch what to sort profiler entries on" );
		AddAction( ACTION_TOGGLE_PROFILE_WINDOW,	"toggleprofilewindow",		SDL_SCANCODE_F6,			"Toggle the profiling window on and off" );
		AddAction( ACTION_TOGGLE_NETWORK_DEBUG_INFO,"togglenetworkdebuginfo",	SDL_SCANCODE_N,				"Toggle network info debug window" );
	);
	// Possible dev-build stuff TODO discuss this
	AddAction( ACTION_TOGGLE_FRAMECOUNTER_INFO,		"toggleframecounterinfo",	SDL_SCANCODE_I,				"Toggle Frame time counter overlay" ); // TODOJM Should this be in non-dev build?
	AddAction( ACTION_RESET_MAX_FRAMETIME,			"resetmaxframetime",		SDL_SCANCODE_T,				"Reset max frame time counter" );
	AddAction( ACTION_PRINT_CLICKED_POSITION,		"printclickedposition",		SDL_SCANCODE_F2,			"Print clicked position" );
	AddAction( ACTION_PRINT_HOST_LATENCIES,			"printhostlatencies",		SDL_SCANCODE_F3,			"Print Latencies (Host only)" );

	AddAction( ACTION_TOGGLE_CONSOLE,				"toggleconsole",			SDL_SCANCODE_GRAVE,			"Toggle console" );

	AddAction( ACTION_GUI_ABORT,					"guiabort",					SDL_SCANCODE_ESCAPE,		"Abort a GUI operation" );
	AddAction( ACTION_OPEN_CHAT,					"openchat",					SDL_SCANCODE_RETURN,		"Open chat window" );
	AddAction( ACTION_SHOW_NETWORK_INFO,			"shownetworkinfo",			SDL_SCANCODE_TAB,			"Show the network info window" );

	AddAction( ACTION_PRINT_KEYS,					"printkeys",				SDL_SCANCODE_F1,			"Print key bindings" );

	AddAction( ACTION_JUMP_TO_SELECTION,			"jumptoselection",			SDL_SCANCODE_SPACE,			"Jump camera to selected squad(s)." );
	AddAction( ACTION_SELECT_CONTROL_GROUP_1,		"selectcontrolgroup1",		SDL_SCANCODE_1,				"Changes squad selection to control group 1" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_2,		"selectcontrolgroup2",		SDL_SCANCODE_2,				"Changes squad selection to control group 2" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_3,		"selectcontrolgroup3",		SDL_SCANCODE_3,				"Changes squad selection to control group 3" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_4,		"selectcontrolgroup4",		SDL_SCANCODE_4,				"Changes squad selection to control group 4" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_5,		"selectcontrolgroup5",		SDL_SCANCODE_5,				"Changes squad selection to control group 5" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_6,		"selectcontrolgroup6",		SDL_SCANCODE_6,				"Changes squad selection to control group 6" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_7,		"selectcontrolgroup7",		SDL_SCANCODE_7,				"Changes squad selection to control group 7" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_8,		"selectcontrolgroup8",		SDL_SCANCODE_8,				"Changes squad selection to control group 8" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_9,		"selectcontrolgroup9",		SDL_SCANCODE_9,				"Changes squad selection to control group 9" );
	AddAction( ACTION_SELECT_CONTROL_GROUP_0,		"selectcontrolgroup0",		SDL_SCANCODE_0,				"Changes squad selection to control group 0" );
	AddAction( ACTION_SELECT_UNMOVED,				"selectunmoved",			SDL_SCANCODE_J,				"Changes squad selection to unmoved squads" );

	AddAction( ACTION_PING,							"ping",						SDL_SCANCODE_LALT,			"Ping location" );

	AddAction( ACTION_AI_FORMATION_SPACEDEC,		"aiformationspacedec",		SDL_SCANCODE_COMMA,			"Decrease formation spacing" );
	AddAction( ACTION_AI_FORMATION_SPACEINC,		"aiformationspaceinc",		SDL_SCANCODE_PERIOD,		"Increase formation spacing" );
	AddAction( ACTION_AI_INVOKE_AUTOATTACK,			"aiinvokeautoattack",		SDL_SCANCODE_G,				"Invoke auto attack behaviour" );
	AddAction( ACTION_AI_FORMATION_RESET,			"aiformationreset",			SDL_SCANCODE_Y,				"Reset formation" );
	AddAction( ACTION_AI_SQUAD_MERGE,				"aisquadmerge",				SDL_SCANCODE_D,				"Merge squads" );
	AddAction( ACTION_AI_SQUAD_SPLIT,				"aisquadsplit",				SDL_SCANCODE_F,				"Split squad" );
	AddAction( ACTION_AI_ATTACK,					"aiattackmove",				SDL_SCANCODE_A,				"Attack move" );
	AddAction( ACTION_SQUADS_HOLD_POSITION,			"squadsholdposition",		SDL_SCANCODE_H,				"Hold position" );
	AddAction( ACTION_SQUADS_STOP,					"squadsstop",				SDL_SCANCODE_S,				"Stop" );

	AddAction( ACTION_CONTEXT_1,					"context1",					SDL_SCANCODE_Q,				"Contextual button 1" );
	AddAction( ACTION_CONTEXT_2,					"context2",					SDL_SCANCODE_W,				"Contextual button 2" );
	AddAction( ACTION_CONTEXT_3,					"context3",					SDL_SCANCODE_E,				"Contextual button 3" );
	AddAction( ACTION_CONTEXT_4,					"context4",					SDL_SCANCODE_R,				"Contextual button 4" );

	g_KeyBindings.ReadConfig( );
}

void SSKeyBinding::Shutdown( )
{
	CallbackConfig* cfg = g_ConfigManager.GetConfig( g_KeyBindings.GetConfigPath( ) );
	assert( cfg );
	cfg->UnregisterInterest( m_ConfigCallbackID );
	m_ActionIdentifers.clear( );
	m_ActionToActionIdentifier.clear( );
	g_KeyBindings.ClearActions( );
}

void SSKeyBinding::AddAction( ACTION action, const rString& name, SDL_Scancode scancode, const rString& description )
{
	assert( action < m_ActionIdentifers.size( ) );
	m_ActionIdentifers.at( action ) = g_KeyBindings.AddAction( name, scancode, description );
	m_ActionToActionIdentifier.at( static_cast<int>( GetActionIdentifier( action ) ) ) = action;
}

void SSKeyBinding::PrintKeys( ) const
{
	for ( auto& keybind : *g_KeyBindings.GetKeyBindCollection().GetScancodeToActionMap() )
	{
		// TODOJM: Remove this library dependand crap.
		Logger::GetStream( ) << "[C=GREEN]" << SDL_GetScancodeName( keybind.first ) << ": [C=WHITE]" << g_KeyBindings.GetDescription( keybind.second ) << std::endl;
	}
}

const rString SSKeyBinding::GetScancodeNameForAction( ACTION action, KeyBindingType bindType ) const
{
	return g_KeyBindings.GetEditableKeyBindCollection( ).GetScancodeNameForAction( GetActionIdentifier( action ), bindType );
}

SDL_Scancode SSKeyBinding::GetPrimaryScancodeFromAction( ACTION action )
{
	return g_KeyBindings.GetEditableKeyBindCollection( ).GetPrimaryScancodeFromAction( GetActionIdentifier( action ) );
}

SDL_Scancode SSKeyBinding::GetSecondaryScancodeFromAction( ACTION action )
{
	return g_KeyBindings.GetEditableKeyBindCollection( ).GetSecondaryScancodeFromAction( GetActionIdentifier( action ) );
}

bool SSKeyBinding::ConsumeFromPressStack( ACTION action ) const
{
	return g_KeyBindings.ConsumeFromPressStack( GetActionIdentifier( action ) );
}

bool SSKeyBinding::ActionUpDown( ACTION action, bool ignorePause ) const
{
	assert( action < m_ActionIdentifers.size() );
	ActionIdentifier actionIdentifier = GetActionIdentifier( action );
	if ( actionIdentifier == ActionIdentifier::invalid() )
		return false;
	return g_KeyBindings.ActionUpDown( actionIdentifier, ignorePause );
}

bool SSKeyBinding::ActionDownUp( ACTION action, bool ignorePause ) const
{
	assert( action < m_ActionIdentifers.size( ) );
	ActionIdentifier actionIdentifier = GetActionIdentifier( action );
	if ( actionIdentifier == ActionIdentifier::invalid() )
		return false;
	return g_KeyBindings.ActionDownUp( actionIdentifier, ignorePause );
}

bool SSKeyBinding::ActionUp( ACTION action, bool ignorePause ) const
{
	assert( action < m_ActionIdentifers.size( ) );
	ActionIdentifier actionIdentifier = GetActionIdentifier( action );
	if ( actionIdentifier == ActionIdentifier::invalid() )
		return true;
	return g_KeyBindings.ActionUp( actionIdentifier, ignorePause );
}

bool SSKeyBinding::ActionDown( ACTION action, bool ignorePause ) const
{
	assert( action < m_ActionIdentifers.size( ) );
	ActionIdentifier actionIdentifier = GetActionIdentifier( action );
	if ( actionIdentifier == ActionIdentifier::invalid() )
		return false;
	return g_KeyBindings.ActionDown( actionIdentifier, ignorePause );
}

const rMap<SDL_Scancode, ActionIdentifier>* SSKeyBinding::GetScancodeToActionMap( ) const
{
	return g_KeyBindings.GetKeyBindCollection().GetScancodeToActionMap( );
}

const rVector<ACTION>* SSKeyBinding::GetActionToActionIdentifier( ) const
{
	return &m_ActionToActionIdentifier;
}

ActionIdentifier SSKeyBinding::GetActionIdentifier( ACTION action ) const
{
	assert( action < m_ActionIdentifers.size( ) );
	return m_ActionIdentifers.at( action );
}

ACTION SSKeyBinding::GetAction( ActionIdentifier actionIdentifier ) const
{
	assert( static_cast<int>( actionIdentifier ) < m_ActionToActionIdentifier.size( ) );
	return m_ActionToActionIdentifier.at( static_cast<int>( actionIdentifier ) );
}
