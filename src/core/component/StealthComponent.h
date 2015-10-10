/**************************************************
2015 David Pejtersen
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <set>

#define STEALTH_ACTIVATION_COOLDOWN 3.0f

struct StealthComponent
{
	bool	CanStealth	= false;
	bool	InStealth	= false;
	float	Cooldown	= 0.0f;

	std::set<short, std::less<short>, MemoryAllocator::STLAllocatorLow<short>> DetectedByPlayers;

	void Stealth( )
	{
		if ( CanStealth && Cooldown <= 0.0f )
			InStealth = true;
	}

	void UnStealth( )
	{
		InStealth	= false;
		Cooldown	= STEALTH_ACTIVATION_COOLDOWN;
	}

	void Detect( short playerID )
	{
		DetectedByPlayers.emplace( playerID );
	}

	void ResetDetection( )
	{
		DetectedByPlayers.clear( );
	}
};