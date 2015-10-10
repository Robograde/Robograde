/**************************************************
2015 Viktor Kelkannen
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"
#include "utility/Randomizer.h"

struct HealthComponent
{
	float	Health;
	float	Regen;
	float	MaxHealth;
	bool	IsDead;
	bool	RenderHPBar;

	HealthComponent( ) : Health( 100.0f ), IsDead( false ), Regen( 0.2f )
	{
		MaxHealth = Health;
		RenderHPBar = true;
	}

	void OnAttacked(float _damage)
	{
		Health -= _damage;
		if (Health <= 0.0f)
		{
			IsDead = true;
		}
	}
};