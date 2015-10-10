/**************************************************
2015 Johan Melin
***************************************************/

#include "SSDecaling.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/scalar_relational.hpp>
#include <utility/CallbackPump.h>
#include "../../component/DecalComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/SelectionComponent.h"
#include "../../component/OwnerComponent.h"
#include "../../component/HealthComponent.h"
#include "../../component/ControlPointComponent.h"
#include "../../component/ChildComponent.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../utility/PlayerData.h"
#include "../../utility/Alliances.h"
#include "../../utility/GameData.h"
#include "SSRender.h"
#include "../gfx/SSFogOfWar.h"
#include "../../EntityFactory.h"
#include "../utility/SSMail.h"
#include "../../input/GameMessages.h"

SSDecaling& SSDecaling::GetInstance( )
{
    static SSDecaling instance;
    return instance;
}

void SSDecaling::Startup( )
{
	g_SSMail.RegisterSubscriber( this );
	m_UserInterests = MessageTypes::USER_PING_MESSAGE;
}

void SSDecaling::UpdateUserLayer( const float deltaTime )
{
	for ( const Message* message : m_UserMailbox )
	{
		const UserPingMessage* pingMessage = static_cast<const UserPingMessage*>(message);
		if ( g_Alliances.IsAllied( pingMessage->PingingPlayerID, g_PlayerData.GetPlayerID( ) ) )
			g_SSDecaling.AddTimedDecal( pingMessage->WorldPosition, "ping.png", 3.5f, glm::vec4( 0.0f, 0.8f, 0.0f, 1.0f ), 5.0f, 4.0f, false, 0.75f );
	}

	UpdateTimedDecals( deltaTime );

	auto decals = GetDenseComponentArray<DecalComponent>();
	auto placements = GetDenseComponentArray<PlacementComponent>();
	auto selections = GetDenseComponentArray<SelectionComponent>();
	auto owners = GetDenseComponentArray<OwnerComponent>( );
	EntityMask decalFlag = GetDenseComponentFlag<DecalComponent>();
	EntityMask placementFlag = GetDenseComponentFlag<PlacementComponent>();
	EntityMask selectionFlag = GetDenseComponentFlag<SelectionComponent>( );
	EntityMask ownerFlag = GetDenseComponentFlag<OwnerComponent>( );
	EntityMask controlPointFlag = GetDenseComponentFlag<ControlPointComponent>();
	EntityMask childFlag = GetDenseComponentFlag<ChildComponent>();
	short thisPlayer = g_PlayerData.GetPlayerID( );
	for ( auto& entity : g_SSRender.GetGraphicsCulling( ) )
	{
		EntityMask mask = g_EntityManager.GetEntityMask( entity );
		if ( ( ( mask & decalFlag ) && ( mask & placementFlag ) ) )
		{
			DecalComponent* decal = &decals->at( entity );
			PlacementComponent* placement = &placements->at( entity );
			// TODOJM/TODOHJ Don't hardcode? // the hardcoded part is the rotation to always put the decals on the ground. since we only want terrain decals i think its fine //HJ
			decal->Decal.World = glm::translate( placement->Position ) * glm::rotate( glm::pi<float>() * 0.5f, glm::vec3( 1, 0, 0 ) ) * glm::scale( glm::vec3( decal->Scale ) );
			if ( mask & selectionFlag )
			{
				SelectionComponent* selection = &selections->at( entity );
				if ( selection->Selected || selection->MouseOvered || decal->AlwaysDraw )
				{
					if ( mask & ownerFlag && !(mask & controlPointFlag) )
					{
						OwnerComponent* owner = &owners->at( entity );
						if ( owner->OwnerID == thisPlayer ) // Me
						{
							decal->Decal.Tint = m_ColourMe;
						}
						else if ( g_Alliances.IsAllied( thisPlayer, owner->OwnerID ) ) // Allied
						{
							decal->Decal.Tint = m_ColourAllied;
						}
						else if ( owner->OwnerID == NO_OWNER_ID ) // Neutral
						{
							decal->Decal.Tint = m_ColourNeutral;
						}
						else // Enemy
						{
							decal->Decal.Tint = m_ColourEnemy;
						}
					}
					else if((mask & ownerFlag) && (mask & controlPointFlag))
					{
						OwnerComponent* owner = &owners->at( entity );
						decal->Decal.Tint = g_GameData.GetPlayerColour(owner->OwnerID);
					}
					else
					{
						decal->Decal.Tint = decal->Decal.Tint;
					}

					if ( g_SSFogOfWar.CalcVisibilityForPosition( placement->Position ) || decal->AlwaysDraw )
					{
						g_DecalManager.AddToQueue( decal->Decal );
					}
				}
			}
			else if (decal->AlwaysDraw)
			{
				g_DecalManager.AddToQueue( decal->Decal );
			}
			if(mask & childFlag)
			{
				//since its the spinning bit on top of the controlpoint that have the decal we have to check if the parent of the entity is a controlpoint
				ChildComponent* childComp = GetDenseComponent<ChildComponent>( entity );
				auto parent = childComp->Parent;
				EntityMask parentMask = g_EntityManager.GetEntityMask( parent );
				// Control point decal
				if ( parentMask & controlPointFlag )
				{
					ControlPointComponent* cpc = GetDenseComponent<ControlPointComponent>( parent );
					if ( parentMask & ownerFlag )
					{
						OwnerComponent* owner = &owners->at( parent );
						decal->Decal.Tint = g_GameData.GetPlayerColour(owner->OwnerID);
						g_DecalManager.AddToQueue( decal->Decal );
					}
					if ( cpc->IsContested && cpc->OwnerShipClock != 0 && cpc->OwnerShipClock != 1000 )
					{
						float oldScale = decal->Scale;
						glm::vec4 oldTint = decal->Decal.Tint; 
						if ( cpc->OwnerShipClock < 500 )
						{
							decal->Scale = oldScale * ( 1.0f - ( cpc->OwnerShipClock / 500.0f ) );
							decal->Decal.Tint = g_GameData.GetPlayerColour( cpc->OwnerID );
						}
						else
						{
							decal->Scale = oldScale * ( cpc->OwnerShipClock - 500 ) / 500.0f;
							decal->Decal.Tint = g_GameData.GetPlayerColour( cpc->CaptureTeam );
						}
						decal->Decal.World = glm::translate( placement->Position ) * glm::rotate( glm::pi<float>( ) * 0.5f, glm::vec3( 1, 0, 0 ) ) * glm::scale( glm::vec3( decal->Scale ) );
						g_DecalManager.AddToQueue( decal->Decal );
						decal->Scale = oldScale;
						decal->Decal.Tint = oldTint;
					}
				}
			}
		}
	}
}

