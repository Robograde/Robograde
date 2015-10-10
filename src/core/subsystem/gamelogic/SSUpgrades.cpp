/**************************************************
2015 David Pejtersen
***************************************************/

#include "SSUpgrades.h"

#include "../../CompileFlags.h"

#include <script/ScriptEngine.h>
#include <utility/Logger.h>
#include "../../input/GameMessages.h"

#include "../../utility/PlayerData.h"
#include "../utility/SSMail.h"

//#include "../../datadriven/DenseComponentCollection.h"
//#include "../../datadriven/EntityManager.h"
#include "../../EntityFactory.h"
#include "../../datadriven/ComponentHelper.h"
#include "../../component/ChildComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/DecalComponent.h"
#include "../../component/ParentComponent.h"
#include "../../component/UpgradeComponent.h"
#include "../../component/AgentComponent.h"
#include "../../component/ModelComponent.h"
#include "../../component/VisionComponent.h"
#include "../../component/StealthComponent.h"
#include "../../component/AnimationComponent.h"

#include <gfx/AnimationBank.h>

#include "../../subsystem/audio/SSSFXEmitter.h"
#include "../../subsystem/gamelogic/SSProjectiles.h"
#include "../../subsystem/gamelogic/SSAI.h"
#include "../../component/ControlPointComponent.h"
#include "../../ai/Terrain.h"

SSUpgrades& SSUpgrades::GetInstance()
{
	static SSUpgrades instance;
	return instance;
}

