/**************************************************
2015 Johan Melin & Daniel Bengtsson
***************************************************/

#include "SubsystemManager.h"
#include "../CompileFlags.h"
#include <utility/PlatformDefinitions.h>
#if PLATFORM == PLATFORM_WINDOWS
	#include <utility/RetardedWindowsIncludes.h>
	#include "psapi.h"
#endif

#include <network/NetworkInfo.h>
#include <network/PacketPump.h>
#include <profiler/Profiler.h>
#include <profiler/AutoProfiler.h>
#include "../utility/GameSpeedController.h"
#include "../utility/GameModeSelector.h"
#include "../utility/LoadingScreen.h"
#include "../utility/GameData.h"
#include "../utility/PlayerData.h"
#include "gfx/SSRender.h"
#include "gfx/SSAnimation.h"
#include "gfx/SSFogOfWar.h"
#include "gfx/SSParticle.h"
#include "gfx/SSGFXLoader.h"
#include "gfx/SSShadow.h"
#include "gfx/SSGraphicsSettings.h"
#include "gfx/SSDecaling.h"
#include "gfx/SSCommandQueueGFX.h"
#include "gui/SSGUIRender.h"
#include "gui/SSGUIUpdate.h"
#include "gui/SSCursor.h"
#include "gui/SSGUIInfo.h"
#include "gui/SSSquadFlag.h"
#include "gui/SSResourceFlag.h"
#include "gui/SSHealthBar.h"
#include "gui/SSMiniMap.h"
#include "gui/SSUpgradeWindow.h"
#include "gui/SSSquadControlGUI.h"
#include "gui/SSChat.h"
#include "gui/SSGameTimer.h"
#include "audio/SSAudio.h"
#include "audio/SSAudioSettings.h"
#include "audio/SSMusicManager.h"
#include "audio/SSSFXEmitter.h"
#include "audio/SSSFXNotification.h"
#include "network/SSNetworkController.h"
#include "network/SSNetworkInput.h"
#include "network/SSNetworkOutput.h"
#include "network/SSNetworkInfoWindow.h"
#include "input/SSInput.h"
#include "input/SSConsole.h"
#include "input/SSPicking.h"
#include "input/SSButtonInput.h"
#include "input/SSKeyBinding.h"
#include "utility/SSMail.h"
#include "utility/SSFrameCounter.h"
#include "utility/SSMemoryDebug.h"
#include "utility/SSStartupScript.h"
#include "utility/SSProfilerWindow.h"
#include "hashing/SSHashGenerator.h"
#include "hashing/SSHashComparison.h"
#include "replay/SSReplayRecorder.h"
#include "replay/SSMetaDataRecorder.h"
#include "replay/SSReplayPlayer.h"
#include "gamelogic/SSAI.h"
#include "gamelogic/SSCollision.h"
#include "gamelogic/SSCamera.h"
#include "gamelogic/SSKill.h"
#include "gamelogic/SSTerrainFollow.h"
#include "gamelogic/SSGameOver.h"
#include "gamelogic/SSControlPoint.h"
#include "gamelogic/SSChildMover.h"
#include "gamelogic/SSProjectiles.h"
#include "gamelogic/SSUpgrades.h"
#include "gamelogic/SSResearch.h"
#include "gamelogic/SSKillAll.h"
#include "gamelogic/SSSceneLoader.h"
#include "gamelogic/SSRadar.h"
#include "gamelogic/SSResourceManager.h"
#include "gamelogic/SSDoodad.h"
#include "gamelogic/SSParticleEmitter.h"
#include "menu/SSMainMenu.h"
#include "menu/SSOptionsMenu.h"
#include "menu/SSGameLobby.h"
#include "menu/SSGameLobbySP.h"
#include "menu/SSInGameMenu.h"
#include "menu/SSReplaySelectMenu.h"
#include "menu/SSHowTo.h"