void SSDecaling::Shutdown( )
{
	g_SSMail.UnregisterSubscriber( this );
	m_TimedDecals.clear( );
}

void SSDecaling::AddTimedDecal( const glm::vec3& position, const fString& texture, float timeToLive, const glm::vec4& tint, float maxScale, float minScale, bool cullWithFogOfWar, float maxAlpha, float minAlpha )
{
	glm::mat4 world = glm::translate( position ) * glm::rotate( glm::pi<float>( ) * 0.5f, glm::vec3( 1, 0, 0 ) ) * glm::scale( glm::vec3( maxScale ) );
	int textureHandle = g_DecalManager.GetTextureAtlas( )->GetHandle( texture.c_str( ) );
	assert( textureHandle >= 0 );
	m_TimedDecals.push_back( TimedDecal{
		gfx::Decal{ world, glm::mat4( 1.0f ), tint, static_cast<unsigned int>( textureHandle ), glm::uvec3( 1 ) },
		timeToLive, timeToLive, maxAlpha, minAlpha, maxScale, minScale, position, cullWithFogOfWar } );
}

void SSDecaling::UpdateTimedDecals( const float deltaTime )
{
	for ( unsigned int i = 0; i < m_TimedDecals.size( ); ++i )
	{
		TimedDecal& decal = m_TimedDecals.at( i );
		decal.TimeToLive -= deltaTime;
		if ( decal.TimeToLive <= 0.0f )
		{
			decal = m_TimedDecals.back( );
			m_TimedDecals.pop_back( );
			--i;
		}
		else
		{
			if ( !decal.FogOfWarCull || g_SSFogOfWar.CalcVisibilityForPosition( decal.Position ) )
			{
				float dx = decal.TimeToLive / decal.TimeToLiveMax;
				decal.Decal.Tint.w = decal.MinAlpha + ( decal.MaxAlpha - decal.MinAlpha ) * dx;
				float scale = decal.MinScale + ( decal.MaxScale - decal.MinScale ) * dx;
				decal.Decal.World = glm::translate( decal.Position ) * glm::rotate( glm::pi<float>( ) * 0.5f, glm::vec3( 1, 0, 0 ) ) * glm::scale( glm::vec3( scale ) );
				g_DecalManager.AddToQueue( decal.Decal );
			}
		}
	}
}
