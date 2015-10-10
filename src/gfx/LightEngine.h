/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <GL/glew.h>
#include <utility/Handle.h>
#include <utility/HandleAllocator.h>
#include "Lights.h"
#include "Shader/ShaderBank.h"

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
	#define g_LightEngine LightEngine::GetInstance()

	class LightEngine
	{
	public:
		GFX_API static LightEngine& GetInstance();
		GFX_API ~LightEngine();
		GFX_API void Initialize();
		GFX_API void AddPointLightToQueue(const Light& light);
		GFX_API void AddDirLightToQueue(const Light& light);
		GFX_API void AddLineLightToQueue(const Light& light);
		GFX_API void ClearPointLights( );
		GFX_API void ClearDirectionalLights( );
		GFX_API void ClearLineLights( );
		GFX_API void BuildBuffer();
		GFX_API void BindBuffer(ShaderProgramHandle handle);
		GFX_API int GetPointLightCount() const;
		GFX_API int GetDirLightCount() const;
		GFX_API int GetLineLightCount() const;	


	private:
		const unsigned int MAX_POINTLIGHTS = 4096;
		const unsigned int MAX_DIRLIGHTS = 4;
		const unsigned int MAX_LINELIGHTS =  128;
		const unsigned int MAX_NUM_LIGHTS = MAX_POINTLIGHTS + MAX_DIRLIGHTS + MAX_LINELIGHTS;

		const unsigned int POINTLIGHT_SIZE = sizeof(Light) * MAX_POINTLIGHTS;
		const unsigned int DIRLIGHT_SIZE =  sizeof(Light) * MAX_DIRLIGHTS;
		const unsigned int LINELIGHT_SIZE =  sizeof(Light) * MAX_LINELIGHTS;
		const unsigned int POINTLIGHT_POS = 0;
		const unsigned int DIRLIGHT_POS = POINTLIGHT_SIZE;
		const unsigned int LINELIGHT_POS = POINTLIGHT_SIZE + DIRLIGHT_SIZE;
		const unsigned int BUFFER_SIZE = POINTLIGHT_SIZE + DIRLIGHT_SIZE + LINELIGHT_SIZE;

		LightEngine();
		rString			m_Buffer;
		rVector<Light> 	m_PointLights;
		rVector<Light> 	m_LineLights;
		rVector<Light> 	m_DirectionalLights;
	};
}
