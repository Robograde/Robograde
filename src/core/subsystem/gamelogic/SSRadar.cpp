/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#include "SSRadar.h"
#include "SSUpgrades.h"
#include "../../ai/Squad.h"
#include "../../datadriven/EntityManager.h"

#include "../../datadriven/ComponentHelper.h"
#include "../../component/OwnerComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/StealthComponent.h"

#include "../../utility/PlayerData.h"
#include "../../utility/Alliances.h"
#include "../gfx/SSFogOfWar.h"

SSRadar& SSRadar::GetInstance( )
{
	static SSRadar instance;
	return instance;
}

void SSRadar::UpdateUserLayer( const float deltaTime )
{
	for ( auto& outer : m_RadarHits )
	{
		for ( auto& radarHit : outer.second )
		{
			if ( m_LastPing < RADAR_PING_TIME / 2.0f )
				radarHit.Intensity += RADAR_PING_LIGTH_HALF_INTENSITY * 2.0f * deltaTime / RADAR_PING_TIME;
			else
				radarHit.Intensity -= RADAR_PING_LIGTH_HALF_INTENSITY * 2.0f * deltaTime / RADAR_PING_TIME;
		}
	}

	for ( auto& players : m_RadarHits )
	{
		for ( auto& radarHit : players.second )
		{
			radarHit.SignalStrength -= deltaTime / RADAR_PING_TIME;
		}
	}

	m_LastPing += deltaTime;
	if ( m_LastPing > RADAR_PING_TIME )
	{
		m_LastPing -= RADAR_PING_TIME;

		struct RadarObject { bool hasRadar; glm::vec3 position; short ownerID; Entity entityID; UPGRADE_NAME radarType; bool isUnit; };
		rVector<RadarObject> radarObjects;

		// Find all objects relevant for radar
		for ( unsigned int entityID = 0; entityID < g_EntityManager.GetEntityMasks( ).size( ); ++entityID )
		{
			if ( HasComponent<StealthComponent>( entityID ) )
				GetDenseComponent<StealthComponent>( entityID )->ResetDetection( );

			if ( HasComponent<AgentComponent>( entityID ) )
			{
				Agent* agent = GetAgentPointer( entityID );
				short ownerID = GetDenseComponent<OwnerComponent>( entityID )->OwnerID;

				bool			hasRadar = false;
				UPGRADE_NAME	radarType = (UPGRADE_NAME) ~0;
				if ( g_SSUpgrades.SquadHasUpgrade( ownerID, agent->GetSquadID( ), UTILITY_ACTIVE_UNIT_RADAR ) )
				{
					hasRadar	= true;
					radarType	= UTILITY_ACTIVE_UNIT_RADAR;
				}
				else if ( g_SSUpgrades.SquadHasUpgrade( ownerID, agent->GetSquadID( ), UTILITY_ACTIVE_RESOURCE_RADAR ) )
				{
					hasRadar	= true;
					radarType	= UTILITY_ACTIVE_RESOURCE_RADAR;
				}

				radarObjects.push_back( {
					hasRadar,
					GetDenseComponent<PlacementComponent>( entityID )->Position,
					ownerID, entityID,
					radarType, true
				} );
			}
			else if ( HasComponent<ResourceComponent>( entityID ) )
			{
				radarObjects.push_back( {
					false, GetDenseComponent<PlacementComponent>( entityID )->Position, -1, entityID, (UPGRADE_NAME) ~0, false
				} );
			}
		}

		// Empty radar list
		m_RadarHits.clear( );

		float unitRadarRange		= g_SSUpgrades.GetUpgrade( UTILITY_ACTIVE_UNIT_RADAR ).Data.Utility.Radar.Range;
		float resourceRadarRange	= g_SSUpgrades.GetUpgrade( UTILITY_ACTIVE_RESOURCE_RADAR ).Data.Utility.Radar.Range;

		// Get new radar hits
		for ( auto& pinger : radarObjects )
		{
			for ( auto& target : radarObjects )
			{
				if ( pinger.hasRadar && !g_Alliances.IsAllied( pinger.ownerID, target.ownerID ) )
				{
					// Check so that the radar can detect the target
					float pingRange = 0.0f;
					if ( pinger.hasRadar && pinger.radarType == UTILITY_ACTIVE_UNIT_RADAR )
					{
						if ( !target.isUnit )
							continue;
						pingRange = unitRadarRange;
					}
					else if ( pinger.hasRadar && pinger.radarType == UTILITY_ACTIVE_RESOURCE_RADAR )
					{
						if ( target.isUnit )
							continue;
						pingRange = resourceRadarRange;
					}

					// Avoid duplicates
					bool skip = false;
					for ( auto& it : m_RadarHits[pinger.ownerID] )
						if ( it.Position == target.position )
							skip = true;
					if ( skip )
						continue;

					// Check if in range and add target if hit
					glm::vec3 directionVector = target.position - pinger.position;
					if ( glm::length2( directionVector ) < pingRange * pingRange )
					{
						for ( auto& ally : g_Alliances.GetAllies( pinger.ownerID ) )
							m_RadarHits[ally].push_back( { target.position, 0.8f, RADAR_PING_LIGTH_HALF_INTENSITY } );

						if ( HasComponent<StealthComponent>( target.entityID ) )
							GetDenseComponent<StealthComponent>( target.entityID )->Detect( pinger.ownerID );
					}
				}
			}
		}
	}
}

void SSRadar::Render( rMap<int, InstancedObject>& renderBucket )
{
	for ( auto& radarHit : m_RadarHits[ g_PlayerData.GetPlayerID( ) ])
	{
		if ( !g_SSFogOfWar.CalcVisibilityForPosition( radarHit.Position ) )
		{
			glm::mat4 world; glm::quat orientation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
			int bucket = gfx::g_ModelBank.LoadModel( "pod1Shape.robo" );
			g_SSRender.CalculateWorldMatrix( radarHit.Position, glm::vec3( 0.5f, 0.5f, 0.5f ), orientation, world );
			renderBucket[bucket].worlds.push_back( world );
			renderBucket[bucket].colors.push_back( glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

			gfx::Light light;
			light.Color		= glm::vec4( 1.0f, 0.0f, 1.0f, 1.0f );
			light.Intensity = radarHit.Intensity;
			light.Range		= 7.5f;
			light.Position = radarHit.Position + glm::vec3( 0.0f, 1.0f, 0.0f );
			gfx::g_LightEngine.AddPointLightToQueue( light );
		}
	}
}
