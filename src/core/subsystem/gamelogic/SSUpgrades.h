/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include "../Subsystem.h"
#include "core/ai/Agent.h"
//#include "core/datadriven/ComponentTypes.h"
#include <messaging/Subscriber.h>

#define UNIT_DEFAULT_VALUE	250
#define UNIT_UPGRADE_GROWTH	1.2f

#define g_SSUpgrades SSUpgrades::GetInstance()

class SSUpgrades : public Subsystem, public Subscriber
{
public:
	static SSUpgrades&		GetInstance( );

	void 					Startup( ) override;
	void 					Shutdown( ) override;

	void					UpdateSimLayer( const float deltaTime ) override;

	int						GetSquadValue	( int playerID, int squadID );
	int						GetUnitValue	( Entity unitID );
	int						GetUpgradeCost  ( int upgradeID, int playerID, int squadID );

	UpgradeComponent*		GetUnitUpgrade	( Entity unitID, MODULE_SLOT slot );

	void					GiveSquadUpgrade( int playerID, int squadID, int upgradeID );
	void					GiveUnitUpgrade	( Entity unitID, int upgradeID );

	UpgradeData&			GetUpgrade	  ( int upgradeID );
	MODULE_SLOT				GetUpgradeSlot( int upgradeID );
	UPGRADE_TYPE			GetUpgradeType( int upgradeID );
	rString					GetUpgradeName( int upgradeID );
	
	rVector<UpgradeData>&	GetUpgradeArray( ) { return m_UpgradeData; };

	bool					SquadHasUpgrade	( int playerID, int squadID, int upgradeID );
	bool					SquadSlotFree	( int playerID, int squadID, MODULE_SLOT slot );

private:
	/* no type */	SSUpgrades( ) : Subsystem( "Upgrades" ), Subscriber( "Upgrades" ) { }
	/* no type */	~SSUpgrades( ) { }
	/* no type */	SSUpgrades( const SSUpgrades & rhs );
	SSUpgrades&		operator= (const SSUpgrades & rhs);

	void	RegisterDebugFunctions( );

	bool	ChangeUnitUpgrade	( Entity unitID, int upgradeID );
	void	InitUpgrade			( Entity entityID, UpgradeComponent* uprgradeComponent );

	// Movement Upgrade Methods
	void	InitMovement	( Entity entityID, UpgradeComponent* upgradeComponent );
	void	UseMovement		( Entity entityID, const float deltaTime );
	void	UpdateMovement	( Entity entityID );

	// Weapon Upgrade Methods
	void	InitWeapon	( Entity entityID, UpgradeComponent* upgradeComponent );
	void	UseWeapon	( Entity entityID, const float deltaTime );
	void	SwitchWeapon( Entity entityID, const float deltaTime );
	void	OnAttacking	( Agent* target, WeaponComponent* wc, Entity entity );
	bool	AimWeapon	( Entity entityID, glm::vec3 targetPoint, const float deltaTime );

	// Utility Active Upgrade Methods
	void	InitUtilityActive	( Entity entityID, UpgradeComponent* upgradeComponent );
	void	UseUtilityActive	( Entity entityID, const float deltaTime );

	// Utility Passive Upgrade Methods
	void	InitUtilityPassive	( Entity entityID, UpgradeComponent* upgradeComponent );
	void	UseUtilityPassive	( Entity entityID, const float deltaTime );

	rVector<UpgradeData> m_UpgradeData;
};