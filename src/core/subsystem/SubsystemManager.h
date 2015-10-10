/**************************************************
Copyright 2015 Johan Melin & Daniel Bengtsson
***************************************************/

#pragma once
#include "Subsystem.h"
#include <utility/GameTimer.h>

#define DELTA_FPS_60 0.01666667f
#define DELTA_FPS_30 (DELTA_FPS_60 * 2)
#define DELTA_TIME_STEP DELTA_FPS_30
#define DEBUG_SUBSYSTEM_MEMORY_STARTUP 1
#define g_SubsystemManager	SubsystemManager::GetInstance()

class SubsystemManager
{
public:
	static SubsystemManager&		GetInstance();	// Added so all components with same UID easily can be removed.

	void							Startup					( );
	void							Update					( const GameTimer::DeltaTimeInfo& deltaTimeInfo );
	void							Shutdown				( );
	void							AddSubsystem			( Subsystem* subsystem );

	void							StartAllUnstarted		( );
	bool							StartSubsystem			( const rString& name );
	void							StartSubsystems			( const rVector<Subsystem*>& subsystemsToStart );

	bool							StopSubsystem			( const rString& name );
	void							StopSubsystems			( const rVector<Subsystem*>& subsystemsTopStop );

	void 							Restart 				( );
	void							RestartSubsystem		( const rString& subsystemName );

	void 							PauseSubsystem			( const rString& subsystemName );
	void 							UnpauseSubsystem		( const rString& subsystemName );

	const rVector<Subsystem*>*		GetUpdateCollection		( ) const;

	void 							PrintStartupOrder 		( ) const;
	void 							PrintUpdateOrder 		( ) const;
	void 							PrintShutdownOrder 		( ) const;
	void							PrintStartedSubsystems	( ) const;
	void							PrintUnstartedSubsystems( ) const;

private:
									SubsystemManager();

	void							AddStartup				( Subsystem* subsystem );
	void							AddUpdate				( Subsystem* subsystem );
	void							AddShutdown				( Subsystem* subsystem );

	void 							PrintOrder 				( const rVector<Subsystem*>& orderToPrint ) const;

	rVector<Subsystem*>				m_StartupOrder;			// List of subsystems ordered in startup order.
	rVector<Subsystem*>				m_UpdateOrder;			// List of subsystems ordered in update order.
	rVector<Subsystem*>				m_ShutdownOrder;		// List of subsystems ordered in shutdown order.

	rMap<rString, Subsystem*>		m_SubsystemMap;			// Maps subsystem names to subsystem pointers
};
