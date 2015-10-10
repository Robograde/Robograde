/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "LineRenderProgram.h"
#include "RenderJobManager.h"
gfx::LineRenderProgram::LineRenderProgram()
{

}

gfx::LineRenderProgram::~LineRenderProgram()
{

}

void gfx::LineRenderProgram::Init(gfx::RenderJobManager* jobManager)
{
	m_ProgramHandle = g_ShaderBank.LoadShaderProgram("../../../shader/LineShader.glsl");
	unsigned char texturedata[12] = { 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0 };

	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,texturedata);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_DashedTexture);
	glBindTexture(GL_TEXTURE_2D, m_DashedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8,3,1,0,GL_RGBA,GL_UNSIGNED_BYTE,texturedata);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	m_RenderJobManager = jobManager;
	m_AnimationCounter = 0;
}

void gfx::LineRenderProgram::Draw()
{
	glDisable( GL_DEPTH_TEST );
	glLineWidth(4.0f);
	glEnable(GL_BLEND);
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ProgramHandle);
	prog->Apply();
	glm::mat4 vp = m_RenderJobManager->GetProj() * m_RenderJobManager->GetView();
	prog->SetUniformMat4("vp", vp);
	
	rVector<Line>& lines = m_RenderJobManager->GetLines();
	for(int i = 0; i < lines.size(); i++)
	{
		prog->SetUniformVec4("Color", lines[i].Color);
		prog->SetUniformVec3("pos1", lines[i].Origin);
		prog->SetUniformVec3("pos2", lines[i].Destination);
		prog->SetUniformFloat("animation", lines[i].AnimationProgres);
		if(lines[i].Dashed)
		{
			prog->SetUniformTextureHandle("Tex", m_DashedTexture, 0);
		}
		else
		{
			prog->SetUniformTextureHandle("Tex", m_Texture, 0);
		}
		glBindVertexArray( 0 );
		glDrawArrays(GL_POINTS, 0, 1);
	}
	glUseProgram( 0 );
	glEnable( GL_DEPTH_TEST );
	glLineWidth(1.0f);
	glDisable(GL_BLEND);
}