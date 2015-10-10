/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "GraphicsEngine.h"
#include "Shader/ShaderBank.h"
#include "LightEngine.h"
#include "MaterialBank.h"
#include "DecalManager.h"

#include <profiler/AutoProfiler.h>
#include <profiler/AutoGPUProfiler.h>
#include <utility/Logger.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>

gfx::GraphicsEngine::GraphicsEngine()
{
}

gfx::GraphicsEngine::~GraphicsEngine()
{
	pDelete( m_RenderJobs );
}

gfx::GraphicsEngine& gfx::GraphicsEngine::GetInstance()
{
	static GraphicsEngine m_Instance;
	return m_Instance;
}

void gfx::GraphicsEngine::InitializeWindow( int width, int height, int msaa, bool fullscreen, bool vsync, bool useFogOfWar )
{
	m_WindowWidth = width;
	m_WindowHeight = height;
	m_MSAA = msaa;
	m_Fullscreen = fullscreen;
	m_Vsync = vsync;

	if ( SDL_InitSubSystem( SDL_INIT_VIDEO ) != 0 )
	{
		Logger::Log( "Failed to initialize SDL video subsystem", "SDL", LogSeverity::ERROR_MSG );
		assert( false );
	}

	//SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
	//SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, msaa );
	//SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,true);
	m_Window = SDL_CreateWindow( "Robograde", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		m_WindowWidth, m_WindowHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
	m_Context = SDL_GL_CreateContext( m_Window );
	glewExperimental = GL_TRUE;
	glewInit();
	glEnable( GL_DEPTH_TEST );
	//glEnable( GL_FRAMEBUFFER_SRGB );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_ALWAYS, 1, 0xFF );
	glClearColor( 0.15f, 0.15f, 0.15f, 0.0f );

	glViewport( 0, 0, m_WindowWidth, m_WindowHeight );
	if ( vsync )
		SDL_GL_SetSwapInterval( 1 );
	else
		SDL_GL_SetSwapInterval( 0 );

	if ( m_Fullscreen )
		SDL_SetWindowFullscreen( m_Window, SDL_TRUE );
	else
		SDL_SetWindowFullscreen( m_Window, SDL_FALSE );

	m_GBuffer.Initialize( m_WindowWidth, m_WindowHeight );

	m_RenderJobs = pNew( RenderJobManager );
	m_RenderJobs->Initialize();

	g_LightEngine.Initialize();

	g_DecalManager.Initialize();
	m_DeferredGeometryProgram.Init( m_RenderJobs );
	m_DeferredLightProgram.Init( m_RenderJobs, useFogOfWar );
	m_DecalProgram.Init( m_RenderJobs );
	m_DownSample = 0.5f;
	m_BlurProgram.Init( glm::vec2( m_WindowWidth, m_WindowHeight ), m_DownSample );
	m_AnimatedRenderProgram.Init( m_RenderJobs );
	m_LineRenderProgram.Init(m_RenderJobs);
	ShaderBitFlags flag = NONE;
	ShaderDefineContainer container;

	if ( glewIsSupported( "GL_ARB_shader_draw_parameters" ) == GL_TRUE )
	{
		flag = VERTEX_DRAWID;
		IsDrawIDAvailable = true;
		container.AddDefineToShaderStage( "DRAWID_EXTENSION", GL_VERTEX_SHADER );
	}
	else
	{
		IsDrawIDAvailable = false;
	}
	//m_DefaultProgram = g_ShaderBank.LoadShaderProgramWithBitFlags( "../../../shader/GeometryShader.glsl", flag );
	//g_ShaderBank.SetDefaultShader( "../../../shader/GeometryShader.glsl" );
	//g_ShaderBank.SetDefaultFlags( flag );

	m_PostFXShader = g_ShaderBank.LoadShaderProgram( "../../../shader/DeferredPostFX.glsl" );

	m_PrePassProgram = g_ShaderBank.LoadShaderProgramWithDefines( "../../../shader/GeometryPrePass.glsl", container );
	m_AnimatedPrePassProgram = g_ShaderBank.LoadShaderProgramWithDefines( "../../../shader/AnimatedPrePass.glsl", container );

	m_RenderJobs->BindBuffers( g_ShaderBank.GetProgramFromHandle( m_AnimatedPrePassProgram ) );
	//m_DepthBuffer.Initialize( m_WindowWidth, m_WindowHeight );

	m_SkyTex.Init( "../../../asset/texture/cubemap/park_filtered.dds", CUBE_TEXTURE );
	m_IrriadianceTex.Init( "../../../asset/texture/cubemap/Irradiance.dds", CUBE_TEXTURE );

	//m_BasicRender.Init( m_RenderJobs );
}

void gfx::GraphicsEngine::ReinitializeWindow(int width, int height, int msaa, bool fullscreen, bool vsync, bool useFogOfWar)
{
	//Fullscreen off
	m_Fullscreen = fullscreen;
	if ( !m_Fullscreen )
	{
		SDL_SetWindowFullscreen( m_Window, SDL_FALSE );
	}

	if ( width != m_WindowWidth || height != m_WindowHeight )
	{
		m_WindowWidth = width;
		m_WindowHeight = height;

		SDL_SetWindowSize( m_Window, m_WindowWidth, m_WindowHeight );
		SDL_SetWindowPosition( m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
		glViewport( 0, 0, width, height );
		//update gbuffers
		//m_GBuffer.Resize(width,height); // Does not work on AMD
		m_GBuffer.Initialize( width, height ); // Will forcably recreate the whole frame buffer object. Just resizing it seems to bug out on AMD.
		m_BlurProgram.Init( glm::vec2( width, height ), m_DownSample ); // same here
	}

	//Fullscreen on
	if ( m_Fullscreen )
	{
		SDL_SetWindowFullscreen( m_Window, SDL_TRUE );
	}

	//set vsync
	m_Vsync = vsync;
	if ( m_Vsync )
	{
		SDL_GL_SetSwapInterval( 1 );
	}
	else
	{
		SDL_GL_SetSwapInterval( 0 );
	}
	//TODOHJ: handle msaa sometime
}

void gfx::GraphicsEngine::Draw()
{
	PROFILE( AutoProfiler pRenderJobBuild( "RenderJobBuild", Profiler::PROFILER_CATEGORY_STANDARD, true, true ) );
	m_RenderJobs->BuildBuffers();
	PROFILE( pRenderJobBuild.Stop() );
	if ( static_cast<int>(m_RenderJobs->GetDrawCallData().size()) == 0 )
	{
		m_RenderJobs->ClearLists();
		g_DecalManager.Clear();
		g_LightEngine.ClearLineLights();
		g_LightEngine.ClearPointLights();
		g_LightEngine.ClearDirectionalLights();
		return;
	}
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	g_DecalManager.Update();

	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_RenderJobs->GetCommandBuffer() );

	g_LightEngine.BuildBuffer();
	
	//update ShadowMap
	m_ShadowMap->Apply();
	DrawDepth( GL_BACK, false );
	//reset values after shadowmap
	glDisable( GL_POLYGON_OFFSET_FILL );
	glViewport( 0, 0, m_WindowWidth, m_WindowHeight );

	//update scenebuffer
	m_RenderJobs->UpdateSceneBuffer();
	DrawGeometry();
	DrawLight();
	DrawPostFX();

	//Draw Lines here as well
	m_LineRenderProgram.Draw( );

	m_RenderJobs->ClearLists();
	g_DecalManager.Clear();
	g_LightEngine.ClearLineLights();
	g_LightEngine.ClearPointLights();
	g_LightEngine.ClearDirectionalLights();
	
	glClearColor( 0.15f, 0.15f, 0.15f, 0.0f );
}

gfx::RenderJobManager* gfx::GraphicsEngine::GetRenderJobManager()
{
	return m_RenderJobs;
}

gfx::Terrain& gfx::GraphicsEngine::GetTerrain()
{
	return m_Terrain;
}

void gfx::GraphicsEngine::SetFogOfWarInfo( const GLuint& textureHandle, const glm::vec2& worldSize )
{
	m_FogOfWarTex = textureHandle;
	m_WorldSize = worldSize;
	g_GFXTerrain.SetFogOfWarInfo( textureHandle, worldSize );
}

void gfx::GraphicsEngine::Swap()
{
	SDL_GL_SwapWindow( m_Window );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

SDL_Window* gfx::GraphicsEngine::GetWindow()
{
	return m_Window;
}

void gfx::GraphicsEngine::DrawDepth( GLenum cullmode, bool terrain )
{
	glDepthFunc( GL_LEQUAL );
	glCullFace( cullmode );
	if ( terrain )
		m_Terrain.DrawDepth();

	
	// g_ModelBank.ApplyBuffers( POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS );
	unsigned int sizeCounter = 0;
	unsigned int animationCount = 0;
	unsigned int index = 0;
	for ( auto batch : m_RenderJobs->GetDrawCallData() )
	{

		ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_PrePassProgram );
		if (batch.shader & VERTEX_ANIMATED && index < m_RenderJobs->GetSkeletonOffsets().size())
		{
			//sizeCounter += batch.count;
			prog = g_ShaderBank.GetProgramFromHandle( m_AnimatedPrePassProgram );
			prog->Apply();
			prog->SetUniformUInt( "animationID", animationCount );
			animationCount++;
			prog->SetUniformUInt( "animationOffset", (unsigned int)m_RenderJobs->GetSkeletonOffsets().at( index ) );
			index++;
			g_ModelBank.ApplyBuffers( POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS );
		}
		else
		{
			prog->Apply();
			g_ModelBank.ApplyBuffers( POS_NORMAL_TEX_TANGENT );
		}

		m_RenderJobs->BindBuffers( prog );

		if ( IsDrawIDAvailable )
		{
			prog->SetUniformUInt( "BatchCounts", sizeCounter );
			GPU_PROFILE( AutoGPUProfiler gpuProfile( rString( "DrawDepthMulti" ) + (terrain ? "WithTerrain" : ""), true ); );
			glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(sizeof( IndirectDrawCall ) * (sizeCounter)), batch.count, 0 );
			GPU_PROFILE( gpuProfile.Stop(); );
		}
		else
		{
			for ( unsigned int i = 0; i < static_cast<unsigned int>(batch.count); i++ )
			{
				prog->SetUniformUInt( "did", i );
				prog->SetUniformUInt( "BatchCounts", sizeCounter );
				GPU_PROFILE( AutoGPUProfiler gpuProfile( rString( "DrawDepthElements" ) + (terrain ? "WithTerrain" : ""), true ); );
				glDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(sizeof( IndirectDrawCall ) * (sizeCounter + i)) );
				GPU_PROFILE( gpuProfile.Stop(); );
			}
		}
		sizeCounter += batch.count;
	}
	glDepthFunc( GL_LEQUAL );
	glCullFace( GL_BACK );
	glUseProgram( 0 );
}