//Editor specific subsystems
#include "editor/SSEditorMain.h"
#include "editor/SSEditorEvents.h"
#include "editor/SSEditorSFXEmitter.h"
#include "editor/SSEditorCameraPaths.h"
#include "editor/SSEditorToolbox.h"
#include "editor/SSEditorObjectPlacer.h"
#include "editor/SSColourPicker.h"
#include "editor/SSEditorParticle.h"
#include "editor/SSEditorTerrain.h"
SubsystemManager& SubsystemManager::GetInstance()
{
	static SubsystemManager instance;
	return instance;
}

SubsystemManager::SubsystemManager()
{}

void SubsystemManager::Startup()
{
	// Set priorities (100 is default). Startup, Update and Shutdown priority can be defined
	g_SSKeyBinding.			SetStartOrderPriority( 2U );		// Before input stuff.						|
	g_SSGraphicsSettings. 	SetStartOrderPriority( 4U );		// Before graphics related systems			|
	g_SSGUIRender.			SetStartOrderPriority( 5U );		// Before anything that use the GUI			|
	g_SSGUIUpdate.			SetStartOrderPriority( 6U );		// Before anything that uses debug windows	|
	g_SSAudio.				SetStartOrderPriority( 10U );		// Before all other sound stuff				|
	g_SSMusicManager.		SetStartOrderPriority( 11U );		//											| After SSAudio		
	g_SSControlPoint.		SetStartOrderPriority( 15U );		// Before Scene loader						|
	g_SSReplayPlayer.		SetStartOrderPriority( 20U );		// Before all simulation randomizations		|
	g_SSUpgrades.			SetStartOrderPriority( 40U );		// Before Sceneloader						|
	g_SSSceneLoader.		SetStartOrderPriority( 50U );		// Before the level and entities are needed	| After Graphical assets are loaded
	g_SSKill.				SetStartOrderPriority( 75U );		// Before SSParticle						|
	g_SSParticle.			SetStartOrderPriority( 85U );		// 											| After SSParticle
	g_SSResearch.			SetStartOrderPriority( 91U );		//											| After Upgrades
	g_SSHealthBar.			SetStartOrderPriority( 92U );		// Before Squad flags						| 
	g_SSResourceFlag.		SetStartOrderPriority( 93U );		// Before squad flags to be drawn bellow	| After GUIUpdate
	g_SSSquadFlag.			SetStartOrderPriority( 94U );		// Before any other gui windows				| After GUIUpdate
	g_SSUpgradeWindow.		SetStartOrderPriority( 97U );		//											| After Upgrades & Research
	g_SSEditorMain.			SetStartOrderPriority( 97U );		// Before any other GUI windows				| 
	g_SSAI.					SetStartOrderPriority( 100U );		// Before camera							|
	g_SSSquadControlGUI.	SetStartOrderPriority( 110U );		//											| After SSInput
	g_SSAudioSettings.		SetStartOrderPriority( 125U );		// After all other sound stuff				|
	g_SSCamera.				SetStartOrderPriority( 150U );		//											| After squad creation.
	g_SSGFXLoader.			SetStartOrderPriority( 160U );		// Before models and textures are needed	|
	g_SSCursor.				SetStartOrderPriority( 170U );		//											| After everything
	g_SSStartupScript.		SetStartOrderPriority( 200U );		//											| After everything that could be changed from scripts

	// ************** BEFORE GAME LOGIC **************
	g_SSInput.				SetUpdateOrderPriority( 1U );		// Before SSMail							|
	g_SSCamera.				SetUpdateOrderPriority( 2U );
	g_SSGUIUpdate.			SetUpdateOrderPriority( 3U );		// Before picking							|
	g_SSSquadControlGUI.	SetUpdateOrderPriority( 5U );		//											| After SSInput
	g_SSNetworkInput.		SetUpdateOrderPriority( 7U );		// Before SSMail							|
	g_SSReplayPlayer.		SetUpdateOrderPriority( 9U );		// Before SSMail							|
	g_SSUpgradeWindow.		SetUpdateOrderPriority( 11U );		// Before SSMail							|
	g_SSHealthBar.			SetUpdateOrderPriority( 13U );		// Before squad flags						|
	g_SSSquadFlag.			SetUpdateOrderPriority( 15U );		// Before SSMail and picking				|
	g_SSResourceFlag.		SetUpdateOrderPriority( 16U );		// Before SSMail and picking				|
	g_SSMiniMap.			SetUpdateOrderPriority( 17U );		// Before SSPicking							|
	g_SSConsole.			SetUpdateOrderPriority( 19U );		// Before SSPicking, chat					| After input
	g_SSPicking.			SetUpdateOrderPriority( 21U );		// Before SSMail							| After input
	g_SSEditorToolbox.		SetUpdateOrderPriority(	22U	);		// Before SSMail							| After Input
	g_SSMail.				SetUpdateOrderPriority( 23U );		// Before game logic						| After messages aimed for this frame
	g_SSReplayRecorder.		SetUpdateOrderPriority( 25U );		//											| After SSMail (ASAP to avoid unrecorded crashes)
	g_SSNetworkOutput.		SetUpdateOrderPriority( 27U );		//											| After SSMail (ASAP to reduce latency)
	g_SSMainMenu.			SetUpdateOrderPriority( 28U );		//											| After SSMusicManager(music is loaded during startup)
	// ************** GAME LOGIC ********************
	g_SSProjectiles.		SetUpdateOrderPriority( 75U );		// 											| After SSAI	
	g_SSParticle.			SetUpdateOrderPriority( 95U );		// Before Camera							| After rendering
	g_SSUpgrades.			SetUpdateOrderPriority( 123U );
	g_SSAI.					SetUpdateOrderPriority( 124U );		//											| After AI and childmover
	g_SSTerrainFollow.		SetUpdateOrderPriority( 130U );		// Before culling							| After things moving objects
	g_SSChildMover.			SetUpdateOrderPriority( 135U );		//											| After moving entities
	g_SSCollision.			SetUpdateOrderPriority( 190U );		// Before rendering (because of culling)	|
	g_SSControlPoint.		SetUpdateOrderPriority( 191U );		// Before SSGamoeOver						| After game logic
	g_SSGameOver.			SetUpdateOrderPriority( 192U );		// Before SSReplay							| After game logic
	// ************** After GAME LOGIC **************
	g_SSDecaling. 			SetUpdateOrderPriority( 193U ); 	// 											| After units movement
	g_SSHashGenerator.		SetUpdateOrderPriority( 194U );		// Before anthing that reads the hash		| After game logic
	g_SSNetworkController.	SetUpdateOrderPriority( 195U );		// Before render(render might be slow)		| After SSHashGenerator and game logic
	g_SSAnimation.			SetUpdateOrderPriority( 196U );		// Before SSRender							| After SSUpgrade
	g_SSHashComparison.		SetUpdateOrderPriority( 197U );		//											| After SSHashGenerator and game logic
	g_SSMetaDataRecorder.	SetUpdateOrderPriority( 199U );		//											| After SSHashGenerator and game logic
	g_SSRender.				SetUpdateOrderPriority( 200U );		//											| After game logic
	g_SSUpgradeWindow.		SetUpdateOrderPriority( 206U );		// 											| After Upgrade & Research
	g_SSNetworkInfoWindow. 	SetUpdateOrderPriority( 220U );		// Before GUI render						| After SSNetworkInput
	g_SSButtonInput.		SetUpdateOrderPriority( 230U );		//											| After UpgradeWindow
	g_SSGUIInfo.			SetUpdateOrderPriority( 240U );		//											| After anything that use GUI buttons
	g_SSCommandQueueGFX.	SetUpdateOrderPriority( 280U );		// Before render							|
	g_SSDoodad.				SetUpdateOrderPriority( 290U );		// Before render							| 
	g_SSGUIRender.			SetUpdateOrderPriority( 300U );		//											| After anything that needs GUI

	// Shutdown priorities
	g_SSReplayRecorder.SetShutdownOrderPriority( 90U );		//	Before SSAI									|
	g_SSKillAll.SetShutdownOrderPriority( 150U );			// TODOJM: Set order description

	#define Singleplayer		GameModeType::Singleplayer
	#define SingleplayerAI		GameModeType::SingleplayerAI
	#define AIOnly				GameModeType::AIOnly
	#define Multiplayer			GameModeType::Multiplayer
	#define Replay				GameModeType::Replay
	#define DedicatedServer		GameModeType::DedicatedServer
	#define ConsoleOnly			GameModeType::ConsoleOnly
	#define NetworkLobby		GameModeType::NetworkLobby
	#define SPLobby				GameModeType::SinglePlayerLobby
	#define MainMenu			GameModeType::MainMenu
	#define Editor				GameModeType::Editor
	
	// Decide in what modes subsystems should be active here				Singleplayer		SingleplayerAI		Ai only 	Multiplayer		Replay		Dedicated Server	Console Only	Network Lobby	SPLobby		Main Menu	Editor
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSRender,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGUIUpdate,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,	SPLobby,	MainMenu,		Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGUIRender,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,	SPLobby,	MainMenu,		Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSCursor,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,	SPLobby,	MainMenu,		Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSAI,					{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSAudio,					{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,											NetworkLobby,	SPLobby,	MainMenu,		Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSAudioSettings,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,											NetworkLobby,	SPLobby,	MainMenu,		Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSMusicManager,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,											NetworkLobby,	SPLobby,	MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSSFXEmitter,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSSFXNotification,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,											NetworkLobby,	SPLobby,	MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSMail,					{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,					NetworkLobby,	SPLobby,	MainMenu,		Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSCollision,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSCamera,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSKill,					{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSConsole,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,	SPLobby,	MainMenu,		Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSTerrainFollow,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSChildMover,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSControlPoint,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGameOver,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,				DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSHashComparison,		{													Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSUpgradeWindow,			{ Singleplayer,		SingleplayerAI,					Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSSquadFlag,				{ Singleplayer,		SingleplayerAI,					Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSResourceFlag,			{ Singleplayer,		SingleplayerAI,					Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSDecaling,				{ Singleplayer,		SingleplayerAI,					Multiplayer,	Replay,																						Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSHealthBar,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSRadar,					{ Singleplayer,		SingleplayerAI,					Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSResourceManager,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSHashGenerator,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSParticle,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSProjectiles,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSUpgrades,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSResearch,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSAnimation,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSMiniMap,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSFogOfWar,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSShadow,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSKillAll,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGFXLoader,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																						Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSSceneLoader,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,		DedicatedServer,																Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSInput,					{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSPicking,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSButtonInput,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,																								Editor	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSReplayRecorder,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,				DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSMetaDataRecorder,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,				DedicatedServer,																		} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSReplayPlayer,			{																	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSNetworkController,		{													Multiplayer,				DedicatedServer,					NetworkLobby,								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSNetworkInput,			{													Multiplayer,				DedicatedServer,					NetworkLobby,								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSNetworkOutput,			{													Multiplayer,				DedicatedServer,					NetworkLobby,								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSChat,					{													Multiplayer,													NetworkLobby,								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSSquadControlGUI,		{ Singleplayer,		SingleplayerAI,					Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGUIInfo,				{ Singleplayer,		SingleplayerAI,					Multiplayer,	Replay,																						Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSMainMenu,				{																																				MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSOptionsMenu,			{																																				MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSReplaySelectMenu,		{																																				MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSHowTo,					{																																				MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGameLobby,				{																													NetworkLobby,										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGameLobbySP,			{																																	SPLobby,							} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSProfilerWindow,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,	SPLobby,	MainMenu,				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSNetworkInfoWindow,		{													Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSKeyBinding,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,						 	ConsoleOnly, 	NetworkLobby, 	SPLobby,	MainMenu, 		Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGraphicsSettings,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay, 						ConsoleOnly, 	NetworkLobby, 	SPLobby,	MainMenu, 				} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSInGameMenu,			{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSGameTimer,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSDoodad,				{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,																								} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSCommandQueueGFX,		{ Singleplayer,		SingleplayerAI,					Multiplayer,																										} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSParticleEmitter,		{																																								Editor	} );
	//Editor specific subsystems
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorMain,			{																																								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorEvents,			{																																								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorSFXEmitter,		{																																								Editor,	} );	
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorCameraPaths,		{																																								Editor, } );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorToolbox,			{																																								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorObjectPlacer,	{																																								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSColourPicker,			{																																								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorParticle,		{																																								Editor,	} );
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSEditorTerrain,			{																																								Editor, } );
	
	// Pilla bli på denna. SKA LIGGA HÄR. TODOJM move to dev only build? May want users to have this, but should default to toggled off?
	g_GameModeSelector.AddSubsystemToGameModes( &g_SSFrameCounter,		{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,				MainMenu,	Editor,		} );
	DEV(
		g_GameModeSelector.AddSubsystemToGameModes( &g_SSMemoryDebug,	{ Singleplayer,		SingleplayerAI,		AIOnly,		Multiplayer,	Replay,							ConsoleOnly,	NetworkLobby,				MainMenu,				} );
		g_GameModeSelector.AddSubsystemToGameModes( &g_SSStartupScript,	{ Singleplayer,		SingleplayerAI,		AIOnly,																															} );
	);

	g_GameSpeedController.Startup();
}

void SubsystemManager::Update( const GameTimer::DeltaTimeInfo& deltaTimeInfo )
{
	// Check if game mode should be switched
	g_GameModeSelector.Update();

	// Update the game
	bool shouldUpdateSimulation = g_GameSpeedController.ShouldUpdateSimLayer( deltaTimeInfo ); // Check if it's time to take a simulation step
	bool shouldUpdateUserLayer = g_GameSpeedController.ShouldUpdateUserLayer();
	
	g_GameSpeedController.Update();

	for ( auto subsystem : m_UpdateOrder )
	{
		// Update every subsystem as long as it isn't paused
		if ( subsystem->IsStarted() == true && subsystem->IsPaused( ) == false )
		{
			PROFILE( AutoProfiler pSS( subsystem->GetName( ).c_str() ) ); // Start profiling for the current subsystem

			if ( shouldUpdateSimulation ) // Only update the simulation layer if it is time to do so
			{
				PROFILE( AutoProfiler pSSS( "UpdateSimLayer", Profiler::PROFILER_CATEGORY_MAJOR, true ) );
				subsystem->UpdateSimLayer( DELTA_TIME_STEP );
				PROFILE( pSSS.Stop() );
			}

			if ( shouldUpdateUserLayer )
			{
				PROFILE( AutoProfiler pSSU( "UpdateUserLayer", Profiler::PROFILER_CATEGORY_MAJOR, true ) );
				subsystem->UpdateUserLayer( deltaTimeInfo.DeltaTime ); // Always update the user layer
				PROFILE( pSSU.Stop() );
				PROFILE( pSS.Stop() );
			}
		}
	}

	if ( shouldUpdateSimulation )
		g_GameData.IncrementFrameCount( ); // Report that a simulation step was taken
}

void SubsystemManager::Shutdown()
{
	for ( auto subsystem : m_ShutdownOrder )
	{
		if ( subsystem->IsStarted() )
		{
			subsystem->ShutdownFromCollection();
		}
	}
}

void SubsystemManager::AddSubsystem( Subsystem* subsystem )
{
	// Add subsystem to map only if it isn't found in it.
	auto it = m_SubsystemMap.find( subsystem->GetName( ) );
	if ( it != m_SubsystemMap.end( ) )
	{
		// Warn about it being in the map already, not standard behaviour.
		Logger::Log( subsystem->GetName( ) + " subsystem is already added. Nothing will be done.", "SubsystemManager", LogSeverity::WARNING_MSG );
		return;
	}
	else
		m_SubsystemMap.emplace( subsystem->GetName( ), subsystem );

	if ( m_StartupOrder.empty( ) )
	{
		m_StartupOrder.push_back( subsystem );
		m_UpdateOrder.push_back( subsystem );
		m_ShutdownOrder.push_back( subsystem );
	}
	else
	{
		AddStartup( subsystem );
		AddUpdate( subsystem );
		AddShutdown( subsystem );
	}
}

void SubsystemManager::StartAllUnstarted()
{
	for ( auto subsystem : m_StartupOrder )
	{
		if ( !subsystem->IsStarted() )
			subsystem->StartupFromCollection();
	}
}

bool SubsystemManager::StartSubsystem( const rString& name )
{
	auto subsystem = m_SubsystemMap.find( name );
	if ( subsystem != m_SubsystemMap.end() )
	{
		if ( !subsystem->second->IsStarted() )
		{
			subsystem->second->StartupFromCollection();
			return true;
		}
		else
		{
			Logger::Log( "Cannot start subsystem <" + name + ">. It is already started", "SubsystemManager", LogSeverity::WARNING_MSG );
			return false;
		}
	}
	Logger::Log( "Cannot start subsystem <" + name + ">. No subsystem with that name exists", "SubsystemManager", LogSeverity::WARNING_MSG );
	return false;
}

void SubsystemManager::StartSubsystems( const rVector<Subsystem*>& subsystemsToStart )
{
	bool loadingScreen = false;
	const GameModeType& gm = g_GameModeSelector.GetCurrentGameMode().Type;
	if( gm == Singleplayer || gm == Multiplayer || gm == SingleplayerAI || gm == AIOnly || gm == Replay )
		loadingScreen = true;

	int loaded = 0;
	for ( int i = 0; i < m_StartupOrder.size(); ++i )
	{
		for ( int j = 0; j < subsystemsToStart.size(); ++j )
		{
			if ( *m_StartupOrder[i] == *subsystemsToStart[j] )
			{
				if( loadingScreen )
					g_LoadingScreen.Render( "Starting system: " + m_StartupOrder[i]->GetUserFriendlyName(), loaded, static_cast<int>( subsystemsToStart.size() ) );


#if PLATFORM == PLATFORM_WINDOWS && DEBUG_SUBSYSTEM_MEMORY_STARTUP == 1
				SIZE_T memUsageBefore = 0;
				SIZE_T memUsageAfter = 0;

				PROCESS_MEMORY_COUNTERS memCounter;
				GetProcessMemoryInfo( GetCurrentProcess( ), &memCounter, sizeof( memCounter ) );
				memUsageBefore = memCounter.PagefileUsage;
#endif

				m_StartupOrder[i]->StartupFromCollection();

#if PLATFORM == PLATFORM_WINDOWS && DEBUG_SUBSYSTEM_MEMORY_STARTUP == 1
				GetProcessMemoryInfo( GetCurrentProcess( ), &memCounter, sizeof( memCounter ) );
				memUsageAfter = memCounter.PagefileUsage;
				Logger::Log( "Subsystem \"" + m_StartupOrder[i]->GetName( ) + "\" grew " + rToString( memUsageAfter - memUsageBefore ) +
					" bytes in non-allocator memory.", "SubsystemManager", LogSeverity::DEBUG_MSG );
#endif


				loaded++;

				if ( g_GameModeSelector.GetCurrentGameMode().Type == Multiplayer && g_NetworkInfo.AmIHost() )
					g_SSNetworkInput.UpdateUserLayer( DELTA_TIME_STEP );

				break;
			}
		}
	}

	if ( g_GameModeSelector.GetCurrentGameMode().Type == Multiplayer )
	{
		UserSignalMessage loadingFinishedMessage = UserSignalMessage( UserSignalType::FINISHED_LOADING, g_PlayerData.GetPlayerID() );

		if ( g_NetworkInfo.AmIHost() )
		{
			g_PacketPump.SendToAll( loadingFinishedMessage );
			g_SSNetworkController.SetPlayerFinishedLoading( g_PlayerData.GetPlayerID() );
		}
		else
			g_PacketPump.Send( loadingFinishedMessage );

		g_SSNetworkController.SetPlayerFinishedLoading( g_PlayerData.GetPlayerID() );

		const rMap<short, NetworkPlayer>& networkPlayers = g_SSNetworkController.GetNetworkedPlayers();
		while ( !g_SSNetworkController.AllFinishedLoading() )
		{
			MemoryAllocator::ResetPos();
			rString toOutput = "Waiting for player(s): ";
			for ( auto player : networkPlayers )
			{
				if ( !player.second.HasFinishedLoading )
					toOutput += player.second.Name + ", ";
			}

			auto commaToErase = toOutput.find_last_of( ',' );
			if ( commaToErase != string::npos )
				toOutput.erase( commaToErase );

			g_SSNetworkInput.UpdateUserLayer( DELTA_TIME_STEP );
			g_LoadingScreen.Render( toOutput, loaded, static_cast<int>( subsystemsToStart.size() ) );
		}
	}
}

bool SubsystemManager::StopSubsystem( const rString& name )
{
	auto subsystem = m_SubsystemMap.find( name );
	if ( subsystem != m_SubsystemMap.end() )
	{
		if ( subsystem->second->IsStarted() )
		{
			subsystem->second->ShutdownFromCollection();
			return true;
		}
		else
		{
			Logger::Log( "Cannot stop subsystem <" + name + ">. It is not started", "SubsystemManager", LogSeverity::WARNING_MSG );
			return false;
		}
	}
	Logger::Log( "Cannot stop subsystem <" + name + ">. No subsystem with that name exists", "SubsystemManager", LogSeverity::WARNING_MSG );
	return false;
}

void SubsystemManager::StopSubsystems( const rVector<Subsystem*>& subsystemsTopStop )
{
	if ( !subsystemsTopStop.empty() )
	{
		for ( int i = 0; i < m_ShutdownOrder.size(); ++i )
		{
			for ( int j = 0; j < subsystemsTopStop.size(); ++j )
			{
				if ( *m_ShutdownOrder[i] == *subsystemsTopStop[j] )
				{
#if PLATFORM == PLATFORM_WINDOWS && DEBUG_SUBSYSTEM_MEMORY_STARTUP == 1
					SIZE_T memUsageBefore = 0;
					SIZE_T memUsageAfter = 0;

					PROCESS_MEMORY_COUNTERS memCounter;
					GetProcessMemoryInfo( GetCurrentProcess( ), &memCounter, sizeof( memCounter ) );
					memUsageBefore = memCounter.PagefileUsage;
#endif

					m_ShutdownOrder[i]->ShutdownFromCollection();

#if PLATFORM == PLATFORM_WINDOWS && DEBUG_SUBSYSTEM_MEMORY_STARTUP == 1
					GetProcessMemoryInfo( GetCurrentProcess( ), &memCounter, sizeof( memCounter ) );
					memUsageAfter = memCounter.PagefileUsage;
					Logger::Log( "Subsystem \"" + m_StartupOrder[i]->GetName( ) + "\" shrank " + rToString( memUsageBefore - memUsageAfter ) +
						" bytes in non-allocator memory.", "SubsystemManager", LogSeverity::DEBUG_MSG );
#endif
					break;
				}
			}
		}
	}
}

void SubsystemManager::Restart()
{
	Logger::Log( "Restarting subsystems.", "SubsystemManager", LogSeverity::DEBUG_MSG );
	Shutdown();
	Logger::Log( "Subsystems shut down.", "SubsystemManager", LogSeverity::DEBUG_MSG );
	Logger::Log( "Starting subsystems.", "SubsystemManager", LogSeverity::DEBUG_MSG );
	Startup();
	Logger::Log( "Subsystems started.", "SubsystemManager", LogSeverity::DEBUG_MSG );
}

void SubsystemManager::RestartSubsystem( const rString& subsystemName )
{
	auto subsystemToRestart = m_SubsystemMap.find(subsystemName);
	if ( subsystemToRestart == m_SubsystemMap.end() )
	{
		Logger::Log( "Tried to restart non existant subsystem: " + subsystemName, "SubsystemManager", LogSeverity::WARNING_MSG );
	}
	else
	{
		if ( subsystemToRestart->second->IsStarted() )
		{
			subsystemToRestart->second->ShutdownFromCollection();
			subsystemToRestart->second->StartupFromCollection();
		}
		else
		{
			Logger::Log( "Tried to restart non-started subsystem. Nothing will happen", "SubsystemManager", LogSeverity::DEBUG_MSG );
		}
	}
}

void SubsystemManager::PauseSubsystem( const rString& subsystemName )
{
	auto it = m_SubsystemMap.find( subsystemName );
	if ( it == m_SubsystemMap.end( ) )
		Logger::Log( "Tried to pause non existant subsystem: " + subsystemName, "SubsystemManager",
		LogSeverity::WARNING_MSG );
	else
		it->second->Pause( );
}

void SubsystemManager::UnpauseSubsystem( const rString& subsystemName )
{
	auto it = m_SubsystemMap.find( subsystemName );
	if ( it == m_SubsystemMap.end( ) )
		Logger::Log( "Tried to unpause non existant subsystem: " + subsystemName, "SubsystemManager",
		LogSeverity::WARNING_MSG );
	else
		it->second->Unpause( );
}

const rVector<Subsystem*>* SubsystemManager::GetUpdateCollection() const
{
	return &m_UpdateOrder;
}

void SubsystemManager::PrintStartupOrder() const
{
	PrintOrder( m_StartupOrder );
}

void SubsystemManager::PrintUpdateOrder() const
{
	PrintOrder( m_UpdateOrder );	
}

void SubsystemManager::PrintShutdownOrder() const
{
	PrintOrder( m_ShutdownOrder );	
}

void SubsystemManager::PrintStartedSubsystems() const
{
	Logger::GetStream() << "Printing started subsystems in update order." << std::endl;

	int index = 0;
	for ( auto& subsystem : m_UpdateOrder )
	{
		if ( subsystem->IsStarted() )
		{
			Logger::GetStream() << index++ << ": " << subsystem->GetName() << std::endl;
		}
	}
}

void SubsystemManager::PrintUnstartedSubsystems() const
{
	Logger::GetStream() << "Printing unstarted subsystems in startup order." << std::endl;

	int index = 0;
	for ( auto& subsystem : m_StartupOrder )
	{
		if ( !subsystem->IsStarted() )
		{
			Logger::GetStream() << index++ << ": " << subsystem->GetName() << std::endl;
		}
	}
}

// Used by std::lower_bound to insert new subsystems in correct startup order.
bool CompareStartup( const Subsystem* lhs, const Subsystem* rhs )
{
	return lhs->GetStartOrderPriority( ) < rhs->GetStartOrderPriority( );
}

void SubsystemManager::AddStartup( Subsystem* subsystem )
{
	auto it = std::lower_bound( m_StartupOrder.begin( ), m_StartupOrder.end( ), subsystem, CompareStartup );
	m_StartupOrder.insert( it, subsystem );
}

// Used by std::lower_bound to insert new subsystems in correct update order.
bool CompareUpdate( const Subsystem* lhs, const Subsystem* rhs )
{
	return lhs->GetUpdateOrderPriority( ) < rhs->GetUpdateOrderPriority( );
}

void SubsystemManager::AddUpdate( Subsystem* subsystem )
{
	auto it = std::lower_bound( m_UpdateOrder.begin( ), m_UpdateOrder.end( ), subsystem, CompareUpdate );
	m_UpdateOrder.insert( it, subsystem );
}

// Used by std::lower_bound to insert new subsystems in correct shutdown order.
bool CompareShutdown( const Subsystem* lhs, const Subsystem* rhs ) // TODOJM: Shouldn't this be a private member function?
{
	return lhs->GetShutdownOrderPriority( ) < rhs->GetShutdownOrderPriority( );
}

void SubsystemManager::AddShutdown( Subsystem* subsystem )
{
	auto it = std::lower_bound( m_ShutdownOrder.begin( ), m_ShutdownOrder.end( ), subsystem, CompareShutdown );
	m_ShutdownOrder.insert( it, subsystem );
}

void SubsystemManager::PrintOrder( const rVector<Subsystem*>& orderToPrint ) const
{
	int index = 0;
	for ( auto& subsystem : orderToPrint )
	{
		Logger::GetStream() << index++ << ": " << subsystem->GetName() << std::endl;
	}
}
