/**************************************************
Copyright 2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include "../datadriven/ComponentTypes.h"

static const int		CONTROL_POINT_RADIUS = 30;
static const int		CONTROL_CLOCK_CAPTURE = 1000;

struct ControlPointComponent
{
	float				OwnerShipClock = 0;
	float				UnitSpawnTimer = 0.0f;
	bool				IsContested = false;
	rMap<short, int>	LastSquadSpawnForPlayer;
	short				CaptureTeam = -1;
	short				OwnerID = -1;

	~ControlPointComponent( )
	{
		LastSquadSpawnForPlayer.clear( );
	}
};
