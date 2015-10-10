/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader/ShaderBank.h"
namespace gfx
{
	class RenderJobManager;
	struct Line
	{
		glm::vec3	Origin;
		glm::vec3	Destination;
		glm::vec4	Color;
		bool		Dashed;
		float		AnimationProgres;
	};
	class LineRenderProgram
	{
	public:
		LineRenderProgram();
		~LineRenderProgram();

		void Init(RenderJobManager* jobManager);
		void Draw();

	private:
		ShaderProgramHandle m_ProgramHandle;
		RenderJobManager*	m_RenderJobManager;
		GLuint				m_Texture;
		GLuint				m_DashedTexture;
		float				m_AnimationCounter;
	};
};