void SSUpgrades::Startup( )
{
	//m_SimInterests = MessageTypes::UPGRADE; // TODODP: Use upgrade messages directly?
	g_SSMail.RegisterSubscriber( this );

	RegisterDebugFunctions( );

	m_UpgradeData.resize( UPGRADE_COUNT );

	const rString asset = "upgrade/";

	rString damage = "";
	rString rof = "";

	// FRONT
	m_UpgradeData[WEAPON_CUTTER].Name				= "Cutter";
	m_UpgradeData[WEAPON_CUTTER].ModelName			= "CutterShape.robo";
	m_UpgradeData[WEAPON_CUTTER].Description		= "A decent short range weapon.";
	m_UpgradeData[WEAPON_CUTTER].ThumbnailPath		= asset + "Cutter.png";
	m_UpgradeData[WEAPON_CUTTER].Slot				= MODULE_SLOT_FRONT;
	m_UpgradeData[WEAPON_CUTTER].Type				= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_CUTTER].Value				= 25;
	m_UpgradeData[WEAPON_CUTTER].PointsToCompletion = 30;
	m_UpgradeData[WEAPON_CUTTER].Data				.Weapon.Damage		= 1.4f;
	m_UpgradeData[WEAPON_CUTTER].Data				.Weapon.Range		= 2.0f;
	m_UpgradeData[WEAPON_CUTTER].Data				.Weapon.Cooldown	= 0.1f;
	m_UpgradeData[WEAPON_CUTTER].Data				.Weapon.FireSFX		= "../../../asset/audio/collection/weapon/weapon_cutter.sfxc";
	m_UpgradeData[WEAPON_CUTTER].Data				.Weapon.ProjectileType = PROJECTILE_TYPE_BEAM;
	m_UpgradeData[WEAPON_CUTTER].Data				.Health.IncreasedHealth = 10.0f;

	damage = rToString( m_UpgradeData[WEAPON_CUTTER].Data.Weapon.Damage );
	damage.resize(4);

	rof = rToString( 1 / m_UpgradeData[WEAPON_CUTTER].Data.Weapon.Cooldown );
	rof.resize(4);

	m_UpgradeData[WEAPON_CUTTER].Description =
		"[C=GREEN]Damage: " + damage + "\\n" +
		"[C=GREEN]Rate of fire: " + rof + " shots/s \\n" +
		"[C=GREEN]Range: " + rToString(static_cast<int>(m_UpgradeData[WEAPON_CUTTER].Data.Weapon.Range));

	m_UpgradeData[WEAPON_PINCERS].Name					= "Pincers";
	m_UpgradeData[WEAPON_PINCERS].Description			= "A terrible short range weapon.";
	m_UpgradeData[WEAPON_PINCERS].ModelName				= "robotClawShape.robo";
	//m_UpgradeData[WEAPON_PINCERS].ThumbnailPath		= asset + "Pincers.png";
	m_UpgradeData[WEAPON_PINCERS].Slot					= MODULE_SLOT_FRONT;
	m_UpgradeData[WEAPON_PINCERS].Type					= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_PINCERS].Value					= 0;
	m_UpgradeData[WEAPON_PINCERS].PointsToCompletion	= 0;
	m_UpgradeData[WEAPON_PINCERS].Default				= true;
	m_UpgradeData[WEAPON_PINCERS].Data					.Weapon.Damage		= 2.0f;
	m_UpgradeData[WEAPON_PINCERS].Data					.Weapon.Range		= 2.0f;
	m_UpgradeData[WEAPON_PINCERS].Data					.Weapon.Cooldown	= 0.6f;
	m_UpgradeData[WEAPON_PINCERS].Data					.Weapon.FireSFX		= "../../../asset/audio/collection/weapon/weapon_pincer.sfxc";
	m_UpgradeData[WEAPON_PINCERS].Data					.Health.IncreasedHealth = 0.0f;
	m_UpgradeData[WEAPON_PINCERS].ModelName				= "robotClawShape.skelrob";
	m_UpgradeData[WEAPON_PINCERS].IsAnimated = true;
	m_UpgradeData[WEAPON_PINCERS].AnimationNames.push_back(std::pair<rString, gfx::AnimationType>("Attack.roboanim", gfx::AnimationType::Attack));
	m_UpgradeData[WEAPON_PINCERS].AnimationNames.push_back(std::pair<rString, gfx::AnimationType>("Claw_Idle.roboanim", gfx::AnimationType::Idle));

	m_UpgradeData[WEAPON_PIERCING_LASER].Name				= "Piercing Laser";
	m_UpgradeData[WEAPON_PIERCING_LASER].Description		= "Damages a unit and the ones standing behind it.";
	m_UpgradeData[WEAPON_PIERCING_LASER].ModelName			= "laserRobotShape.robo";
	m_UpgradeData[WEAPON_PIERCING_LASER].ThumbnailPath		= asset + "Piercing_Laser.png";
	m_UpgradeData[WEAPON_PIERCING_LASER].Slot				= MODULE_SLOT_FRONT;
	m_UpgradeData[WEAPON_PIERCING_LASER].Type				= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_PIERCING_LASER].Value				= 150;
	m_UpgradeData[WEAPON_PIERCING_LASER].PointsToCompletion	= 120;
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Weapon.Damage		= 4.5f;
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Weapon.Range		= 15.0f;
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Weapon.Cooldown	= 1.2f;
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Weapon.FireSFX		= "../../../asset/audio/collection/weapon/weapon_laser.sfxc";
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Weapon.ProjectileType	= PROJECTILE_TYPE_BEAM;
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Weapon.AreaOfEffect	= 1.5f;
	m_UpgradeData[WEAPON_PIERCING_LASER].Data				.Health.IncreasedHealth = 10.0f;

	damage = rToString( m_UpgradeData[WEAPON_PIERCING_LASER].Data.Weapon.Damage );
	damage.resize(4);

	rof = rToString( 1 / m_UpgradeData[WEAPON_PIERCING_LASER].Data.Weapon.Cooldown );
	rof.resize(4);

	m_UpgradeData[WEAPON_PIERCING_LASER].Description =
		"[C=GREEN]Damage: " + damage + "\\n" +
		"[C=GREEN]Rate of fire: " + rof + " shots/s \\n" +
		"[C=GREEN]Range: " + rToString( static_cast<int>( m_UpgradeData[WEAPON_PIERCING_LASER].Data.Weapon.Range ) );

	m_UpgradeData[WEAPON_PLASMA_SPEWER].Name				= "Plasma Spewer";
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Description			= "Spew a flood of plasma at your foes.";
	m_UpgradeData[WEAPON_PLASMA_SPEWER].ModelName			= "plasmaWeaponShape.robo";
	m_UpgradeData[WEAPON_PLASMA_SPEWER].ThumbnailPath		= asset + "Plasma_Spewer.png";
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Slot				= MODULE_SLOT_FRONT;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Type				= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Value				= 100;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].PointsToCompletion	= 70;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Weapon.Damage			= 2.1f;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Weapon.Range			= 3.0f;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Weapon.Cooldown		= 0.35f;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Weapon.FireSFX			= "../../../asset/audio/collection/weapon/weapon_plasma.sfxc";
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Weapon.ProjectileType	= PROJECTILE_TYPE_CONE;
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Weapon.AreaOfEffect	= 6.0f; // Width of cone
	m_UpgradeData[WEAPON_PLASMA_SPEWER].Data				.Health.IncreasedHealth = 10.0f;

	damage = rToString( m_UpgradeData[WEAPON_PLASMA_SPEWER].Data.Weapon.Damage );
	damage.resize(4);

	rof = rToString( 1 / m_UpgradeData[WEAPON_PLASMA_SPEWER].Data.Weapon.Cooldown );
	rof.resize(4);

	m_UpgradeData[WEAPON_PLASMA_SPEWER].Description =
		"[C=GREEN]Damage: " + damage + "\\n" +
		"[C=GREEN]Rate of fire: " + rof + " shots/s \\n" +
		"[C=GREEN]Range: " + rToString( static_cast<int>( m_UpgradeData[WEAPON_PLASMA_SPEWER].Data.Weapon.Range ) );

	// TOP
	m_UpgradeData[WEAPON_MACHINE_GUN].Name					= "Machine Gun";
	m_UpgradeData[WEAPON_MACHINE_GUN].Description			= "Rattatattatatata.";
	m_UpgradeData[WEAPON_MACHINE_GUN].ModelName				= "MachineGun1Shape.robo";
	m_UpgradeData[WEAPON_MACHINE_GUN].ThumbnailPath			= asset + "Machine_Gun.png";
	m_UpgradeData[WEAPON_MACHINE_GUN].Slot					= MODULE_SLOT_TOP;
	m_UpgradeData[WEAPON_MACHINE_GUN].Type					= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_MACHINE_GUN].Value					= 75;
	m_UpgradeData[WEAPON_MACHINE_GUN].PointsToCompletion	= 45;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.Damage		= 0.6f;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.Range		= 25.0f;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.Cooldown	= 0.05f;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.FireSFX		= "../../../asset/audio/collection/weapon/weapon_machinegun.sfxc";
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.ProjectileType	= PROJECTILE_TYPE_BULLET;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.ProjectileSpeed = 200.0f;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.TracerOccurence = 5;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Weapon.MissChance = 35;
	m_UpgradeData[WEAPON_MACHINE_GUN].Data					.Health.IncreasedHealth = 10.0f;

	damage = rToString( m_UpgradeData[WEAPON_MACHINE_GUN].Data.Weapon.Damage );
	damage.resize(4);

	rof = rToString( 1 / m_UpgradeData[WEAPON_MACHINE_GUN].Data.Weapon.Cooldown );
	rof.resize(4);

	m_UpgradeData[WEAPON_MACHINE_GUN].Description =
		"[C=GREEN]Damage: " + damage + "\\n" +
		"[C=GREEN]Rate of fire: " + rof + " shots/s \\n" +
		"[C=GREEN]Range: " + rToString( static_cast<int>( m_UpgradeData[WEAPON_MACHINE_GUN].Data.Weapon.Range ) );

	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Name					= "Artillery Cannon";
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Description			= "Damage a bunch of units at a distance.";
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].ModelName		    = "ArtilleryModelShape.robo";
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].ThumbnailPath		= asset + "Artillery_Cannon.png";
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Slot					= MODULE_SLOT_TOP;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Type					= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Value				= 150;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].PointsToCompletion	= 140;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Movement.MoveSpeedModifier	= 0.9f;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.Damage		= 12.0f;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.Range		= 80.0f;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.Cooldown	= 6.0f;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.FireSFX		= "../../../asset/audio/collection/weapon/weapon_tank.sfxc";
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.ProjectileType		= PROJECTILE_TYPE_GRENADE;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.ProjectileSpeed		= 20.0f;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Weapon.AreaOfEffect		= 6.0f;
	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data					.Health.IncreasedHealth = 10.0f;

	damage = rToString( m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data.Weapon.Damage );
	damage.resize(4);

	rof = rToString( 1 / m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data.Weapon.Cooldown );
	rof.resize(4);

	m_UpgradeData[WEAPON_ARTILLERY_CANNON].Description =
		"[C=GREEN]Damage: " + damage + "\\n" +
		"[C=GREEN]Rate of fire: " + rof + " shots/s \\n" +
		"[C=GREEN]Range: " + rToString( static_cast<int>( m_UpgradeData[WEAPON_ARTILLERY_CANNON].Data.Weapon.Range ) );
		

	m_UpgradeData[WEAPON_TANK_CANNON].Name					= "Tank Turret";
	m_UpgradeData[WEAPON_TANK_CANNON].ModelName				= "tankTurretShape.robo";
	m_UpgradeData[WEAPON_TANK_CANNON].ThumbnailPath			= asset + "Tank_Turret.png";
	m_UpgradeData[WEAPON_TANK_CANNON].Slot					= MODULE_SLOT_TOP;
	m_UpgradeData[WEAPON_TANK_CANNON].Type					= UPGRADE_TYPE_WEAPON;
	m_UpgradeData[WEAPON_TANK_CANNON].Value					= 80;
	m_UpgradeData[WEAPON_TANK_CANNON].PointsToCompletion	= 80;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.Damage			= 18.5f;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.Range			= 35.0f;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.Cooldown		= 3.0f;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.FireSFX			= "../../../asset/audio/collection/weapon/weapon_tank.sfxc";
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.ProjectileType	= PROJECTILE_TYPE_BULLET;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.ProjectileSpeed	= 200.0f;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.TracerOccurence	= 1;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Weapon.MissChance		= 5;
	m_UpgradeData[WEAPON_TANK_CANNON].Data					.Health.IncreasedHealth = 10.0f;
	
	damage = rToString( m_UpgradeData[WEAPON_TANK_CANNON].Data.Weapon.Damage );
	damage.resize(4);

	rof = rToString( 1 / m_UpgradeData[WEAPON_TANK_CANNON].Data.Weapon.Cooldown );
	rof.resize(4);

	m_UpgradeData[WEAPON_TANK_CANNON].Description =
		"[C=GREEN]Damage: " + damage + "\\n" +
		"[C=GREEN]Rate of fire: " + rof + " shots/s \\n" +
		"[C=GREEN]Range: " + rToString( static_cast<int>( m_UpgradeData[WEAPON_TANK_CANNON].Data.Weapon.Range ) );

	// SIDES
	m_UpgradeData[MOVEMENT_LEGS].Name				= "Legs";
	m_UpgradeData[MOVEMENT_LEGS].ModelName		    = "LegsShape.robo";
	//m_UpgradeData[MOVEMENT_LEGS].ThumbnailPath	= asset + "Legs.png";
	m_UpgradeData[MOVEMENT_LEGS].Slot				= MODULE_SLOT_SIDES;
	m_UpgradeData[MOVEMENT_LEGS].Type				= UPGRADE_TYPE_MOVEMENT;
	m_UpgradeData[MOVEMENT_LEGS].Value				= 0;
	m_UpgradeData[MOVEMENT_LEGS].PointsToCompletion	= 0;
	m_UpgradeData[MOVEMENT_LEGS].Default			= true;
	m_UpgradeData[MOVEMENT_LEGS].Data				.Movement.AddedMoveSpeed = 12.0f;
	m_UpgradeData[MOVEMENT_LEGS].Data				.Movement.AddedTurnSpeed = 5.0f;
	m_UpgradeData[MOVEMENT_LEGS].Data				.Health.IncreasedHealth  = 0.0f;
	m_UpgradeData[MOVEMENT_LEGS].ModelName		    = "LegsShape.skelrob";
	m_UpgradeData[MOVEMENT_LEGS].IsAnimated			= true;
	m_UpgradeData[MOVEMENT_LEGS].AnimationNames.push_back( std::pair<rString, gfx::AnimationType>( "WalkCycle.roboanim", gfx::AnimationType::Walk ) );
	m_UpgradeData[MOVEMENT_LEGS].AnimationNames.push_back( std::pair<rString, gfx::AnimationType>( "IdleCycle.roboanim", gfx::AnimationType::Idle ) );

	m_UpgradeData[MOVEMENT_TRACKS].Name					= "Tracks";
	m_UpgradeData[MOVEMENT_TRACKS].ModelName			= "trackShape.robo";
	m_UpgradeData[MOVEMENT_TRACKS].ThumbnailPath		= asset + "Tracks.png";
	m_UpgradeData[MOVEMENT_TRACKS].Slot					= MODULE_SLOT_SIDES;
	m_UpgradeData[MOVEMENT_TRACKS].Type					= UPGRADE_TYPE_MOVEMENT;
	m_UpgradeData[MOVEMENT_TRACKS].Value				= 100;
	m_UpgradeData[MOVEMENT_TRACKS].PointsToCompletion	= 90;
	m_UpgradeData[MOVEMENT_TRACKS].Data					.Movement.AddedMoveSpeed = 18.0f;
	m_UpgradeData[MOVEMENT_TRACKS].Data					.Movement.AddedTurnSpeed = 6.0f;
	m_UpgradeData[MOVEMENT_TRACKS].Data					.Health.IncreasedHealth	 = 10.0f;
	m_UpgradeData[MOVEMENT_TRACKS].Description =
		"[C=GREEN]Movement Speed: High \\n[C=GREEN]Turn Rate: Slightly higher than legs \\n";

	m_UpgradeData[MOVEMENT_FORTIFICATION].Name					= "Fortification";
	m_UpgradeData[MOVEMENT_FORTIFICATION ].ModelName			= "Fortifikation1Shape.robo";
	m_UpgradeData[MOVEMENT_FORTIFICATION].ThumbnailPath			= asset + "Fortification.png";
	m_UpgradeData[MOVEMENT_FORTIFICATION].Slot					= MODULE_SLOT_SIDES;
	m_UpgradeData[MOVEMENT_FORTIFICATION].Type					= UPGRADE_TYPE_MOVEMENT;
	m_UpgradeData[MOVEMENT_FORTIFICATION].Value					= 100;
	m_UpgradeData[MOVEMENT_FORTIFICATION].PointsToCompletion	= 55;
	m_UpgradeData[MOVEMENT_FORTIFICATION].Data					.Movement.AddedMoveSpeed = 1.0f;
	m_UpgradeData[MOVEMENT_FORTIFICATION].Data					.Movement.AddedTurnSpeed = 12.0f;
	m_UpgradeData[MOVEMENT_FORTIFICATION].Data					.Health.IncreasedHealth = 200.0f;
	m_UpgradeData[MOVEMENT_FORTIFICATION].Description = 
		"[C=RED]Movement Speed: Much slower \\n[C=GREEN]Turn Rate: Much faster \\n";

	// BACK
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Name				= "Unit Radar";
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].ModelName		    = "Radar1Shape.robo";
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].ThumbnailPath		= asset + "Radar.png";
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Slot				= MODULE_SLOT_BACK;
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Type				= UPGRADE_TYPE_UTILITY_ACTIVE;
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Value				= 50;
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].PointsToCompletion	= 100;
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Data				.Utility.Radar.Range	= 100.0f;
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Data				.Health.IncreasedHealth = 10.0f;
	m_UpgradeData[UTILITY_ACTIVE_UNIT_RADAR].Description =
		"[C=GREEN]Detects Enemy Units outside vision\\n \\n";

	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Name				= "Resource Radar";
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].ModelName		    = "ResourceRadarShape1.robo";
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].ThumbnailPath		= asset + "Resource_Radar.png";
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Slot				= MODULE_SLOT_BACK;
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Type				= UPGRADE_TYPE_UTILITY_ACTIVE;
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Value				= 50;
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].PointsToCompletion	= 100;
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Data				.Utility.Radar.Range	= 150.0f;
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Data				.Health.IncreasedHealth = 10.0f;
	m_UpgradeData[UTILITY_ACTIVE_RESOURCE_RADAR].Description =
		"[C=GREEN]Detects Resources far outside vision\\n \\n";

	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].Name				= "Improved Sensors";
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].ModelName			= "ImprovedSensor.robo";
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].ThumbnailPath		= asset + "Imroved_Sensor.png";
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].Slot				= MODULE_SLOT_BACK;
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].Type				= UPGRADE_TYPE_UTILITY_PASSIVE;
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].Value				= 50;
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].PointsToCompletion	= 100;
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].Data				.Utility.IncreasedVisionRange = 15.0f;
	m_UpgradeData[UTILITY_PASSIVE_IMPROVED_SENSORS].Description =
	"[C=GREEN]Increases vision range \\n \\n";

	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Name							= "Energy Shield";
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].ModelName						= "shieldShape.robo";
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].ThumbnailPath					= asset + "Energy_Shield.png";
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Slot							= MODULE_SLOT_BACK;
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Type							= UPGRADE_TYPE_UTILITY_PASSIVE;
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Value							= 100;
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].PointsToCompletion			= 60;
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data							.Utility.Shield.MaxHealth	= 35.0f;
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data							.Utility.Shield.Regeneration = 0.8f;
	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data							.Health.IncreasedHealth	= 10.0f;

	rString regen = rToString(m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data.Utility.Shield.Regeneration);
	regen.resize(4);

	m_UpgradeData[UTILITY_PASSIVE_SHIELD].Description =
		"[C=GREEN]Shield Health: " + rToString(static_cast<int>(m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data.Utility.Shield.MaxHealth)) + "\\n" +
		"[C=GREEN]Shield Regeneration: " + regen + "\\n";

	//m_UpgradeData[UTILITY_ACTIVE_STEALTH].Name					= "Stealth (JUNK)";
	//m_UpgradeData[UTILITY_ACTIVE_STEALTH].ModelName				= "stealthShape.robo";
	////m_UpgradeData[UTILITY_ACTIVE_STEALTH].ModelName		    = "stealth2Shape.robo";
	//m_UpgradeData[UTILITY_ACTIVE_STEALTH].Slot					= MODULE_SLOT_BACK;
	//m_UpgradeData[UTILITY_ACTIVE_STEALTH].Type					= UPGRADE_TYPE_UTILITY_ACTIVE;
	//m_UpgradeData[UTILITY_ACTIVE_STEALTH].Value					= 100;
	//m_UpgradeData[UTILITY_ACTIVE_STEALTH].PointsToCompletion	= 125;
}

