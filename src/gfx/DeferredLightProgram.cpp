/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "DeferredLightProgram.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include "RenderJob.h"
#include "LightEngine.h"
#include "BufferManager.h"
#include "DecalManager.h"
#include <profiler/AutoGPUProfiler.h>
gfx::DeferredLightProgram::DeferredLightProgram()
{

}

gfx::DeferredLightProgram::~DeferredLightProgram()
{

}

void gfx::DeferredLightProgram::Init(RenderJobManager* jobManager)
{
	Init(jobManager, true);
}
void gfx::DeferredLightProgram::Init(RenderJobManager* jobManager, bool useFogOfWar)
{
	if (useFogOfWar)
	{
		m_ProgramHandle = g_ShaderBank.LoadShaderProgram("../../../shader/DeferredLight.glsl");
	}
	else
	{
		m_ProgramHandle = g_ShaderBank.LoadShaderProgram("../../../shader/DeferredLightNoFog.glsl");
	}
	m_RenderJobManager = jobManager;
	jobManager->BindBuffers(g_ShaderBank.GetProgramFromHandle(m_ProgramHandle));
	g_LightEngine.BindBuffer(m_ProgramHandle);
}

void gfx::DeferredLightProgram::Draw(DrawData* data)
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ProgramHandle);
	prog->Apply();
	glm::vec2 ss = ((InputParameters*)data->ExtraData)->ScreenSize;
	m_WorkGroupSizeX = GLuint((ss.x + m_WORK_GROUP_SIZE - 1) / float(m_WORK_GROUP_SIZE));
	m_WorkGroupSizeY = GLuint((ss.y + m_WORK_GROUP_SIZE - 1) / float(m_WORK_GROUP_SIZE));
	prog->SetUniformVec2("gScreenSize", ss);
	prog->SetUniformInt("numDLights", g_LightEngine.GetDirLightCount());
	//prog->SetUniformInt("numPLights", g_LightEngine.GetPointLightCount());
	prog->SetUniformUInt("gTotalLightCount", g_LightEngine.GetPointLightCount());
	prog->SetUniformInt("numLLights", g_LightEngine.GetLineLightCount());
	prog->SetUniformMat4("gProj", m_RenderJobManager->GetProj());
	prog->SetUniformMat4("gView", m_RenderJobManager->GetView());
	glm::mat4 invViewProj =  glm::inverse(m_RenderJobManager->GetProj() * m_RenderJobManager->GetView());
	prog->SetUniformMat4("gInvProjView", invViewProj);
	prog->SetUniformVec4("gCamPos", m_RenderJobManager->GetCamPos());
	prog->SetUniformMat4("gShadowMat",((InputParameters*)data->ExtraData)->ShadowMat );
	
	//input buffers
	GBuffer* gbuffer = ((InputParameters*)data->ExtraData)->gbuffers;
	prog->SetUniformTextureHandle("gAlbedoBuffer", gbuffer->GetTexture(GBUFFER_TEX::ALBEDO24),1);
	prog->SetUniformTextureHandle("gNormalBuffer", gbuffer->GetTexture(GBUFFER_TEX::NORMAL48),2);
	prog->SetUniformTextureHandle("gRoughMetalBuffer", gbuffer->GetTexture(GBUFFER_TEX::ROUGHNESS8_METAL8),3);
	prog->SetUniformTextureHandle("gDepthBuffer", gbuffer->GetTexture(GBUFFER_TEX::DEPTH_SENCIL32),4);
	prog->SetUniformTextureHandle("gMaterialBuffer", gbuffer->GetTexture(GBUFFER_TEX::MATID8),5);
	prog->SetUniformVec2("gWorldSize", ((InputParameters*)data->ExtraData)->WorldSize);
	prog->SetUniformTextureHandle("gFogTex", ((InputParameters*)data->ExtraData)->FogOfWarTex,6);
	prog->SetUniformTextureHandle("gShadowMap", ((InputParameters*)data->ExtraData)->ShadowMap,7);

	
	((InputParameters*)data->ExtraData)->SkyTex->Apply(prog->FetchUniform("g_LightCubeTex"),8);
	((InputParameters*)data->ExtraData)->IrradianceTex->Apply(prog->FetchUniform("g_IrradianceCube"),9);

	//output buffer
	glBindImageTexture( 0, gbuffer->GetTexture(GBUFFER_TEX::COLLECTIVE24), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8 );
	GPU_PROFILE( AutoGPUProfiler gpDeferredLighting( "DeferredLightingProgramCompute", true ); );
	glDispatchCompute(m_WorkGroupSizeX, m_WorkGroupSizeY, 1);
	GPU_PROFILE( gpDeferredLighting.Stop(); );
	glUseProgram(0);
}

void gfx::DeferredLightProgram::Shutdown()
{
}
