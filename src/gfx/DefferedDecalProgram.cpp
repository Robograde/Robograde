/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "DefferedDecalProgram.h"
#include <profiler/AutoGPUProfiler.h>
#include "BufferManager.h"
#include "DecalManager.h"
gfx::DefferedDecalProgram::DefferedDecalProgram()
{

}

gfx::DefferedDecalProgram::~DefferedDecalProgram()
{

}

void gfx::DefferedDecalProgram::Init(RenderJobManager* jobManager)
{
	m_JobManager = jobManager;
	//Shader
	m_ProgramHandle = g_ShaderBank.LoadShaderProgram( "../../../shader/DeferredDecals.glsl" );
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_ProgramHandle );
	//bind buffers
	g_BufferManager.BindBufferToProgram( "DecalBuffer", prog, 12 );
	//cube vertices
	GLfloat cube_vertices[] = {
								// front
								-1.0, -1.0,  1.0,
								 1.0, -1.0,  1.0,
								 1.0,  1.0,  1.0,
								-1.0,  1.0,  1.0,
								// back
								-1.0, -1.0, -1.0,
								 1.0, -1.0, -1.0,
								 1.0,  1.0, -1.0,
								-1.0,  1.0, -1.0,
								};

	GLushort cube_elements[] = {
								// front
								0, 1, 2,
								2, 3, 0,
								// top
								3, 2, 6,
								6, 7, 3,
								// back
								7, 6, 5,
								5, 4, 7,
								// bottom
								4, 5, 1,
								1, 0, 4,
								// left
								4, 0, 3,
								3, 7, 4,
								// right
								1, 5, 6,
								6, 2, 1,
								};

  glGenBuffers(1, &m_IndexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 36, cube_elements, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_VertexBuffer.Init(VertexType::POS, (void*)(cube_vertices), sizeof(GLfloat) * 24 , 17); //random bindingindex
}

void gfx::DefferedDecalProgram::Draw(DrawData* data)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	m_VertexBuffer.Apply();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);

	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_ProgramHandle );
	prog->Apply();
	prog->SetUniformTextureHandle("gDecalAlbedo", g_DecalManager.GetTextureAtlas()->GetTexture()->GetHandle(), 0);
	GBuffer* gb = ((DecalData*)(data->ExtraData))->gbuffer;
	prog->SetUniformTextureHandle("gDepth",gb->GetTexture(GBUFFER_TEX::DEPTH_SENCIL32) , 1);
	glm::mat4 invvp = glm::inverse(m_JobManager->GetProj() * m_JobManager->GetView());
	prog->SetUniformMat4("gInvViewProj",invvp);
	prog->SetUniformVec2("gScreenSize",((DecalData*)(data->ExtraData))->ScreenSize);
	//draw
	GPU_PROFILE( AutoGPUProfiler gpDecals( "DrawDecals" ); );
	glDrawElementsInstanced(GL_TRIANGLES,36, GL_UNSIGNED_SHORT,((GLvoid*)(0)), g_DecalManager.GetDecalCount());
	GPU_PROFILE( gpDecals.Stop( ); );
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void gfx::DefferedDecalProgram::Shutdown()
{
	//shutdown
	//i has nothing to shut down
}