void SSUpgrades::Shutdown( )
{
	m_UpgradeData.clear();
	g_SSMail.UnregisterSubscriber( this );

	Subsystem::Shutdown();
}

void SSUpgrades::UpdateSimLayer(const float deltaTime)
{
	for ( auto& message : m_SimMailbox )
	{
		const UpgradeMessage* upgradeMessage = static_cast<const UpgradeMessage*>(message);
		for ( auto& squad : upgradeMessage->Squads )
			GiveSquadUpgrade( upgradeMessage->PlayerID, squad, upgradeMessage->UpgradeType );
	}

	auto components = GetDenseComponentArray<UpgradeComponent>();
	for ( unsigned int entityID = 0; entityID < g_EntityManager.GetEntityMasks().size(); ++entityID )
	{
		if (HasComponent<UpgradeComponent>(entityID))
		{
			UpgradeComponent* upgrade = &components->at(entityID);
			//Entity parent = child->Parent;

			if ( upgrade->Changed )
				InitUpgrade( entityID, upgrade );

			switch (m_UpgradeData[upgrade->UpgradeDataID].Type)
			{
				case UPGRADE_TYPE_EMPTY:
				break;

				case UPGRADE_TYPE_MOVEMENT:
					if ( upgrade->Changed )
						InitMovement( entityID, upgrade );
					UseMovement( entityID, deltaTime );
				break;

				case UPGRADE_TYPE_WEAPON:
					if ( upgrade->Changed )
						InitWeapon( entityID, upgrade );
					UseWeapon( entityID, deltaTime );
				break;

				case UPGRADE_TYPE_UTILITY_ACTIVE:
					if ( upgrade->Changed )
						InitUtilityActive( entityID, upgrade );
					UseUtilityActive( entityID, deltaTime );
				break;

				case UPGRADE_TYPE_UTILITY_PASSIVE:
					if ( upgrade->Changed )
						InitUtilityPassive( entityID, upgrade );
					UseUtilityPassive( entityID, deltaTime );
				break;
			}

			upgrade->Changed = false;
		}
	}
}