GLuint gfx::GraphicsEngine::GetDepthBuffer() const
{
	return m_GBuffer.GetTexture( GBUFFER_TEX::DEPTH_SENCIL32 );
}

const gfx::ShadowMap* gfx::GraphicsEngine::GetShadowMap() const
{
	return m_ShadowMap;
}

void  gfx::GraphicsEngine::DrawGeometry()
{
	m_GBuffer.ApplyGeometryStage();
	m_GBuffer.ClearScreen();
	//draw terrain
	m_Terrain.DrawDeferred( m_ShadowMap->GetMatrix(), m_ShadowMap->GetTexture() );
	m_GBuffer.ApplyDecalStage();
	//draw decals on terrain
	DefferedDecalProgram::DecalData dd;
	dd.gbuffer = &m_GBuffer;
	dd.ScreenSize = vec2( m_WindowWidth, m_WindowHeight );
	DrawData data;
	data.ExtraData = &dd;
	m_DecalProgram.Draw( &data );
	//draw models
	m_GBuffer.ApplyGeometryStage();
	g_ModelBank.ApplyBuffers( POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS );
	unsigned int sizeCounter = 0;
	rVector<int> skeletonOffsets = m_RenderJobs->GetSkeletonOffsets();
	int index = 0;
	for ( auto& batch : m_RenderJobs->GetDrawCallData() )
	{
		DrawData data;
		data.ShaderFlags = batch.shader;
		if ( data.ShaderFlags & VERTEX_ANIMATED )
		{
			AnimatedRenderProgram::InputParameters drawparams;
			drawparams.BatchCount = batch.count;
			drawparams.BatchOffset = sizeCounter;
			//drawparams.IrradianceTex = &m_IrriadianceTex;
			//drawparams.SkyTex = &m_SkyTex;
			if ( skeletonOffsets.size() > index )
			{
				drawparams.SkeletonOffset = skeletonOffsets.at( index );
				index++;
			}
			data.ExtraData = &drawparams;
			m_AnimatedRenderProgram.Draw( &data );
		}
		else
		{
			DeferredGeometryRenderProgram::InputParameters drawparams;
			drawparams.BatchCount = batch.count;
			drawparams.BatchOffset = sizeCounter;
			drawparams.IrradianceTex = &m_IrriadianceTex;
			drawparams.SkyTex = &m_SkyTex;
			data.ExtraData = &drawparams;

			m_DeferredGeometryProgram.Draw( &data );
		}

		sizeCounter += batch.count;
		glUseProgram( 0 );
	}
	m_GBuffer.ApplyPostProcessStage();
	AnimatedRenderProgram::ResetAnimationCounter();
}

