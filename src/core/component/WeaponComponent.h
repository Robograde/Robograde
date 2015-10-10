/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"
#include "../component/UpgradeComponent.h"

struct WeaponComponent
{
	float					Damage = 0.0f;
	float					Range = 0.0f;
	float					CoolDownTimer = 0.0f;
	float					CoolDown = 0.0f;
	UPGRADE_NAME			Name = EMPTY;
    rString					SfxPath;
	bool					IsTurret = false;

	WeaponComponent(){}

	void LoadComponentStats(int name, const UpgradeData& upgrade)
	{
		SetStats( 
			upgrade.Data.Weapon.Damage,
			upgrade.Data.Weapon.Range,
			upgrade.Data.Weapon.Cooldown,
			upgrade.Data.Weapon.FireSFX,
			upgrade.Slot == MODULE_SLOT_TOP
			);

		Name = (UPGRADE_NAME) name;
		CoolDownTimer = 0.0f;
	}

	void SetStats(float dmg, float rng, float cd, rString sfxPath, bool turret)
	{
		Damage		= dmg;
		Range		= rng;
		CoolDown	= cd;
        SfxPath		= sfxPath;
		IsTurret	= turret;
	}
};