int SSUpgrades::GetSquadValue( int playerID, int squadID )
{
	int squadValue = 0;

	for ( auto& agent : g_SSAI.GetSquadWithID( playerID, squadID )->GetAgents( ) )
		squadValue += GetUnitValue( agent->GetEntityID( ) );

	return squadValue;
}

int SSUpgrades::GetUnitValue( Entity unitID )
{
	int unitValue = UNIT_DEFAULT_VALUE;

	ParentComponent* parentComponent = GetDenseComponent<ParentComponent>( unitID );
	for ( Entity slot : parentComponent->Children )
	{
		if ( slot != ENTITY_INVALID )
		{
			unitValue += m_UpgradeData[ GetDenseComponent<UpgradeComponent>(slot)->UpgradeDataID ].Value;
		}
	}

	return unitValue;
}

int SSUpgrades::GetUpgradeCost( int upgradeID, int playerID, int squadID )
{
	float squadValue = (float) GetSquadValue( playerID, squadID );
	int squadSize = g_SSAI.GetSquadWithID( playerID, squadID )->GetSize();
	float unitValue = squadValue / squadSize;
	float newValue  = unitValue + g_SSUpgrades.GetUpgrade( upgradeID ).Value;
	float unitCount	= squadValue / newValue;
	
	return squadSize - static_cast<int>( glm::floor( unitCount ) );
}

