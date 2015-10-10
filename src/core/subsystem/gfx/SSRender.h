/**************************************************
Copyright 2015 Robograde Development Team
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <gfx/RenderJobManager.h>
#include <gfx/DebugRenderer.h>
#include "../input/SSPicking.h"

#define g_SSRender SSRender::GetInstance()

struct InstancedObject
{
	rVector<glm::mat4> worlds;
	rVector<glm::vec4> colors;
	bool animated;
	rVector<glm::mat4> joints;
	rVector<glm::mat4> bindPose;
};
class SSRender : public Subsystem
{
public:
	static SSRender&				GetInstance				();

	void							Startup					() override;
	void							Shutdown				() override;
	void							UpdateUserLayer			( const float deltaTime ) override;

	void							RenderLine				(const glm::vec3& origin, const glm::vec3& destination, const glm::vec4& color, bool dashed, float animationProgres);
	void							CalculateWorldMatrix	( const glm::vec3& position, const glm::vec3& scale, glm::quat& outOrientation, glm::mat4& outWorld );
	const rVector<unsigned int>&	GetGraphicsCulling		( ) const;

private:
									SSRender				();

	gfx::RenderJobManager*			m_RenderJobManager;
	Picking							m_GraphicsCulling;
	rVector<int>					m_CullingGroups;
	rMap<int, InstancedObject>		m_RenderBuckets;
	gfx::DebugRenderer				m_DebugRenderer;
};
