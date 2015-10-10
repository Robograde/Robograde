/**************************************************
2015 Ola Enberg & Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <script/ScriptEngine.h>
#include "../../camera/CameraRTS.h"
#include "../../camera/CameraSpline.h"
#include "../../camera/CameraFirstPerson.h"

#define g_SSCamera SSCamera::GetInstance()

#define CAMERA_SUBSYSTEM_DEFAULT_MAP_WIDTH	256.0f
#define CAMERA_SUBSYSTEM_DEFAULT_MAP_DEPTH	256.0f
#define CAMERA_SUBSYSTEM_TRANSITION_TIME	0.5f

class SSCamera : public Subsystem
{
public:
	static SSCamera&		GetInstance( );

	void					Startup					() override;
	void					UpdateUserLayer			( const float deltaTime ) override;

	void					ToggleCamera			();

	Camera*					GetActiveCamera			();
	CameraRTS*				GetRTSCamera			();
	CameraSpline*			GetSplineCamera			();

	void					UseRTSCamera			();
	void					UseFPSCamera			();

	void					SetWindowSize			( int width, int height );
	void					SetMapSize				( float x, float z );
	void					SetBlockMovement		( bool newBlockMovement );

private:
							SSCamera				( ) : Subsystem( "Camera" )	{}
							SSCamera				( const SSCamera & rhs );
							~SSCamera				( )							{}
	SSCamera&				operator =				(const SSCamera & rhs);

	glm::vec3				CalculateStartPosition	();

	int						ScriptSetPosition		( IScriptEngine* scriptEngine );
	int						ScriptLookAt			( IScriptEngine* scriptEngine );
	int						ScriptSetMoveSpeed		( IScriptEngine* scriptEngine );
	int						ScriptSetAltMoveSpeed	( IScriptEngine* scriptEngine );
	int						ScriptSetFOV			( IScriptEngine* scriptEngine );
	int						ScriptSetBoundsActive	( IScriptEngine* scriptEngine );
	int						ScriptActivateCameraRTS	( IScriptEngine* scriptEngine );
	int						ScriptActivateCameraFP	( IScriptEngine* scriptEngine );

	Camera*			 		m_Active				= nullptr;
	CameraRTS 				m_RTS;
	CameraSpline			m_Spline;
	CameraFirstPerson 		m_FirstPerson;
	unsigned int			m_WindowWidth			= 0;
	unsigned int			m_WindowHeight			= 0;
	float					m_MapSizeX				= 0.0f;
	float					m_MapSizeZ				= 0.0f;
	float					m_TransitionTimer		= 0.0f;
	glm::vec3				m_TransitionPosBase		= glm::vec3( 0.0f );
	glm::vec3				m_TransitionPosChange	= glm::vec3( 0.0f );
	glm::quat				m_TransitionRotBase		= glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
	glm::quat				m_TransitionRotChange	= glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
	bool					m_BlockMovement			= false;
	float					m_MaxZoom				= 0.0f;
	float					m_MinZoom				= 0.0f;
};