void gfx::GraphicsEngine::DrawLight()
{
	m_GBuffer.ApplyLightingStage();
	DrawData dd;
	DeferredLightProgram::InputParameters params;
	params.gbuffers = &m_GBuffer;
	params.FogOfWarTex = m_FogOfWarTex;
	params.WorldSize = m_WorldSize;
	params.ScreenSize = glm::vec2( m_WindowWidth, m_WindowHeight );
	params.ShadowMap = m_ShadowMap->GetTexture();
	params.ShadowMat = m_ShadowMap->GetMatrix();
	params.SkyTex = &m_SkyTex;
	params.IrradianceTex = &m_IrriadianceTex;
	dd.ExtraData = &params;
	m_DeferredLightProgram.Draw( &dd );
}

void gfx::GraphicsEngine::DrawPostFX()
{
	glViewport( 0, 0, (GLsizei)(m_WindowWidth * m_DownSample), (GLsizei)(m_WindowHeight * m_DownSample) );
	m_GBuffer.ApplyPostProcessStage();
	glDisable( GL_DEPTH_TEST );
	DrawData dd = DrawData();
	BlurProgram::Inparams inp;
	inp.SrcTexture = m_GBuffer.GetTexture( GBUFFER_TEX::GLOW24 );
	dd.ExtraData = &inp;
	m_BlurProgram.Draw( &dd );

	glViewport( 0, 0, m_WindowWidth, m_WindowHeight );
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_PostFXShader );
	prog->Apply();
	prog->SetUniformTextureHandle( "InputTex", m_GBuffer.GetTexture( GBUFFER_TEX::COLLECTIVE24 ), 0 );
	//prog->SetUniformTextureHandle("EmissiveTex", m_GBuffer.GetTexture(GBUFFER_TEX::EMISSIVE24),1);
	prog->SetUniformTextureHandle( "GlowTex", m_BlurProgram.GetBlurredTexture(), 2 );

	prog->SetUniformTextureHandle( "NormalTex", m_GBuffer.GetTexture( GBUFFER_TEX::NORMAL48 ), 3 );
	prog->SetUniformTextureHandle( "DepthTex", m_GBuffer.GetTexture( GBUFFER_TEX::DEPTH_SENCIL32 ), 4 );
	prog->SetUniformTextureHandle( "RoughMetalTex", m_GBuffer.GetTexture( GBUFFER_TEX::ROUGHNESS8_METAL8 ), 5 );
	prog->SetUniformBool( "Split", false );
	glBindVertexArray( 0 );
	GPU_PROFILE( AutoGPUProfiler gpDeferredPostFX( "DeferredPostFX", true ); );
	glDrawArrays( GL_POINTS, 0, 1 );
	GPU_PROFILE( gpDeferredPostFX.Stop(); );
	glEnable( GL_DEPTH_TEST );
	glUseProgram( 0 );
}
