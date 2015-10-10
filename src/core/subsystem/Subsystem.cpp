/**************************************************
2015 Johan Melin
***************************************************/

#include "Subsystem.h"
#include <utility/Logger.h>

Subsystem::Subsystem( const rString& name, const rString& userFriendlyName )
	: m_Name( name ), m_UserFriendlyName( userFriendlyName != "" ? userFriendlyName : name )
{
}

const rString& Subsystem::GetName( ) const
{
	return m_Name;
}

const rString& Subsystem::GetUserFriendlyName( ) const
{
	return m_UserFriendlyName;
}

void Subsystem::StartupFromCollection()
{
	// This will call start up for the specific system (virtual funtion call)
	Startup();
	m_Started = true;
	Logger::Log( "Subsystem \"" + GetName() + "\" started.", "Subsystem", LogSeverity::DEBUG_MSG );
}

void Subsystem::UpdateUserLayer( const float deltaTime )
{
}

void Subsystem::UpdateSimLayer(const float timeStep)
{
}

void Subsystem::ShutdownFromCollection()
{	// This will call shut down for the specific system (virtual funtion call)
	Shutdown();
	Unpause();
	m_Started = false;
	Logger::Log( "Subsystem \"" + GetName() + "\" shut down.", "Subsystem", LogSeverity::DEBUG_MSG );
}

void Subsystem::SetStartOrderPriority( unsigned int priority )
{
	m_StartOrderPriority = priority;
}

void Subsystem::SetUpdateOrderPriority( unsigned int priority )
{
	m_UpdateOrderPriority = priority;
}

void Subsystem::SetShutdownOrderPriority( unsigned int priority )
{
	m_ShutdownOrderPriority = priority;
}

unsigned int Subsystem::GetStartOrderPriority() const
{
	return m_StartOrderPriority;
}

unsigned int Subsystem::GetUpdateOrderPriority() const
{
	return m_UpdateOrderPriority;
}

unsigned int Subsystem::GetShutdownOrderPriority() const
{
	return m_ShutdownOrderPriority;
}

void Subsystem::Pause()
{
	m_Paused = true;
}

void Subsystem::Unpause()
{
	m_Paused = false;
}

bool Subsystem::IsPaused() const
{
	return m_Paused;
}

bool Subsystem::IsStarted() const
{
	return m_Started;
}

void Subsystem::Startup() 
{
}

void Subsystem::Shutdown()
{
}

bool Subsystem::operator==( const Subsystem& rhs )
{
	return this->m_Name == rhs.m_Name;
}

bool Subsystem::operator!=( const Subsystem& rhs )
{
	return this->m_Name != rhs.m_Name;
}
