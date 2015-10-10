/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <glm/glm.hpp>
#include "../../CallbackMessage.h"
#include <gfx/DecalManager.h>
#include <messaging/Subscriber.h>
// Conveniency access function
#define g_SSDecaling SSDecaling::GetInstance()

class SSDecaling : public Subsystem, public Subscriber
{
public:
    static SSDecaling& GetInstance();
    void Startup( ) override;
    void UpdateUserLayer( const float deltaTime ) override;
    void Shutdown( ) override;
	void AddTimedDecal( const glm::vec3& position, const fString& texture,
		float timeToLive, const glm::vec4& tint, float maxScale, float minScale,
		bool cullWithFogOfWar = true, float maxAlpha = 1.0f, float minAlpha = 0.0f );

private:
    // No external instancing allowed
	SSDecaling( ) : Subsystem( "Decaling" ), Subscriber( "Decaling" ) { }
    SSDecaling ( const SSDecaling & rhs );
    ~SSDecaling ( ) {};
    SSDecaling& operator=(const SSDecaling & rhs);
	void UpdateTimedDecals( const float deltaTime );

	const glm::vec4 m_ColourMe		= glm::vec4( 0.0f, 0.6f, 0.0f, 1.0f);
	const glm::vec4 m_ColourAllied	= glm::vec4( 0.0f, 0.0f, 0.6f, 1.0f);
	const glm::vec4 m_ColourEnemy	= glm::vec4( 0.6f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 m_ColourNeutral = glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f);

	struct TimedDecal
	{
		gfx::Decal Decal;
		float TimeToLive;
		float TimeToLiveMax;
		float MaxAlpha;
		float MinAlpha;
		float MaxScale;
		float MinScale;
		glm::vec3 Position;
		bool FogOfWarCull;
	};
	rVector<TimedDecal> m_TimedDecals;
};
