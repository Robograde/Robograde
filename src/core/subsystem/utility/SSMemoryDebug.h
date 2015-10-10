/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSMemoryDebug SSMemoryDebug::GetInstance( )

#define MEMORY_AVG_FACTOR 100

class SSMemoryDebug : public Subsystem
{
public:
	static SSMemoryDebug& GetInstance( );

	void Startup( ) override;

	void UpdateUserLayer( const float deltaTime ) override;

	void Shutdown( ) override;

	void SetMemoryMisses( size_t count ) { m_MemoryMisses = count; }
	void SetMemoryAllocations( size_t count ) { m_MemoryAllocations = count; }

	void UpdateDistance( );

	std::string GetMemoryUsagePerFile( );

private:
	SSMemoryDebug( ) : Subsystem( "MemoryDebug" ) { }
	SSMemoryDebug( const SSMemoryDebug& );
	SSMemoryDebug& operator=( const SSMemoryDebug& );

	//GUI::TextDefinition m_MemoryUsageText;
	GUI::Window	*		m_Window;
	GUI::Text*			m_MemoryUsageText;

	size_t				m_MemoryMisses;
	size_t				m_AvgMemoryMisses;
	size_t				m_MemoryAllocations;
	size_t				m_AvgMemoryAllocations;

	size_t				m_LowestTop;
	size_t				m_HightestBottom;
};
