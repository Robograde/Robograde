/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <memory/Alloc.h>

class Subsystem
{
public:
							Subsystem( const rString& name, const rString& userFriendlyName = "" );
	virtual 				~Subsystem() {};

	const rString&			GetName( ) const;
	const rString&			GetUserFriendlyName( ) const;

	void 					StartupFromCollection		( );
	virtual void 			UpdateUserLayer				( const float deltaTime );
	virtual void			UpdateSimLayer				( const float timeStep );
	void 					ShutdownFromCollection		( );

	// Subsystems are started in ascending order. Default value is 100.
	void 					SetStartOrderPriority		( unsigned int priority );

	// Subsystems are updated in ascending order. Default value is 100.
	void 					SetUpdateOrderPriority		( unsigned int priority );

	// Subsystems are shut down in ascending order. Default value is 100.
	void 					SetShutdownOrderPriority	( unsigned int priority );

	unsigned int 			GetStartOrderPriority		( ) const;
	unsigned int 			GetUpdateOrderPriority		( ) const;
	unsigned int 			GetShutdownOrderPriority	( ) const;

	void 					Pause 						( );
	void 					Unpause						( );
	bool 					IsPaused					( ) const;
	bool					IsStarted					( ) const;

	bool operator==( const Subsystem& rhs );
	bool operator!=( const Subsystem& rhs );
protected:
	virtual void 			Startup						( );
	virtual void 			Shutdown					( );

private:
	unsigned int 			m_StartOrderPriority		= 100U;
	unsigned int 			m_UpdateOrderPriority		= 100U;
	unsigned int 			m_ShutdownOrderPriority		= 100U;
	rString 				m_Name;
	rString 				m_UserFriendlyName;
	bool 					m_Paused					= false;
	bool					m_Started					= false;
};