bool SSUpgrades::SquadHasUpgrade( int playerID, int squadID, int upgradeID ) // TODODP: Simplify implementation when squads only can contain one kind of unit
{
	for ( auto& agent : g_SSAI.GetSquadWithID( playerID, squadID )->GetAgents( ) )
	{
		ParentComponent* parentComponent = GetDenseComponent<ParentComponent>( agent->GetEntityID( ) );
		for ( auto& slot : parentComponent->Children )
		{
			if ( slot != ENTITY_INVALID )
			{
				UpgradeComponent* upgradeComponent = GetDenseComponent<UpgradeComponent>( slot );
				if ( upgradeComponent->UpgradeDataID == upgradeID )
					return true;
			}
		}
	}
	return false;
}

bool SSUpgrades::SquadSlotFree( int playerID, int squadID, MODULE_SLOT slot )
{
	for ( auto& agent : g_SSAI.GetSquadWithID( playerID, squadID )->GetAgents( ) )
	{
		ParentComponent* parentComponent = GetDenseComponent<ParentComponent>( agent->GetEntityID( ) );
		if ( parentComponent->Children[slot] != ENTITY_INVALID && !m_UpgradeData[GetDenseComponent<UpgradeComponent>( parentComponent->Children[slot] )->UpgradeDataID].Default )
			return false;
	}
	return true;
}

void SSUpgrades::GiveSquadUpgrade( int playerID, int squadID, int upgradeID )
{
	for ( auto& agent : g_SSAI.GetSquadWithID( playerID, squadID )->GetAgents( ) )
		GiveUnitUpgrade( agent->GetEntityID( ), upgradeID );
}

void SSUpgrades::GiveUnitUpgrade( Entity unitID, int upgradeID )
{
	if ( ChangeUnitUpgrade( unitID, upgradeID ) )
		return;

	EntityFactory::CreateSquadUnitModule( unitID, GetDenseComponent<OwnerComponent>( unitID )->OwnerID, upgradeID );
}

UpgradeData& SSUpgrades::GetUpgrade( int upgradeID ) 
{ 
	return m_UpgradeData[upgradeID]; 
}

MODULE_SLOT	SSUpgrades::GetUpgradeSlot( int upgradeID ) 
{ 
	return m_UpgradeData[upgradeID].Slot; 
}

UPGRADE_TYPE SSUpgrades::GetUpgradeType( int upgradeID ) 
{ 
	return m_UpgradeData[upgradeID].Type; 
}

rString	SSUpgrades::GetUpgradeName( int upgradeID ) 
{
	return m_UpgradeData[upgradeID].Name; 
}

