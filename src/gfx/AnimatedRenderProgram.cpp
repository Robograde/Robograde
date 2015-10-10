/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#include "AnimatedRenderProgram.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include "RenderJob.h"
#include <profiler/AutoGPUProfiler.h>
#include <glm/gtc/matrix_transform.hpp>

unsigned int gfx::AnimatedRenderProgram::ANIMATION_COUNTER = 0;

gfx::AnimatedRenderProgram::AnimatedRenderProgram()
{

}

gfx::AnimatedRenderProgram::~AnimatedRenderProgram()
{
	
}

void gfx::AnimatedRenderProgram::Init( RenderJobManager* jobManager )
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
	m_ProgramHandle = g_ShaderBank.LoadShaderProgramWithBitFlags("../../../shader/AnimatedDeferredGeometry.glsl",flag);

	//jobManager->BindBuffers(g_ShaderBank.GetProgramFromHandle(m_ProgramHandle));
	
	m_RenderJobManager = jobManager;


	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_ProgramHandle );
	m_RenderJobManager->BindBuffers( prog );
}

void gfx::AnimatedRenderProgram::Draw( DrawData* data )
{
	//draw
	int flag = data->ShaderFlags;
	g_ModelBank.ApplyBuffers( POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS );
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_ProgramHandle );
	
	prog->Apply();
	GLint loc = -1;

	prog->SetUniformUInt( "animationID", ANIMATION_COUNTER );
	ANIMATION_COUNTER++;
	

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
	if(flag & FRAGMENT_GLOW)
	{
		Texture* glow = g_MaterialBank.GetTextureAtlasTex(Texture_Atlas_Type::Glow);
		loc = prog->FetchUniform("g_GlowTex");
		glow->Apply(loc, 6);
		prog->SetUniformBool("useGlow",true);
	}
	else
	{
		prog->SetUniformBool("useGlow",false);
	}

	InputParameters* frameData = (InputParameters*)(data->ExtraData);
	prog->SetUniformUInt("BatchCounts",frameData->BatchOffset);
	//frameData->SkyTex->Apply(prog->FetchUniform("g_LightCubeTex"),4);
	//frameData->IrradianceTex->Apply(prog->FetchUniform("g_IrradianceCube"), 5);
	prog->SetUniformUInt( "animationOffset", (unsigned int)frameData->SkeletonOffset );

	if (m_HasDrawID)
	{
		GPU_PROFILE( AutoGPUProfiler gpDeferredGeometry( "DeferredGeometryMultidraw", true ); );
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(sizeof(IndirectDrawCall) * (frameData->BatchOffset)), frameData->BatchCount, 0);
		GPU_PROFILE( gpDeferredGeometry.Stop(); );
	}
	else
	{
		for ( unsigned int i = 0; i < static_cast<unsigned int>(frameData->BatchCount); i++ )
		{
			prog->SetUniformUInt("drawid",i);
			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(sizeof(IndirectDrawCall) * (frameData->BatchOffset + i)));
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void gfx::AnimatedRenderProgram::Shutdown()
{
	//shutdown
	//i has nothing to shut down
}

void gfx::AnimatedRenderProgram::ResetAnimationCounter()
{
	ANIMATION_COUNTER = 0;
}