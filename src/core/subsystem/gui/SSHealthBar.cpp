/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSHealthBar.h"

#include "../../datadriven/EntityManager.h"
#include "../../utility/Alliances.h"
#include "../../utility/PlayerData.h"
#include "../gamelogic/SSAI.h"
#include "../gfx/SSFogOfWar.h"
#include "../../component/ColourComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/OwnerComponent.h"
#include "../../component/ColourComponent.h"
#include "../../component/HealthComponent.h"
#include "../../component/UpgradeComponent.h"
#include "../../component/ParentComponent.h"
#include "../input/SSPicking.h"
#include "../gamelogic/SSUpgrades.h"

#include "../../datadriven/DenseComponentCollection.h"

SSHealthBar& SSHealthBar::GetInstance()
{
	static SSHealthBar instance;
	return instance;
}

void SSHealthBar::Startup()
{
	glm::ivec2 winSize = g_GUI.GetWindowSize( "RootWindow" );
	
	GUI::Window* window = g_GUI.AddWindow( m_HealthBarWindowName, GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "InGameWindow" );
	window->Open();
}

void SSHealthBar::Shutdown()
{
	g_GUI.DeleteObject( m_HealthBarWindowName );
	m_HealthBars.clear();
}

void SSHealthBar::UpdateUserLayer( const float deltaTime )
{
	int barAmount = static_cast<int>( m_HealthBars.size() );
	
	int barCount = 0;
	
	EntityMask healthFlag	= GetDenseComponentFlag<HealthComponent>( );
	EntityMask ownerFlag	= GetDenseComponentFlag<OwnerComponent>( );
	EntityMask parentFlag	= GetDenseComponentFlag<ParentComponent>( );
	
	Entity entity = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		//Unit rendering
		if ( entityMask & healthFlag )
		{
			HealthComponent* healthComp = GetDenseComponent<HealthComponent>( entity );
			PlacementComponent* placementComp = GetDenseComponent<PlacementComponent>( entity );

			if ( !healthComp->RenderHPBar )
			{
				entity++;
				continue;
			}
			
			glm::vec3 worldPos = placementComp->Position + glm::vec3( 0.0f, m_BarYOffset, 0.0f );
			if( g_SSFogOfWar.CalcVisibilityForPosition( worldPos ) )
			{
				glm::ivec2 pixelPos = g_SSPicking.WorldToScreen( worldPos );
				
				GUI::ProgressBar* healthBar;
				if( barCount > barAmount - 1 )
				{
					healthBar = g_GUI.AddProgressBar( "", GUI::Rectangle( 0, 0, m_HealthBarWidth, m_HealthBarHeight ), m_HealthBarWindowName );
					healthBar->SetBackgroundColour( glm::vec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
					m_HealthBars.push_back( healthBar );
				}
				
				healthBar = m_HealthBars[barCount];
				
				if( entityMask & ownerFlag )
				{
					OwnerComponent* ownerComp = GetDenseComponent<OwnerComponent>( entity );
					
					if( ownerComp->OwnerID == g_PlayerData.GetPlayerID() ) //This is you
					{
						healthBar->SetBarColour( m_OwnBarColour );
					}
					else if( g_Alliances.IsAllied( g_PlayerData.GetPlayerID(), ownerComp->OwnerID ) ) //Friendly :)
					{
						healthBar->SetBarColour( m_FriendlyBarColour );
					}
					else //Enemy :(
					{
						healthBar->SetBarColour( m_EnemyBarColour );
					}
				}
				else //Neutral, maybe a resource
				{
					healthBar->SetBarColour( m_NeutralBarColour );
				}
				
				healthBar->SetMaxValue( healthComp->MaxHealth );
				healthBar->SetValue( healthComp->Health );
				
				if( healthComp->Health < healthComp->MaxHealth )
					healthBar->SetVisible( true );
				else
					healthBar->SetVisible( false );
				
				healthBar->SetPosition( pixelPos.x - m_HealthBarWidth / 2, pixelPos.y );

				barAmount = static_cast<int>( m_HealthBars.size() );

				barCount++;

				if ( parentFlag & entityMask )
				{
					ParentComponent* parentComp = GetDenseComponent<ParentComponent>( entity );
					Entity child = parentComp->Children[MODULE_SLOT::MODULE_SLOT_BACK];

					if ( child != ENTITY_INVALID )
					{
						UpgradeComponent* upgradeComp = GetDenseComponent<UpgradeComponent>( child );
						if ( upgradeComp->UpgradeDataID == UPGRADE_NAME::UTILITY_PASSIVE_SHIELD )
						{
							if ( barCount > barAmount - 1 )
							{
								healthBar = g_GUI.AddProgressBar( "", GUI::Rectangle( 0, 0, m_HealthBarWidth, m_HealthBarHeight ), m_HealthBarWindowName );
								healthBar->SetBackgroundColour( glm::vec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
								m_HealthBars.push_back( healthBar );
							}

							healthBar = m_HealthBars[barCount];
							UpgradeData& data = g_SSUpgrades.GetUpgrade( upgradeComp->UpgradeDataID );
							if ( upgradeComp->Data.ShieldHealth != data.Data.Utility.Shield.MaxHealth )
								healthBar->SetVisible( true );
							else
								healthBar->SetVisible( false );

							healthBar->SetBarColour( glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );


							healthBar->SetMaxValue( data.Data.Utility.Shield.MaxHealth );
							healthBar->SetValue( upgradeComp->Data.ShieldHealth );

							healthBar->SetPosition( pixelPos.x - m_HealthBarWidth / 2, pixelPos.y - m_HealthBarHeight );

							barCount++;
						}
					}
				}
			}
		}
		entity++;
	}
	
	
	if ( barAmount > barCount + 1 )
	{
		for ( int i = barCount; i < barAmount; i++ )
		{
			m_HealthBars[i]->SetVisible( false ); //Remember to reset this :)
		}
	}
}