bool SSUpgrades::ChangeUnitUpgrade( Entity unitID, int upgradeID )
{
	MODULE_SLOT slot	= m_UpgradeData[upgradeID].Slot;
	UPGRADE_TYPE type	= m_UpgradeData[upgradeID].Type;

	if ( !HasComponent<ParentComponent>( unitID ) )
		return false;

	ParentComponent* parentComponent = GetDenseComponent<ParentComponent>( unitID );

	bool wasChanged = false;
	for ( auto it : parentComponent->Children )
	{
		if ( it != ENTITY_INVALID )
		{
			UpgradeComponent* upgradeComponent = GetDenseComponent<UpgradeComponent>( it );
			if ( m_UpgradeData[upgradeComponent->UpgradeDataID].Slot == slot )
			{
				upgradeComponent->Changed		= true;
				upgradeComponent->UpgradeDataID	= upgradeID;
				wasChanged = true;
				if (!m_UpgradeData[upgradeComponent->UpgradeDataID].IsAnimated)
				{
					if (HasComponent<AnimationComponent>(it))
					{
						g_EntityManager.RemoveComponent( it, GetDenseComponentTypeIndex<AnimationComponent>() );
					}
				}
			}
		}
	}

	return wasChanged;
}

UpgradeComponent* SSUpgrades::GetUnitUpgrade( Entity unitID, MODULE_SLOT slot )
{
	Entity upgradeEntity = GetDenseComponent<ParentComponent>( unitID )->Children[slot];
	if ( upgradeEntity == ENTITY_INVALID )
		return nullptr;
	return GetDenseComponent<UpgradeComponent>( upgradeEntity );
}

void SSUpgrades::InitUpgrade( Entity entityID, UpgradeComponent* upgradeComponent )
{
	//only agents need this
	if (HasComponent<AgentComponent>(GetDenseComponent<ChildComponent>(entityID)->Parent))
	{

		ModelComponent* modelComponent = GetDenseComponent<ModelComponent>(entityID);
		modelComponent->ModelHandle = gfx::g_ModelBank.LoadModel(m_UpgradeData[upgradeComponent->UpgradeDataID].ModelName.c_str());
		modelComponent->Min = gfx::g_ModelBank.FetchModel(modelComponent->ModelHandle).Min;
		modelComponent->Max = gfx::g_ModelBank.FetchModel(modelComponent->ModelHandle).Max;

		if (m_UpgradeData[upgradeComponent->UpgradeDataID].IsAnimated)
		{
			if (!HasComponent<AnimationComponent>(entityID))
			{
				g_EntityManager.AddComponent(entityID, GetDenseComponentTypeIndex<AnimationComponent>());
			}

			AnimationComponent* animationComponent = GetDenseComponent<AnimationComponent>(entityID);
			for (auto& pair : m_UpgradeData[upgradeComponent->UpgradeDataID].AnimationNames)
			{
				gfx::AnimationHandle animHandle = gfx::g_AnimationBank.LoadAnimation(pair.first.c_str());
				animationComponent->HandleMap[pair.second] = animHandle;
				animationComponent->Playing = true;
				animationComponent->CurrentAnimation = gfx::AnimationType::Idle;
				animationComponent->CurrentTime = g_Randomizer.UserRand(0, 1000) / 100.0f;
			}
		}

		// Increase unit health when upgrading
		float healthIncrease = m_UpgradeData[upgradeComponent->UpgradeDataID].Data.Health.IncreasedHealth;

		Entity				unitID = GetDenseComponent<ChildComponent>(entityID)->Parent;
		HealthComponent*	healthComponent = GetDenseComponent<HealthComponent>(unitID);

		healthComponent->Health += healthIncrease;
		healthComponent->MaxHealth += healthIncrease;

		// Increase unit vision when upgrading
		VisionComponent* visionComponent = GetDenseComponent<VisionComponent>(unitID);
		visionComponent->VisionRange += m_UpgradeData[upgradeComponent->UpgradeDataID].Data.Utility.IncreasedVisionRange;

		// Increase unit size when upgrading
		if (!m_UpgradeData[upgradeComponent->UpgradeDataID].Default)
		{
			PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>(unitID);
			DecalComponent*		decalComponent = GetDenseComponent<DecalComponent>(unitID);

			placementComponent->Scale *= UNIT_UPGRADE_GROWTH;
			decalComponent->Scale *= UNIT_UPGRADE_GROWTH;

			for (auto& submodule : GetDenseComponent<ParentComponent>(unitID)->Children)
				if (submodule != ENTITY_INVALID)
					GetDenseComponent<PlacementComponent>(submodule)->Scale = placementComponent->Scale;
		}
	}
}

#pragma region MOVEMENT
void SSUpgrades::InitMovement( Entity entityID, UpgradeComponent* upgradeComponent )
{
	UpdateMovement( entityID );
}

void SSUpgrades::UseMovement( Entity entityID, const float deltaTime )
{

}

void SSUpgrades::UpdateMovement( Entity entityID )
{
	float moveSpeed = 0.0f;
	float turnSpeed = 0.0f;
	float moveSpeedMultiplier = 1.0f;
	float turnSpeedMultiplier = 1.0f;

	Entity parentID = GetDenseComponent<ChildComponent>( entityID )->Parent;

	for ( unsigned int childID = 0; childID < g_EntityManager.GetEntityMasks( ).size( ); ++childID )
	{
		if ( HasComponent<UpgradeComponent>( childID ) )
		{
			ChildComponent* childComponent = GetDenseComponent<ChildComponent>( childID );
			if ( childComponent->Parent == parentID )
			{
				int id = GetDenseComponent<UpgradeComponent>( childID )->UpgradeDataID;
				moveSpeed			+= m_UpgradeData[id].Data.Movement.AddedMoveSpeed;
				turnSpeed			+= m_UpgradeData[id].Data.Movement.AddedTurnSpeed;
				moveSpeedMultiplier *= m_UpgradeData[id].Data.Movement.MoveSpeedModifier;
				turnSpeedMultiplier *= m_UpgradeData[id].Data.Movement.TurnSpeedModifier;
			}
		}
	}

	Agent* agent = GetAgentPointer( parentID );

	agent->SetSpeed( moveSpeed * moveSpeedMultiplier );
	agent->SetTurnRate( turnSpeed * turnSpeedMultiplier );
}
#pragma endregion

