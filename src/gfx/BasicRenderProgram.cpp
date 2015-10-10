/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "BasicRenderProgram.h"
#include <profiler/AutoGPUProfiler.h>
#include "ModelBank.h"
#include "MaterialBank.h"
#include "RenderJob.h"
#include "LightEngine.h"
gfx::BasicRenderProgram::BasicRenderProgram()
{

}

gfx::BasicRenderProgram::~BasicRenderProgram()
{

}

void gfx::BasicRenderProgram::Init(RenderJobManager* jobManager)
{
	//load shader
	ShaderBitFlags flag = NONE;
	if (glewIsSupported("GL_ARB_shader_draw_parameters") == GL_TRUE)
	{
		flag = VERTEX_DRAWID;
		m_HasDrawID = true;
	}
	else
	{
		m_HasDrawID = false;
	}
	m_ProgramHandle = g_ShaderBank.LoadShaderProgramWithBitFlags("../../../shader/GeometryShader.glsl",flag);

	jobManager->BindBuffers(g_ShaderBank.GetProgramFromHandle(m_ProgramHandle));
	
	//g_LightEngine.BindBuffer(m_ProgramHandle);
	m_RenderJobManager = jobManager;
}

void gfx::BasicRenderProgram::Draw(DrawData* data)
{
	//draw
	int flag = data->ShaderFlags;
	g_ModelBank.ApplyBuffers( POS_NORMAL_TEX_TANGENT );
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ProgramHandle);
	
	prog->Apply();
	m_RenderJobManager->BindBuffers(prog);
	GLint loc = -1;
	if(flag & FRAGMENT_DIFFUSEMAP)
	{
		Texture* diffuse = g_MaterialBank.GetTextureAtlasTex(Texture_Atlas_Type::Diffuse);
		loc = prog->FetchUniform("g_DiffuseTex");
		diffuse->Apply(loc, 0);
		prog->SetUniformBool("useDiffuse",true);
	}
	else
	{
		prog->SetUniformBool("useDiffuse",false);
	}
	if(flag & FRAGMENT_NORMALMAP)
	{
		Texture* normal = g_MaterialBank.GetTextureAtlasTex(Texture_Atlas_Type::Normal);
		loc = prog->FetchUniform("g_NormalTex");
		normal->Apply(loc, 1);
		prog->SetUniformBool("useNormal",true);
	}
	else
	{
		prog->SetUniformBool("useNormal",false);
	}
	if(flag & FRAGMENT_ROUGHNESSMAP)
	{
		Texture* roughness = g_MaterialBank.GetTextureAtlasTex(Texture_Atlas_Type::Roughness);
		loc = prog->FetchUniform("g_RoughnessTex");
		roughness->Apply(loc, 2);
		prog->SetUniformBool("useRoughness",true);
	}
	else
	{
		prog->SetUniformBool("useRoughness",false);
	}
	if(flag & FRAGMENT_METALMAP)
	{
		Texture* metal = g_MaterialBank.GetTextureAtlasTex(Texture_Atlas_Type::Metal);
		loc = prog->FetchUniform("g_MetalTex");
		metal->Apply(loc, 3);
		prog->SetUniformBool("useMetal",true);
	}
	else
	{
		prog->SetUniformBool("useMetal",false);
	}
	BasicData* frameData = (BasicData*)(data->ExtraData);
	prog->SetUniformInt("numPLights", frameData->PointLightCount);
	prog->SetUniformInt("numDLights", frameData->DirLightCount);
	prog->SetUniformUInt("BatchCounts",frameData->BatchOffset);
	prog->SetUniformVec2("g_WorldSize", frameData->WorldSize);
	prog->SetUniformMat4("ShadowMat",frameData->ShadowMat);
	loc = prog->FetchUniform("g_LightCubeTex");
	frameData->SkyTex->Apply(loc, 4);
	loc = prog->FetchUniform("g_IrradianceCube");
	frameData->IrradianceTex->Apply(loc, 5);

	//fog tex
	loc = prog->FetchUniform("g_FogOfWarTex");
	glUniform1i(loc,6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, frameData->FogTex);

	//fog tex
	loc = prog->FetchUniform("g_ShadowMap");
	glUniform1i(loc,7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, frameData->ShadowTex);

	
	if (m_HasDrawID)
	{
		GPU_PROFILE( AutoGPUProfiler gpMultiDraw( "BasicRenderProgramMultiDrawElementsIndirect" ); );
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(sizeof(IndirectDrawCall) * (frameData->BatchOffset)), frameData->BatchCount, 0);
		GPU_PROFILE( gpMultiDraw.Stop(); );
	}
	else
	{
		for ( unsigned int i = 0; i < static_cast<unsigned int>(frameData->BatchCount); i++ )
		{
			prog->SetUniformUInt("drawid",i);
			GPU_PROFILE( AutoGPUProfiler gpDrawElementsIndirect( "BasicRenderProgramDrawElementsIndirect", true ); );
			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(sizeof(IndirectDrawCall) * (frameData->BatchOffset + i)));
			GPU_PROFILE( gpDrawElementsIndirect.Stop(); );
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glUseProgram(0);
}

void gfx::BasicRenderProgram::Shutdown()
{
	//shutdown
	//i has nothing to shut down
}
