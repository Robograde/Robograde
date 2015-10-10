/**************************************************
Zlib Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
namespace Audio
{
	//+===+================================================
	//----| ListenerOrientation |
	//+===+================================================
	struct ListenerOrientation
	{
		ListenerOrientation() :
			Px(0), Py(0), Pz(0),
			Vx(0), Vy(0), Vz(0),
			Fx(0), Fy(0), Fz(0),
			Ux(0), Uy(0), Uz(0)
		{
		}

		unsigned int	Id;

		///Position
		float			Px,
						Py,
						Pz;

		///Velocity
		float			Vx,
						Vy,
						Vz;

		///Forward
		float			Fx,
						Fy,
						Fz;

		///Up
		float			Ux,
						Uy,
						Uz;
	};
}