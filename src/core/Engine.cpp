/**************************************************
2015 Robograde Development Team
***************************************************/

#include "Engine.h"
#include "CompileFlags.h"
#include <glm/gtx/transform.hpp>
#include <input/TextInput.h>
#include <profiler/Profiler.h>
#include <profiler/AutoProfiler.h>
#include <utility/Randomizer.h>
#include <network/PacketPump.h>

#include <gfx/LightEngine.h>
#include <gfx/MaterialBank.h>
#include <input/KeyBindings.h>
#include <gfx/GraphicsEngine.h>

#include "component/ModelComponent.h"
#include "component/CollisionComponent.h"
#include "component/SelectionComponent.h"
#include "component/DoodadComponent.h"
#include "component/WeaponComponent.h"
#include "component/OwnerComponent.h"
#include "component/ColourComponent.h"
#include "component/ChildComponent.h"
#include "component/ParentComponent.h"
#include "component/ControlPointComponent.h"
#include "component/SFXEmitterComponent.h"
#include "component/ParticleEmitterComponent.h"
#include "component/UpgradeComponent.h"
#include "component/VisionComponent.h"
#include "component/AnimationComponent.h"
#include "component/TerrainFollowComponent.h"
#include "component/DecalComponent.h"
#include "component/StealthComponent.h"
#include "component/FollowMouseComponent.h"

#include "subsystem/SubsystemManager.h"
#include "subsystem/SubsystemManagerScriptBinder.h"
#include "subsystem/utility/SSMemoryDebug.h"
#include "subsystem/gfx/SSAnimation.h"

#include "datadriven/CollectionFactory.h"
#include "EntityFactory.h"
#include "ai/Terrain.h"
#include "utility/SceneLoader.h"
#include "utility/GameModeSelector.h"
#include "utility/GameData.h"
#include <utility/Colours.h> // TODOJM Remove
#include "binding/ConfigScriptBindings.h"

bool Engine::Initialize()
{
	// We will initialize all SDL subsystems on by one
	SDL_Init( 0 );

	// Profiler needs the timer subsystem
	if ( SDL_InitSubSystem( SDL_INIT_TIMER ) != 0 )
	{
		Logger::Log( "Failed to initialize SDL timer subsystem", "SDL", LogSeverity::ERROR_MSG );
		assert( false );
	}
	g_Profiler.GetInstance().SetFrequency( SDL_GetPerformanceFrequency() );

	// Several systems need the event system
	if ( SDL_InitSubSystem( SDL_INIT_EVENTS ) != 0 )
	{
		Logger::Log( "Failed to initialize SDL event subsystem", "SDL", LogSeverity::ERROR_MSG );
		assert( false );
	}

	// Seed the randomizer
	const unsigned int randomSeed = static_cast<const unsigned int>( SDL_GetPerformanceCounter() );
	g_Randomizer.Seed( randomSeed );

	g_Script.Initialize();
	EntityFactory::ScriptInitializeBindings();
	ConfigScriptBinder::BindConfigScripts();

	g_GameData.Reset();
	g_GameData.ReadGameSettingsConfig();
	if ( !g_GameData.IsDedicatedServer() )
	{
		g_Input->Initialize();
		g_TextInput.Initialize();
		InitializeGFX();
		g_GUI.Initialize( g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight() );
	}

	if ( g_NetworkEngine.Initialize() == false )
		return false;

	InitializeSubsystems();

	return true;
}

