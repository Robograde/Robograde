/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "ShadowMap.h"
#include <glm/gtx/transform.hpp>

#include "BufferManager.h"
#include "RenderJobManager.h"

gfx::ShadowMap::ShadowMap()
{
	m_LightMatrix = glm::mat4( 1 );
}

gfx::ShadowMap::~ShadowMap()
{

}

void gfx::ShadowMap::Initialize(int size)
{
	m_Size = size;

	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_DEPTH_COMPONENT24, m_Size, m_Size, 0 ,GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, m_Texture);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_Texture, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gfx::ShadowMap::Apply()
{
	//update scene buffer
	SceneInput si;
	si.viewProj = m_LightMatrix;
	g_BufferManager.UpdateBuffer( "SceneInputs", 0, &si, static_cast<int>(sizeof( SceneInput )));
	//bind framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glViewport(0,0,m_Size, m_Size);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);
}

void gfx::ShadowMap::GenerateMatrixFromLight( const Light& light , float frustrumsize, float near, float far)
{
	glm::mat4 view = glm::lookAt(light.Position , light.Position + light.Direction * 2.0f, glm::vec3(0,1,0));
	glm::mat4 ortho = glm::ortho( -frustrumsize, frustrumsize, -frustrumsize, frustrumsize, near, far );
	m_LightMatrix = ortho * view;
}

const glm::mat4& gfx::ShadowMap::GetMatrix()
{
	//const glm::mat4 bias = glm::mat4(	glm::vec4(0.5f,0.0f,0.0f,0.0f),
	//									glm::vec4(0.0f,0.5f,0.0f,0.0f),
	//									glm::vec4(0.0f,0.0f,0.5f,0.0f),
	//									glm::vec4(0.5f,0.5f,0.5f,1.0f)	);

	return m_LightMatrix;
}

void gfx::ShadowMap::SetMatrix(const glm::mat4& mat)
{
	m_LightMatrix = mat;
}
