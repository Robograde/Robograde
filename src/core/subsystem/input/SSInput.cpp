/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson & Johan Melin
***************************************************/

#include "SSInput.h"

#include <network/NetworkEngine.h>
#include <network/PacketPump.h>
#include <input/Input.h>
#include <input/KeyBindings.h>
#include <utility/NeatFunctions.h>
#include <messaging/GameMessages.h>
#include <gfx/GraphicsEngine.h>
#include "SSButtonInput.h"
#include "../gui/SSSquadControlGUI.h"
#include "../SubsystemManager.h"
#include "../gamelogic/SSCamera.h"
#include "../gamelogic/SSAI.h"
#include "../utility/SSMail.h"
#include "../utility/SSMemoryDebug.h"
#include "../utility/SSFrameCounter.h"
#include "../gfx/SSParticle.h"
#include "../../picking/Picking.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/GameSpeedController.h"
#include "../input/SSKeyBinding.h"
#include "../../CompileFlags.h"

SSInput& SSInput::GetInstance( )
{
	static SSInput instance;
	return instance;
}

void SSInput::Startup()
{
	Subsystem::Startup();
}

void SSInput::UpdateUserLayer( const float deltaTime )
{
	for ( auto& action : *g_SSKeyBinding.GetActionToActionIdentifier( ) )
	{
		if ( g_SSKeyBinding.ActionUpDown( action ) ) // Check if the key has been triggered // TODODB: Add support for other checks than keyUpDown
		{
			g_SSButtonInput.HandleAction( action );

		}
	}


	if ( g_Input->KeyDown( SDL_SCANCODE_LCTRL ) )
	{
		CheckCTRLCommands();
		return;
	}
	else if ( g_Input->KeyDown( SDL_SCANCODE_LSHIFT ) )
	{
		CheckShiftCommands();
		return;
	}
	else
		CheckActions();
}

void SSInput::CheckActions()
{
	for ( auto& action : *g_SSKeyBinding.GetActionToActionIdentifier( ) )
	{
		if ( g_SSKeyBinding.ActionUpDown( action ) ) // Check if the key has been triggered // TODODB: Add support for other checks than keyUpDown
		{
			//if ( g_SSButtonInput.HandleAction( action ) )
			//	int TODOIA = 0; //continue;

			switch ( action ) // Check which action was trigered by the key.
			{
				case ACTION_HEST: // Feel free to test whatever you want here. It is here for utility purposes
				{
					DEV(//g_SSMail.PushToCurrentFrame( HestMessage( 4.7f, 5 ) );
					//g_SubsystemManager.Restart();

					gfx::ParticleSystem particleSystem = g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_LASER );
					particleSystem.EmitterPosition = glm::vec3( 375, 0, 380 );
					particleSystem.Direction = glm::vec3( 25, 0, 0 );
					g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_LASER, particleSystem, false );

					/*particleSystem = g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_BULLET );
					particleSystem.EmitterPosition = glm::vec3(380, 0, 380);
					g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_BULLET, particleSystem );*/

					particleSystem = g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_SMOKE_CLOUD );
					particleSystem.EmitterPosition = glm::vec3(380, 0, 380);
					g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_SMOKE_CLOUD, particleSystem );

					/*particleSystem = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_EXPLODING);
					particleSystem.EmitterPosition = glm::vec3(420, 5, 410);
					g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_EXPLODING, particleSystem );*/
					);
					return;
				} break;

				case ACTION_PRINT_KEYS:
				{
					g_SSKeyBinding.PrintKeys();
				} break;

				case ACTION_PRINT_CLICKED_POSITION:
				{
					Picking picking;
					glm::vec3 clickedWorldPos;
					picking.CalcTerrainPosUnderMouse( *g_SSCamera.GetActiveCamera(), &clickedWorldPos );
					Logger::GetStream() << "(U) Clicked terrain position:"
						<< "[C=RED]" << " x:" << "[C=WHITE]" << clickedWorldPos.x
						<< "[C=GREEN]" << " y:" << "[C=WHITE]" << clickedWorldPos.y
						<< "[C=BLUE]" << " z:" << "[C=WHITE]" << clickedWorldPos.z << std::endl;
				} break;

				case ACTION_PRINT_HOST_LATENCIES:
				{
					Logger::GetStream() << "[C=BLUE]" << "Clients latencies to the Host are:\n";
					for ( auto& connectionPair : g_NetworkInfo.GetConnections() )
					{
						Logger::GetStream() << "Client " << connectionPair.first << ": " << connectionPair.second.Latency << "ms" << "\n"; // TODODB: Output with nicer decimals
					}
				} break;

				case ACTION_PRINT_ENTITY_MASKS:
				{
					DEV( g_EntityManager.PrintMasks() );
				} break;

				case ACTION_TOGGLE_MEMORY_INFO:
				{
					DEV( g_GUI.ToggleWindow( "MemoryWindow" ) );
				} break;

				case ACTION_TOGGLE_FRAMECOUNTER_INFO:
				{
					g_GUI.ToggleWindow( "FrameCounterWindow" );
				} break;

				case ACTION_RESET_MAX_FRAMETIME:
				{
					g_SSFrameCounter.ResetMaxFrameTime();
				} break;

				case ACTION_AI_FORMATION_SPACEINC:
				{
					//SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::FORMATION_ADD_SPACE, g_PlayerData.GetPlayerID(), SQUAD_SPACING_STEP_SIZE );
				} break;

				case ACTION_AI_FORMATION_SPACEDEC:
				{
					//SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::FORMATION_ADD_SPACE, g_PlayerData.GetPlayerID(), -SQUAD_SPACING_STEP_SIZE );
				} break;

				case ACTION_AI_FORMATION_RESET:
				{
					SendOrderInvoke( g_PlayerData.GetSelectedSquads(), AICommands::FORMATION_RESET, g_PlayerData.GetPlayerID(), 0 );
				} break;

				default:
					break;
			}

			//reset key press
			g_PlayerData.SetLastActionPressed( action );

			// Check control group selection
			for ( int i = ACTION_SELECT_CONTROL_GROUP_1; i < ACTION_SELECT_CONTROL_GROUP_0; ++i )
			{
				if ( action == i )
				{
					SelectControlGroup( i - ACTION_SELECT_CONTROL_GROUP_1 );
					break;
				}
			}

			if ( action == ACTION_SELECT_UNMOVED )
				g_Script.Perform( "GE_SelectUnmovedSquads( );" );
		}
	}
}

