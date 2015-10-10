/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSHashComparison.h"

#include <network/NetworkInfo.h>
#include <utility/Randomizer.h>
#include "SSHashGenerator.h"
#include "../replay/SSReplayPlayer.h"
#include "../gui/SSGUIInfo.h"
#include "../../subsystem/network/SSNetworkController.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../datadriven/EntityManager.h"
#include "../../utility/GameData.h"
#include "../../utility/GameModeSelector.h"
#include "../../CompileFlags.h"

SSHashComparison& SSHashComparison::GetInstance()
{
	static SSHashComparison instance;
	return instance;
}

void SSHashComparison::UpdateSimLayer( const float timeStep )
{
	if ( !m_OutOfSync )
	{
		unsigned int	hashToCompare;
		unsigned int	randomCountToCompare;

		unsigned int currentPositionHash = g_SSHashGenerator.GetHash();
		if ( ( g_GameModeSelector.GetCurrentGameMode().IsNetworked && !g_NetworkInfo.AmIHost() ) )
		{
			hashToCompare			= g_SSNetworkController.PopHostHash();
			randomCountToCompare	= g_SSNetworkController.PopHostRandomCount();
		}
		else if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Replay )
		{
			if ( g_SSReplayPlayer.CurrentFrameRecorded() )
			{
				hashToCompare			= g_SSReplayPlayer.GetHash();
				randomCountToCompare	= g_SSReplayPlayer.GetRandomCount();
			}
			else
				return;
		}
		else
		{
			Logger::Log( "Failed to deduce what hash to compare against", "SSHashing", LogSeverity::ERROR_MSG );
			return;
		}

		if ( g_Randomizer.GetRandomizationCounter() != randomCountToCompare )
		{
			m_DesynchronizationCause = "Random count mismatch (Current = " + rToString( g_Randomizer.GetRandomizationCounter() ) + " Other = " + rToString( randomCountToCompare ) + ")";
			m_OutOfSync = true;
		}
		else if ( currentPositionHash != hashToCompare )
		{
			m_DesynchronizationCause = "Hash failure (Current = " + rToString(currentPositionHash) + " Other = " + rToString(hashToCompare) + ")" + " Differance = " + rToString( currentPositionHash < hashToCompare ? hashToCompare - currentPositionHash : currentPositionHash - hashToCompare);
			m_OutOfSync = true;
		}

		DEV(
			if ( m_OutOfSync )
			{
				g_SSGUIInfo.DisplayMessage( "OUT OF SYNC : <" );
				Logger::GetStream() << "[C=RED]HASH FAILED - OUT OF SYNC ON FRAME " << g_GameData.GetFrameCount() << "!\nDesynchronizationcause : " << m_DesynchronizationCause << '\n';
			}
		)
	}
}

void SSHashComparison::Shutdown()
{
	m_OutOfSync = false;
	m_DesynchronizationCause = DefaultDesynchronizationCause;
}