/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <GL/glew.h>
#include "Lights.h"
#include "GFXLibraryDefine.h"
namespace gfx
{
	class ShadowMap
	{
	public:
		GFX_API ShadowMap();
		GFX_API ~ShadowMap();

		GFX_API void Initialize( int size );
		void Apply();
		GFX_API void GenerateMatrixFromLight( const Light& light, float frustrumsize, float near, float far );
		const glm::mat4& GetMatrix();
		void SetMatrix(const glm::mat4& mat );
		GLuint GetTexture() const { return m_Texture; };
	private:
		GLuint m_FBO;
		GLuint m_Texture;
		int m_Size;
		glm::mat4 m_LightMatrix;
	};
}