void Engine::Run()
{
	GameTimer gameTimer;
	gameTimer.Start();
	gameTimer.Tick();

	SDL_Event event;
	bool quit = false;
	while ( quit == false )
	{
		gameTimer.Tick();
		if ( !g_GameData.IsDedicatedServer() )
		{
			g_Input->Update();
			while ( SDL_PollEvent( &event ) > 0 )
			{
				g_TextInput.HandleEvents( event );
				g_Input->HandleEvent( event );
				switch ( event.type )
				{
					case SDL_QUIT:
						Logger::Log( "SDL received quit message, shutting down...", "Engine", LogSeverity::INFO_MSG );
						quit = true;
						break;
				}
			}

		PROFILE( AutoProfiler pRender( "TotalRender", Profiler::PROFILER_CATEGORY_MAJOR ) );
			Render();
			PROFILE( pRender.Stop() );
		}

		PROFILE( AutoProfiler pUpdate( "TotalUpdate", Profiler::PROFILER_CATEGORY_MAJOR ) );
		Update( gameTimer.GetDeltaTimeInfo( ) );
		PROFILE( pUpdate.Stop( ) );

		if ( !g_GameData.IsDedicatedServer() )
		{
			gfx::g_GFXEngine.Swap();

			// Reset allocator counters and set values to UI
			g_SSMemoryDebug.SetMemoryMisses( MemoryAllocator::NrMisses() );
			g_SSMemoryDebug.SetMemoryAllocations( MemoryAllocator::NrAllocations() );
			MemoryAllocator::NrMisses() = 0ULL;
			MemoryAllocator::NrAllocations() = 0ULL;

			// Reset allocator state
			g_SSMemoryDebug.UpdateDistance();
		}
		MemoryAllocator::ResetPos( );
		FrameAllocator::Reset( );

		// Reset profiler frame
		g_Profiler.GetInstance().ResetFrame( );
	}
}

void Engine::Cleanup()
{
	g_SubsystemManager.Shutdown();

	g_NetworkEngine.Stop( );
	g_NetworkEngine.Terminate( );
}

void Engine::Update( const GameTimer::DeltaTimeInfo& deltaTimeInfo )
{
	PROFILE( AutoProfiler pSSCollectionUpdate( "SSCollection" ) );
	g_SubsystemManager.Update( deltaTimeInfo );
	PROFILE( pSSCollectionUpdate.Stop() );
}

void Engine::Render()
{
#if AI_REMOVE_MESHES == 0
	PROFILE( AutoProfiler pGFXRender( "GFX" ) );
	gfx::g_GFXEngine.Draw();
	PROFILE( pGFXRender.Stop() );
#endif
}

bool Engine::InitializeGFX()
{
	g_GameData.ReadGraphicsConfig();
	// GFX needs to be initialized before the render manager
	bool useFogOfWar = !(g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Editor);
	gfx::g_GFXEngine.InitializeWindow( g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight(), g_GameData.m_MSAA, g_GameData.m_Fullscreen,
		g_GameData.m_Vsync, useFogOfWar);
	m_debugRenderer.Initialize();

	return true;
}

bool Engine::InitializeSubsystems()
{
	BindSubsystemManagerToLua();

	// Add new component types here
	rVector<BaseCollection*>* collections = GetCollections <
											PlacementComponent,			// 0
											ModelComponent,				// 1
											AgentComponent,				// 2
											CollisionComponent,			// 3
											ResourceComponent,			// 4
											HealthComponent,			// 5
											SFXEmitterComponent,		// 6
											WeaponComponent,			// 7
											SelectionComponent,			// 8
											DoodadComponent,			// 9
											OwnerComponent,				// 10
											ColourComponent,			// 11
											ChildComponent,				// 12
											ParentComponent,			// 13
											ControlPointComponent,		// 14
											UpgradeComponent,			// 15
											VisionComponent,			// 16
											AnimationComponent,			// 17
											TerrainFollowComponent,		// 18
											DecalComponent,				// 19
											StealthComponent,			// 20
											FollowMouseComponent,		// 21
											ParticleEmitterComponent	// 22
											> ( );

	g_EntityManager.SetCollectionVector( collections );
	g_GameModeSelector.Initialize();

	// TODOJM Remove when singleplayer lobby implemented. HAX hax h4x
	g_GameData.SetPlayerColour( 0, Colours::KINDA_GREEN );
	g_GameData.SetPlayerColour( 1, Colours::KINDA_RED );
	g_GameData.SetPlayerColour( 2, Colours::KINDA_BLUE );
	g_GameData.SetPlayerColour( 3, Colours::KINDA_PINK );

	return true;
}
