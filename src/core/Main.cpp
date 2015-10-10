/**************************************************
2015 Robograde Development Team
***************************************************/

#include <utility/PlatformDefinitions.h>
#include "core/CompileFlags.h"
#include <memory/Alloc.h>
#ifndef GLM_FORCE_RADIANS
	#define GLM_FORCE_RADIANS	// Needs to be included before any GLM stuff is loaded.
#endif
#ifdef WINDOWS_DEBUG
	#include <crtdbg.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include "Engine.h"
#include <utility/ConfigManager.h>

void RegisterLogger( );

int main( int argc, char* argv[] )
{
	#ifdef WINDOWS_DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc( 3007 ); //Insert allocation numbers here to invoke a break at that point
	#endif
	InitMem( ); // Must be at the top of main; ALWAYS!!!

	g_ConfigManager.SetConfigFolderPath( "../../../cfg/" );
	RegisterLogger( );

	Engine engine;
	engine.Initialize();
	engine.Run();
	engine.Cleanup();

	g_ConfigManager.SaveDirty( );

	Logger::Cleanup( );
	return 0;
}

void RegisterLogger( )
{
	struct
	{
		rString name;
		rString parent;
	} loggerRegisters[] =
	{
		// NAME						PARENT		SEVERITY
		{ "Network",				"" },
		{ "NetworkEngine",			"Network" },
		{ "TCPSocket",				"Network" },
		{ "TCPListener",			"Network" },
		{ "NetworkInfo",			"Network" },
		{ "PacketPump",				"Network" },
		{ "Replicator",				"Network" },
		{ "MessagePump",			"" },
		{ "MeshImporter",			"" },
		{ "MaterialImporter",		"" },
		{ "ObjectImporter",			"" },
		{ "GUIEngine",				"" },
		{ "Graphics2D",				"" },
		{ "2DEngine",				"" },
		{ "GFX",					"" },
		{ "ParticleEngine",			"GFX" },
		{ "DecalManager", 			"GFX" },
		{ "ParticleBucket",			"ParticleEngine" },
		{ "Graphics",				"" }, // TODOHJ Is this used anywhere? No intereset for it is registered and I didn't see any log calls to it //DB
		{ "Shader",					"" },
		{ "ShaderBank",				"" },
		{ "ShaderProgram",			"" },
		{ "Texture",				"" },
		{ "ScriptEngine",			"" },
		{ "EntityFactory",			"" },
		{ "GameData",				"" },
		{ "Randomizer",				"" },
		{ "ComponentCollection",	"" },
		{ "SubsystemManager",		"" },
		{ "Subsystem",				"" },
		{ "SSInput",				"Subsystem" },
		{ "SSMail",					"Subsystem" },
		{ "SSAudio",				"Subsystem" },
		{ "SSMusicManager",			"Subsystem" },
		{ "SSSFXNotification",		"Subsystem" },
		{ "SSAI",	 				"Subsystem" },
		{ "SSCamera",				"Subsystem" },
		{ "SSKill",					"Subsystem" },
		{ "SSReplayRecorder",		"Subsystem" },
		{ "SSReplayPlayer",			"Subsystem" },
		{ "SSGameOver",				"Subsystem" },
		{ "SSControlPoint",			"Subsystem" },
		{ "SSParticle",				"Subsystem" },
		{ "SSResearch",				"Subsystem" },
		{ "SSNetworkController",	"Subsystem" },
		{ "SSConsole",				"Subsystem" },
		{ "SSAnimation",			"Subsystem" },
		{ "SSMusicManager",			"Subsystem" },
		{ "SSSceneLoader",			"Subsystem" },
		{ "SSGameLobby",			"Subsystem" },
		{ "Input", 					"" },
		{ "KeyBindings",			"Input" },
		{ "GameSpeedController",	"" },
		{ "GameModeSelector",		"" },
		{ "FileUtility",			"" },
		{ "Config",					"" },
		{ "CallbackConfig",			"" },
		{ "ConfigManager",			"" },
		{ "Editor",					"" },
		{ "EditorPicking",			"Editor" },
		{ "EditorObjectPlacer",		"Editor" },
	};

	for ( auto& entry : loggerRegisters )
		Logger::RegisterType( entry.name, entry.parent );
	
	Logger::Initialize();

	Logger::RegisterInterestFromFile( );
}
