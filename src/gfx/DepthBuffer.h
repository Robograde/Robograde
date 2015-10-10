/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once 
#include <GL/glew.h>
namespace gfx
{
	class DepthBuffer
	{
	public:
		DepthBuffer();
		~DepthBuffer();
		void	Initialize( int width, int height );
		void	Apply();
		GLuint	GetTexture() const;
	private:
		GLuint	m_DepthTexture;
		GLuint	m_FBO;
		int		m_Width;
		int		m_Height;
	};
}
