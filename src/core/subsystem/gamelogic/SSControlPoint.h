/**************************************************
2015 Robograde Development Team
***************************************************/

#pragma once
#include "../Subsystem.h"
#include "../../utility/GlobalDefinitions.h"
#include <script/ScriptEngine.h>
#include "../../datadriven/ComponentTypes.h"
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSControlPoint SSControlPoint::GetInstance()

#define CONTROL_POINT_UNIT_SPAWN_TIMER 180.0f
#define CONTROL_POINT_UNIT_SPAWN_COUNT 6
#define CONTROL_POINT_BASE_CAPTURE_SPEED 1.0f
#define CONTROL_POINT_MAX_UNIT_CAPTURE_RANGE 3.0f
#define CONTROL_PONT_UNIT_CAPTURE_SPEED 0.1f

enum CONTROL_POINT_STATE_CHANGE
{
	CONTROL_POINT_STATE_CHANGE_CAPTURED,
	CONTROL_POINT_STATE_CHANGE_LOST,
	CONTROL_POINT_STATE_CHANGE_CONTESTED,
};

class SSControlPoint : public Subsystem
{
public:
	static SSControlPoint&	GetInstance();

	void					Startup() override;
	void					Shutdown() override;
	void					UpdateSimLayer( const float timestep ) override;

	bool					RegisterControlPoint( Entity entityID );

	const rVector<Entity>&	GetControlPoints() const;
	unsigned int			GetTotalControlPointCount() const;
	unsigned int			GetOwnedControlPointsCount( short ownerID ) const;
	unsigned int			GetUnownedControlPointsCount() const;
	
	void					Capture( short newOwnerID, Entity controlPoint );
	void					SetAllNeutral();
	void					SetAllBelongTo( short ownerID );
	void					SetAllNeutralThatBelongTo( short ownerID );

	float					GetAverageHeight(); //used by the camera to set bounds

private:
							// No external instancing allowed
							SSControlPoint() : Subsystem( "ControlPoint" ) {}
							SSControlPoint( const SSControlPoint& rhs );
							~SSControlPoint() {};
							SSControlPoint& operator=( const SSControlPoint& rhs );

	bool					CheckForCapture( short owner, Entity controlPoint );

	int						ScriptSetAllNeutral				( IScriptEngine* scriptEngine );
	int						ScriptSetAllBelongTo			( IScriptEngine* scriptEngine );
	int						ScriptSetAllNeutralThatBelongTo	( IScriptEngine* scriptEngine );
	int						ScriptSetNeutral				( IScriptEngine* scriptEngine );
	int						ScriptSetBelongTo				( IScriptEngine* scriptEngine );

	unsigned int			m_ControlPointsPerPlayer[MAX_PLAYERS];
	unsigned int			m_UnownedControlPoints = 0;
	rVector<Entity>			m_ControlPoints;
	rVector<bool>			m_ControlPointsContested;
	rVector<GUI::Text*>		m_Counters;
	const glm::vec3			m_CounterYOffset = glm::vec3( 0.0f, 20.0f, 0.0f );
};