#pragma region WEAPONS
void SSUpgrades::InitWeapon( Entity entityID, UpgradeComponent* upgradeComponent )
{
	WeaponComponent* weaponComponent = GetDenseComponent<WeaponComponent>( entityID );
	weaponComponent->LoadComponentStats( upgradeComponent->UpgradeDataID, m_UpgradeData[upgradeComponent->UpgradeDataID] );

	//only for agents
	if (HasComponent<AgentComponent>(GetDenseComponent<ChildComponent>(entityID)->Parent))
		UpdateMovement( entityID );
}

void SSUpgrades::SwitchWeapon( Entity entityID, const float deltaTime )
{
// 	Agent* agent = GetAgentPointer( GetDenseComponent<ChildComponent>( entityID )->Parent );
// 	Agent* target = GetAgentPointer( agent->GetTarget( ) );
// 
// 	if ( target )
// 	{
// 		WeaponComponent* weaponComponent = agent->GetWeapon( );
// 		ParentComponent* parentComponent = GetDenseComponent<ParentComponent>( agent->GetEntityID( ) );
// 		for ( auto& submodule : parentComponent->Children )
// 		{
// 			if ( submodule != ENTITY_INVALID )
// 			{
// 				UpgradeComponent* upgradeComponent = GetDenseComponent<UpgradeComponent>( submodule );
// 				if ( m_UpgradeData[upgradeComponent->UpgradeDataID].Type == UPGRADE_TYPE_WEAPON )
// 				{
// 					WeaponComponent* otherWeapon = GetDenseComponent<WeaponComponent>( submodule );
// 					if ( weaponComponent != otherWeapon )
// 					{
// 						if ( otherWeapon->CoolDownTimer > 0.0f )
// 							otherWeapon->CoolDownTimer -= deltaTime;
// 
// 						float distance = glm::length2( GetDenseComponent<PlacementComponent>( target->GetEntityID( ) )->Position
// 														- GetDenseComponent<PlacementComponent>( agent->GetEntityID( ) )->Position );
// 
// 						if ( weaponComponent->CoolDownTimer > 0.0f && otherWeapon->CoolDownTimer <= 0.0f && distance <= (otherWeapon->Range + 2.0f) * (otherWeapon->Range + 2.0f) )
// 							agent->SetWeapon( otherWeapon );
// 
// 						break;
// 					}
// 				}
// 			}
// 		}
// 	}
}

void SSUpgrades::UseWeapon( Entity entityID, const float deltaTime )
{
	SwitchWeapon( entityID, deltaTime );

	WeaponComponent* wc = GetDenseComponent<WeaponComponent>(entityID);

	if (wc == nullptr)
		return;

	Agent* agent = GetAgentPointer(GetDenseComponent<ChildComponent>(entityID)->Parent);
	if (agent != nullptr)
	{
		Agent* targetagent = GetAgentPointer(agent->GetTarget());
		if (targetagent != nullptr && wc == agent->GetWeapon())
		{
			if (agent->IsAttacking() && agent->GetWeapon() == wc)
			{
				if (wc->IsTurret)
				{
					glm::vec3 targetPoint = GetDenseComponent<PlacementComponent>(agent->GetTarget())->Position;
					bool targetInSight = AimWeapon(entityID, targetPoint, deltaTime);

					if (targetInSight)
					{
						if (wc->CoolDownTimer <= 0)
						{
							g_SSProjectiles.CreateProjectile(entityID, agent->GetTarget());
							OnAttacking(targetagent, wc, entityID);
						}
					}
				}
				else
				{
					if (wc->CoolDownTimer <= 0)
					{
						g_SSProjectiles.CreateProjectile(entityID, agent->GetTarget());
						OnAttacking(targetagent, wc, entityID);
					}
				}
			}
		}
	}
	else
	{
		if (HasComponent<ChildComponent>(entityID))
		{
			Entity controlPoint = GetDenseComponent<ChildComponent>(entityID)->Parent;

			if (HasComponent<ControlPointComponent>(controlPoint))
			{
				OwnerComponent* oc = GetDenseComponent<OwnerComponent>(controlPoint);

				if (oc->OwnerID != NO_OWNER_ID)
				{
					PlacementComponent* pc = GetDenseComponent<PlacementComponent>(entityID);

					Terrain::EnemyZone ez = Terrain::GetInstance()->GetClosestEnemy(Terrain::GetInstance()->GetTile(pc->Position.x, pc->Position.z), CONTROL_POINT_RADIUS, oc->OwnerID, 0);

					if (ez.Closest != ENTITY_INVALID)
					{
						if (wc->CoolDownTimer <= 0)
						{
							Agent* enemyAgent = GetAgentPointer(ez.Closest);

							if (enemyAgent != nullptr)
							{
								wc->CoolDownTimer = wc->CoolDown;
								g_SSProjectiles.CreateProjectile(entityID, ez.Closest);
								OnAttacking(enemyAgent, wc, entityID);
							}
						}
						else
							wc->CoolDownTimer -= deltaTime * 2; //twice as fast cool down because it's a pain to make a new separate weapon
					}
				}
			}
		}
	}
}

