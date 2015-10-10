/**************************************************
Copyright 2015 Mattias Wilelmsen
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <gfx/Animation.h>
#include <gfx/AnimationBank.h>
#include <gfx/DebugRenderer.h>
#include "../../component/AnimationComponent.h"


#define g_SSAnimation SSAnimation::GetInstance()

class SSAnimation : public Subsystem
{
public:
	static SSAnimation&			GetInstance();
	void						Startup() override;
	void						UpdateUserLayer( const float deltaTime ) override;
	void						UpdateSimLayer( const float timeStep ) override;
	void						DebugDrawJoints( gfx::DebugRenderer& debugRenderer );

private:
	SSAnimation();
	rVector<gfx::Animation>		m_Animations;
	void UpdateAnimation( AnimationComponent* animation, const float deltaTime );
};