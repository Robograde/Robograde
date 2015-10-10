/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"
#include "ParentComponent.h"
#include "gfx/AnimationTypes.h"

enum UPGRADE_TYPE
{
	UPGRADE_TYPE_EMPTY,
	UPGRADE_TYPE_WEAPON,
	UPGRADE_TYPE_MOVEMENT,
	UPGRADE_TYPE_UTILITY_ACTIVE,
	UPGRADE_TYPE_UTILITY_PASSIVE,
};

enum UPGRADE_NAME
{
	WEAPON_CUTTER,
	WEAPON_PINCERS,
	WEAPON_MACHINE_GUN,
	WEAPON_ARTILLERY_CANNON,
	WEAPON_PIERCING_LASER,
	WEAPON_PLASMA_SPEWER,
	WEAPON_TANK_CANNON,

	MOVEMENT_LEGS,
	MOVEMENT_TRACKS,
	MOVEMENT_FORTIFICATION,

	UTILITY_ACTIVE_UNIT_RADAR,
	UTILITY_ACTIVE_RESOURCE_RADAR,
	//UTILITY_ACTIVE_STEALTH,

	UTILITY_PASSIVE_IMPROVED_SENSORS,
	UTILITY_PASSIVE_SHIELD,

	UPGRADE_COUNT,
	
	EMPTY
};

struct MovementModifier
{
	float AddedMoveSpeed	= 0.0f; // summed
	float AddedTurnSpeed	= 0.0f; // summed
	float MoveSpeedModifier = 1.0f; // multiplied
	float TurnSpeedModifier = 1.0f; // multiplied
};

enum PROJECTILE_TYPE
{
	PROJECTILE_TYPE_GRENADE,
	PROJECTILE_TYPE_BULLET,
	PROJECTILE_TYPE_BEAM,
	PROJECTILE_TYPE_CONE,
	PROJECTILE_TYPE_NONE,
};

struct WeaponModifier
{
	float			Damage		= 0.0f;
	float			Range		= 0.0f;
	float			Cooldown	= 0.0f;
	rString			FireSFX;

	PROJECTILE_TYPE	ProjectileType	= PROJECTILE_TYPE_NONE;
	float			AreaOfEffect	= 0.0f;
	float			ProjectileSpeed = 0.0f;
	short			TracerOccurence = 0; // 1 out of every TracerOccurence becomes tracer (1 = only tracers)
	short			MissChance = 0; // Percent chance to miss shot
};

struct UtilityModifier
{
	union
	{
		struct
		{
			float Range;
		} Radar;
		struct
		{
			float MaxHealth;
			float Regeneration;
		} Shield;
	};

	float IncreasedVisionRange;

	UtilityModifier( )
	{
		Shield.MaxHealth		= 0.0f;
		Shield.Regeneration		= 0.0f;
		IncreasedVisionRange	= 0.0f;
	}
};

struct HealthModifier
{
	float IncreasedHealth = 0.0f;
};

struct UpgradeData
{
	rString			Name;
	rString			Description;
	rString			ModelName = "pod1Shape.robo";
	bool			IsAnimated = false;
	rVector<std::pair<rString, gfx::AnimationType>> AnimationNames;
	rString			ThumbnailPath;
	MODULE_SLOT		Slot;
	UPGRADE_TYPE	Type;
	int				Value;
	int				PointsToCompletion;
	bool			Default = false;

	struct
	{
		MovementModifier	Movement;
		UtilityModifier		Utility;
		WeaponModifier		Weapon;
		HealthModifier		Health;
	} Data;
};

struct UpgradeComponent
{
	int		UpgradeDataID	= -1;
	bool	Changed			= true;

	struct
	{
		union
		{
			float ShieldHealth = 0.0f;
		};
	} Data;
};