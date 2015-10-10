/**************************************************
2015 David Pejtersen
***************************************************/

#pragma once

#include <gfx/LightEngine.h>

#include <glm/glm.hpp>
#include "../Subsystem.h"
#include "../gfx/SSRender.h"

#define g_SSRadar SSRadar::GetInstance( )

#define RADAR_PING_LIGTH_HALF_INTENSITY 10.0f
#define RADAR_PING_TIME					2.0f

class SSRadar : public Subsystem
{
public:
	struct RadarHit
	{
		glm::vec3	Position;
		float		SignalStrength;
		float 		Intensity;
	};

	static SSRadar&	GetInstance( );

	void			UpdateUserLayer( const float deltaTime ) override;

	void			Render( rMap<int, InstancedObject>& renderBucket );

	rMap<int, rVector<RadarHit>>& GetRadarHits( ) { return m_RadarHits; }

private:
	/* no type */	SSRadar( ) : Subsystem( "Radar" ) { }
	/* no type */	~SSRadar( ) { }
	/* no type */	SSRadar( const SSRadar& );
	SSRadar&		operator=( const SSRadar& );

	rMap<int, rVector<RadarHit>>	m_RadarHits;
	float							m_LastPing = 0.0f;
};
