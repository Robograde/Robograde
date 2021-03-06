/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

// You may add an action here. External usage should not use the underlaying type so it's okey to add in any order.
// Add name and description in the Initialize() function.
enum ACTION : unsigned int
{
	ACTION_HEST,
	ACTION_TOGGLE_CONSOLE,
	ACTION_TOGGLE_MEMORY_INFO,
	ACTION_TOGGLE_FRAMECOUNTER_INFO,
	ACTION_TOGGLE_CAMERA,
	ACTION_TOGGLE_PROFILE_WINDOW,

	ACTION_SHOW_NETWORK_INFO,
	ACTION_TOGGLE_NETWORK_DEBUG_INFO,

	ACTION_GUI_ABORT,

	ACTION_PRINT_KEYS,
	ACTION_PRINT_CLICKED_POSITION,
	ACTION_PRINT_HOST_LATENCIES,
	ACTION_PRINT_ENTITY_MASKS,

	ACTION_RESET_MAX_FRAMETIME,
	ACTION_RESET_MAX_PROFILER_TIMES,
	ACTION_SWITCH_PROFILER_SORT,
	
	ACTION_SQUADS_HOLD_POSITION,
	ACTION_SQUADS_STOP,
	ACTION_INCREASE_SIMULATION_RATE,
	ACTION_DECREASE_SIMULATION_RATE,
	ACTION_TOGGLE_USER_LAYER,
	ACTION_PAUSE,

	ACTION_JUMP_TO_SELECTION,
	ACTION_SELECT_CONTROL_GROUP_1,
	ACTION_SELECT_CONTROL_GROUP_2,
	ACTION_SELECT_CONTROL_GROUP_3,
	ACTION_SELECT_CONTROL_GROUP_4,
	ACTION_SELECT_CONTROL_GROUP_5,
	ACTION_SELECT_CONTROL_GROUP_6,
	ACTION_SELECT_CONTROL_GROUP_7,
	ACTION_SELECT_CONTROL_GROUP_8,
	ACTION_SELECT_CONTROL_GROUP_9,
	ACTION_SELECT_CONTROL_GROUP_0,
	ACTION_SELECT_UNMOVED,

	ACTION_PING,

	//AI Commands
	//ACTION_AI_FORMATION_CYCLE,
	ACTION_AI_FORMATION_SPACEDEC,
	ACTION_AI_FORMATION_SPACEINC,
	ACTION_AI_MOVEMENT_RELATIVE,
	ACTION_AI_INVOKE_UPGRADE,
	ACTION_AI_INVOKE_AUTOATTACK,
	ACTION_AI_FORMATION_RESET,
	ACTION_AI_SQUAD_MERGE,
	ACTION_AI_SQUAD_SPLIT,
	ACTION_AI_ATTACK,
	
	//Contextual actions
	ACTION_CONTEXT_1,
	ACTION_CONTEXT_2,
	ACTION_CONTEXT_3,
	ACTION_CONTEXT_4,
	ACTION_CONTEXT_5,
	ACTION_CONTEXT_6,
	ACTION_CONTEXT_7,
	ACTION_CONTEXT_8,

	ACTION_OPEN_CHAT,

	// Must be kept last, tracks number of elements in enum
	ACTION_SIZE,
};