void SSInput::CheckCTRLCommands()
{
	// Check control group assignment
	for ( int i = SDL_SCANCODE_1; i < SDL_SCANCODE_0; ++i )
	{
		if ( g_Input->KeyUpDown( static_cast<SDL_Scancode>( i ) ) )
		{
			g_PlayerData.SetControlGroup( i - SDL_SCANCODE_1, g_PlayerData.GetSelectedSquads() );
			break;
		}
	}
}

void SSInput::CheckShiftCommands()
{
	rVector<int>& selectedSquads = g_PlayerData.GetEditableSelectedSquads();

	// Check adding/removing control group from selection
	for ( int i = SDL_SCANCODE_1; i < SDL_SCANCODE_0; ++i )
	{
		if ( g_Input->KeyUpDown( static_cast<SDL_Scancode>( i ) ) )
		{
			if ( selectedSquads.size() != 0 )
			{
				// Get the selected control group
				rVector<int> controlGroup = g_PlayerData.GetControlGroup( i - SDL_SCANCODE_1 );

				// Get the squads that exist in both lists
				rVector<int> intersection = NeatFunctions::GetVectorIntersection( selectedSquads, controlGroup );
				if ( intersection.size() == controlGroup.size() && controlGroup.size() != selectedSquads.size() ) // If we selected squads that exist in both the control group and the current selection and it wasn't every squad that is currently selected
				{
					// Remove all intersecting squads from the current selection
					for ( int i = static_cast< int >( intersection.size() ) - 1; i >= 0; --i )
						selectedSquads.erase( selectedSquads.begin() + NeatFunctions::GetIndexOfElement( intersection[i], selectedSquads ) );
				}
				else
				{
					// Add all squads that only exist in the control group to the current selection
					rVector<int> diff = NeatFunctions::GetVectorDiff( controlGroup, selectedSquads );
					selectedSquads.insert( selectedSquads.end(), diff.begin(), diff.end() );
				}
			}
			else // If no squads are currently selected we just select the whole control group
			{
				SelectControlGroup( i - SDL_SCANCODE_1 );
			}
		}
	}
}

void SSInput::SendOrder( int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand )
{
	OrderUnitsMessage message = OrderUnitsMessage( g_GameData.GetFrameCount(), missionType, teamID, squads, targetLocation, targetEntity, overridingCommand );
	g_SSMail.PushToCurrentFrame( message );
}

void SSInput::SendOrderInvoke( const rVector<int>& squads, int command, const int teamID, int genericValue )
{
	OrderInvokeMessage message = OrderInvokeMessage( g_GameData.GetFrameCount(), squads, command, teamID, genericValue );
	g_SSMail.PushToCurrentFrame( message );
}

void SSInput::SelectControlGroup( short controlGroupID )
{
	rVector<int> controlGroup = g_PlayerData.GetControlGroup( controlGroupID );
	if ( !controlGroup.empty() )
	{
		if ( g_PlayerData.GetSelectedSquads() != controlGroup )
		{
			g_PlayerData.SetSelectedSquads( controlGroup );
		}
		else // Jump camera to squad location // TODODB: Add timer between button pushes and only jump if it is fast enough
		{
			const glm::vec2& squadPosition = g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), controlGroup[0] )->GetPosition();
			g_SSCamera.GetRTSCamera()->LookAtPosition( glm::vec3( squadPosition.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( squadPosition.x, squadPosition.y ), squadPosition.y ) );
		}
	}
}
