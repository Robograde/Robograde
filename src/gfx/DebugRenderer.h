/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once 
#include "VertexBuffer.h"
#include <glm/glm.hpp>
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
	class DebugRenderer
	{
	public:
		GFX_API DebugRenderer();
		GFX_API ~DebugRenderer();
		GFX_API void Initialize();
		GFX_API void RenderSphere(const glm::mat4& world,const glm::vec3& color);
		GFX_API void RenderDebugInfo(); //Grid and axises
		GFX_API void RenderBox(const glm::mat4& world,const glm::vec3& color);
		GFX_API void SetViewProj(const glm::mat4& view, const glm::mat4& proj);
		GFX_API void RenderRay(const glm::vec3& pos, const glm::vec3& pos2, const glm::vec3& color);
	private:
		VertexBuffer		m_VertexBuffer;
		ShaderProgramHandle m_ShaderProg;
		ShaderProgramHandle m_ShaderProgRay;
		glm::mat4			m_Proj;
		glm::mat4			m_View;
		glm::mat4			m_ViewProj;
		int					m_BoxPos;
		int					m_BoxVertexCount;
		int					m_AxisesPos;
		int					m_AxisesCount;
		int					m_SphereNumvertices;
		int					m_GridPos;
		int					m_GridCount;
	};
}