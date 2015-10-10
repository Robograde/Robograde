/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <core/camera/Camera.h>
#include "GraphicsInterface.h"
#include "RenderJobManager.h"
#include "Terrain.h"
#include "DepthBuffer.h"
#include "BasicRenderProgram.h"
#include "ShadowMap.h"
#include "GBuffer.h"
#include "DeferredGeometryRenderProgram.h"
#include "AnimatedRenderProgram.h"
#include "DeferredLightProgram.h"
#include "DefferedDecalProgram.h"
#include "BlurProgram.h"
#include "LineRenderProgram.h"

#define g_GFXEngine GraphicsEngine::GetInstance()
#define g_GFXTerrain GraphicsEngine::GetInstance().GetTerrain()

#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

namespace gfx
{
    class GraphicsEngine : public gfx::IGraphics
	{
	public:
		GFX_API ~GraphicsEngine();
		GFX_API static GraphicsEngine& GetInstance();
		GFX_API virtual void InitializeWindow( int width, int height, int msaa, bool fullscreen, bool vsync );
		GFX_API void ReinitializeWindow( int width, int height, int msaa, bool fullscreen, bool vsync );
		GFX_API virtual void Swap();
		GFX_API virtual void Draw();
		GFX_API virtual SDL_Window* GetWindow();
		GFX_API Terrain& GetTerrain();
		GFX_API	void SetFogOfWarInfo( const GLuint& textureHandle, const glm::vec2& worldSize );
		GFX_API RenderJobManager* GetRenderJobManager();
		GFX_API GLuint GetDepthBuffer() const;
		GFX_API const ShadowMap* GetShadowMap() const;
		GFX_API void SetShadowMap( ShadowMap* shadowmap ) { m_ShadowMap = shadowmap;};
    private:
        GraphicsEngine();
		void DrawDepth(GLenum cullmode, bool terrain);
		void DrawGeometry();
		void DrawLight();
		void DrawPostFX();

		int						m_WindowWidth;
		int						m_WindowHeight;
		int						m_MSAA;
		bool					m_Fullscreen;
		bool					m_Vsync;
		SDL_Window*				m_Window;
        SDL_GLContext			m_Context;
        GLuint					m_IndirectBuffer;
        RenderJobManager*		m_RenderJobs;
		ShaderProgramHandle		m_DefaultProgram;
		ShaderProgramHandle		m_PrePassProgram;
		ShaderProgramHandle		m_AnimatedPrePassProgram;
		DepthBuffer				m_DepthBuffer;
		bool					IsDrawIDAvailable; //Remove
		GBuffer					m_GBuffer;
		DeferredGeometryRenderProgram m_DeferredGeometryProgram;
		DeferredLightProgram	m_DeferredLightProgram;
		AnimatedRenderProgram	m_AnimatedRenderProgram;
		ShaderProgramHandle		m_PostFXShader;
		DefferedDecalProgram	m_DecalProgram;
		BlurProgram				m_BlurProgram;
		LineRenderProgram		m_LineRenderProgram;
		float					m_DownSample;
		//TODOHJ: move these somewhere else
		Terrain					m_Terrain;
		Texture					m_SkyTex;
		Texture					m_IrriadianceTex;
		GLuint					m_FogOfWarTex;
		glm::vec2				m_WorldSize;
		BasicRenderProgram		m_BasicRender;
		ShadowMap*				m_ShadowMap;
    };
}
