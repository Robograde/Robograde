/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSResourceFlag SSResourceFlag::GetInstance()

class SSResourceFlag : public Subsystem
{
public:
    static SSResourceFlag&	GetInstance		( );
    void					Startup			( ) override;
    void					Shutdown		( ) override;
    void					UpdateUserLayer	( const float deltaTime ) override;

private:
							// No external instancing allowed
							SSResourceFlag	( ) : Subsystem( "ResourceFlag" ) {}
							SSResourceFlag	( const SSResourceFlag & rhs );
							~SSResourceFlag	( ) {};
    SSResourceFlag&			operator=		( const SSResourceFlag & rhs );

	//void					UpdateFlag		( ResourceFlag& flag, glm::ivec2 position, glm::ivec2 iconOffset, int, Squad* squad );
	//void					HandleClicks	( Squad* squad, const glm::vec3& squadWorldPos, const ResourceFlag& flag );
	
	rVector<GUI::Sprite*>	m_Flags;
	float					m_FlagYOffset				= 5.0f;
	const rString			m_ResourceFlagWindowName	= "ResourceFlag";
	const glm::ivec2		m_FlagSize					= glm::ivec2( 42, 42 );
	const glm::vec4			m_HighlightColour			= glm::vec4( 0.2f, 0.2f, 0.2f, 0.0f );
};