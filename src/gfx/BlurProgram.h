/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include "RenderProgram.h"
#include <glm/glm.hpp>
namespace gfx
{

	class BlurProgram
	{
	public:
		BlurProgram();
		~BlurProgram();
		
		void Init(glm::vec2 screenSize, float downsample);
		void Draw(DrawData* data);
		void Resize(glm::vec2 screenSize, float downsample);

		GLuint GetBlurredTexture(){ return m_Textures[1]; };
		struct Inparams
		{
			GLuint SrcTexture;
		};
	private:
		void CalculateWeight(float sigma);

		ShaderProgramHandle	m_Program;
		GLuint				m_FBO[2];
		GLuint				m_Textures[2];
		int					m_Width, m_Height;
		float				m_Weights[3];
	};
};