bool SSUpgrades::AimWeapon(Entity entityID, glm::vec3 targetPoint, const float deltaTime)
{
	PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>(entityID);
	glm::vec3 position = placementComponent->Position;
	float turnRate = 3.0f;
	glm::vec3 lineAB = position - targetPoint;

	float lngth = sqrt(lineAB.x*lineAB.x + lineAB.z*lineAB.z);

	lineAB.x = (-lineAB.x / lngth)*turnRate*deltaTime;
	lineAB.z = (-lineAB.z / lngth)*turnRate*deltaTime;

	float angleGoal = (glm::half_pi<float>() + glm::atan(lineAB.z, -lineAB.x));
	glm::vec3 startRot = placementComponent->Orientation * glm::vec3(0.0f, 0.0f, -1.0f);
	float angleStart = (glm::half_pi<float>() + glm::atan(startRot.z, -startRot.x));
	if (angleStart < 0)
		angleStart += 2.0f * glm::pi<float>();

	float diff = std::abs(angleGoal - angleStart);

	float turnEpsilon = (turnRate*deltaTime)*0.5f;

	float speedToUse = 1.0f;

	if (diff > turnEpsilon && diff < 2.0f * glm::pi<float>() - turnEpsilon)
	{
		float angle = angleGoal - angleStart;
		if (angle < 0)
			angle += 2.0f * glm::pi<float>();

		float direction = (angle < glm::pi<float>()) ? turnRate : -turnRate;
		direction = (direction*deltaTime)*0.5f;

		placementComponent->Orientation = glm::quat(glm::cos(direction), 0, glm::sin(direction), 0) * placementComponent->Orientation;

		return false;
	}

	placementComponent->Orientation = glm::quat(glm::cos(angleGoal*0.5f), 0, glm::sin(angleGoal*0.5f), 0);

	return true;
}

void SSUpgrades::OnAttacking(Agent* target, WeaponComponent* wc, Entity entityID)
{
	target->IsUnderAttack(true);

	///Request to play SFX while firing
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(entityID);
	if (sfxEmitter)
	{
		g_SSSFXEmitter.AddEvent(sfxEmitter, SFXTriggerType::WHILE_ATTACKING);
	}

	wc->CoolDownTimer = wc->CoolDown;
}
#pragma endregion

#pragma region UTILITY ACTIVE
void SSUpgrades::InitUtilityActive( Entity entityID, UpgradeComponent* upgradeComponent )
{
	//else if ( upgradeComponent->UpgradeDataID == UTILITY_ACTIVE_STEALTH )
	//{
	//	StealthComponent* stealthComponent	= GetDenseComponent<StealthComponent>( unitID );
	//	stealthComponent->CanStealth		= true;

	//	stealthComponent->ResetDetection( );
	//	stealthComponent->UnStealth( );
	//}
}

void SSUpgrades::UseUtilityActive( Entity entityID, const float deltaTime )
{
	Entity unitID	= GetDenseComponent<ChildComponent>( entityID )->Parent;
	Agent* agent	= GetAgentPointer( unitID );

	StealthComponent* stealthComponent = GetDenseComponent<StealthComponent>( unitID );
	stealthComponent->Cooldown -= deltaTime;

	if ( agent->IsAttacking( ) )
		stealthComponent->UnStealth( );
	else
		stealthComponent->Stealth( );
}
#pragma endregion

#pragma region UTILITY PASSIVE
void SSUpgrades::InitUtilityPassive( Entity entityID, UpgradeComponent* upgradeComponent )
{
	if ( upgradeComponent->UpgradeDataID == UTILITY_PASSIVE_SHIELD )
		upgradeComponent->Data.ShieldHealth = 0.0f;
}

void SSUpgrades::UseUtilityPassive( Entity entityID, const float deltaTime )
{
	UpgradeComponent* upgradeComponent = GetDenseComponent<UpgradeComponent>( entityID );

	if ( upgradeComponent->UpgradeDataID == UTILITY_PASSIVE_SHIELD )
	{
		float&	health		= upgradeComponent->Data.ShieldHealth;
		float	healthRegen	= m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data.Utility.Shield.Regeneration;
		float	maxHealth	= m_UpgradeData[UTILITY_PASSIVE_SHIELD].Data.Utility.Shield.MaxHealth;
		
		if ( health < maxHealth )
		{
			health += healthRegen * deltaTime;
			if ( health > maxHealth )
				health = maxHealth;
		}
	}
}
#pragma endregion

void SSUpgrades::RegisterDebugFunctions( )
{
	DEV(
		g_Script.Register( "ListUpgrades", [] ( IScriptEngine* se ) -> int
		{
			Logger::GetStream( ) <<	"[C=GREEN]Available Upgrades\n";

			for ( int i = 0; i < g_SSUpgrades.GetUpgradeArray( ).size( ); ++i )
				Logger::GetStream( ) << "  [C=PURPLE]" << i << "  [C=BLUE] " << g_SSUpgrades.GetUpgradeName( i ) << "\n";

			return 0;
		} );

		g_Script.Register( "Give", [] ( IScriptEngine* se ) -> int
		{
			int upgrade = se->PopInt( );
			for ( auto& squad : g_PlayerData.GetSelectedSquads() )
			{
				for ( auto& agent : g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID( ), squad )->GetAgents( ) )
				{
					if ( 0 <= upgrade && upgrade < g_SSUpgrades.GetUpgradeArray( ).size( ) )
					{
						g_SSUpgrades.GiveUnitUpgrade( agent->GetEntityID( ), upgrade );
						Logger::GetStream( ) << "[C=BLUE]Gave a unit [C=GREEN]" << g_SSUpgrades.GetUpgradeName(upgrade) << "[C=BLUE].\n";
					}
					else
					{
						Logger::GetStream( ) << "[C=BLUE]Unknow upgrade with ID [C=RED]" << upgrade << "[C=BLUE]. Use [C=GREEN]ListUpgrades( )[C=BLUE] to see available upgrades.\n";
					}
				}
			}
			return 0;
		} );
	);
}
