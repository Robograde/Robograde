/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <glm/vec2.hpp>
#include "../../datadriven/ComponentTypes.h"

// Conveniency access function
#define g_SSCollision SSCollision::GetInstance()

class SSCollision : public Subsystem
{
public:
	static SSCollision&		GetInstance				();

	void					Startup					() override;
	void					UpdateUserLayer			( const float deltaTime ) override;
	void					Shutdown				() override;

	void					RemoveCollisionEntity	( Entity entity );
	void					SetGroupsAffectedByFog	( const rVector<int>& groupsAffectedByFog );
	void					SetFogOfWarInfo			( const rVector<rVector<bool>>& texture, const glm::ivec2& textureSize, const glm::vec2& tileSize );

private:
							// No external instancing allowed
							SSCollision				() : Subsystem( "Collision" ) {}
							SSCollision				( const SSCollision & rhs );
							~SSCollision			() {}
	SSCollision&			operator=				(const SSCollision & rhs);
};