/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "DebugRenderer.h"
#include <glm/gtx/transform.hpp>
	#include <profiler/AutoGPUProfiler.h>
#define PI 3.1415926535897932384626433832
#define TAU 2 * PI
gfx::DebugRenderer::DebugRenderer()
{

}

gfx::DebugRenderer::~DebugRenderer()
{

}

void gfx::DebugRenderer::Initialize()
{
	rVector<VertexPos> vertices;
	VertexPos vertex;
	//circle
	//32 vertices
	//Line strip
	m_SphereNumvertices = 32;
	m_BoxPos = (m_SphereNumvertices + 1) * 3;
	for(int i = 0; i < m_SphereNumvertices; ++i)
	{
		vertex.Position = glm::vec3(cos(i * (TAU / m_SphereNumvertices)) * 0.5f, 0, sin(i * (TAU / m_SphereNumvertices)) * 0.5f);
		vertices.push_back(vertex);
	}
	vertex.Position = glm::vec3(0.5f,0,0);
	vertices.push_back(vertex);
	for(int i = 0; i < m_SphereNumvertices; ++i)
	{
		vertex.Position = glm::vec3(sin(i * (TAU / m_SphereNumvertices)) * 0.5f ,cos(i * (TAU / m_SphereNumvertices)) * 0.5f, 0);
		vertices.push_back(vertex);
	}
	vertex.Position = glm::vec3(0,0.5f,0);
	vertices.push_back(vertex);
	for(int i = 0; i < m_SphereNumvertices; ++i)
	{
		vertex.Position = glm::vec3(0,sin(i * (TAU / m_SphereNumvertices)) * 0.5f,  cos(i * (TAU / m_SphereNumvertices)) * 0.5f);
		vertices.push_back(vertex);
	}
	vertex.Position = glm::vec3(0,0,0.5f);
	vertices.push_back(vertex);
	//box
	//Lines
	m_BoxVertexCount = 24;
	vertex.Position = glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,-0.5f, 0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,-0.5f, 0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices.push_back(vertex);

	vertex.Position = glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.5f,0.5f,-0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.5f,0.5f, 0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,-0.5f,0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,0.5f,0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices.push_back(vertex);

	vertex.Position = glm::vec3(-0.5f, 0.5f,-0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3( 0.5f, 0.5f, 0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3( 0.5f, 0.5f, 0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices.push_back(vertex);		   
	vertex.Position = glm::vec3(-0.5f, 0.5f,-0.5f);
	vertices.push_back(vertex);
	

	m_AxisesPos = m_BoxPos + m_BoxVertexCount;
	m_AxisesCount = 6;
	//axises
	//x axis
	vertex.Position = glm::vec3(0,0,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(1,0,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(1,0,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0.8f, 0.2f, 0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(1,0,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0.8f, -0.2f, 0);
	vertices.push_back(vertex);

	//y axis
	vertex.Position = glm::vec3(0,0,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0,1,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0,1,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0.2f, 0.8f, 0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0,1,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(-0.2f, 0.8f, 0);
	vertices.push_back(vertex);

	//z axis
	vertex.Position = glm::vec3(0,0,0);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0,0,1);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0,0,1);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0, 0.2f, 0.8f);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0,0,1);
	vertices.push_back(vertex);
	vertex.Position = glm::vec3(0, -0.2f, 0.8f);
	vertices.push_back(vertex);

	int gridSize = 500;
	int gridPatchCount = 100;
	m_GridPos = m_AxisesPos + m_AxisesCount * 3;
	m_GridCount = (gridPatchCount + 1) * 4;
	float delta = (float)gridSize / gridPatchCount;
	for(int i = 0; i < gridPatchCount + 1;++i)
	{
		vertex.Position = glm::vec3(0 , 0, delta * i);
		vertices.push_back(vertex);
		vertex.Position = glm::vec3(gridSize, 0, delta * i);
		vertices.push_back(vertex);
	}
	for(int i = 0; i < gridPatchCount + 1;++i)
	{
		vertex.Position = glm::vec3(delta * i , 0, 0);
		vertices.push_back(vertex);
		vertex.Position = glm::vec3(delta * i, 0,gridSize);
		vertices.push_back(vertex);
	}

	m_VertexBuffer.Init(POS, vertices.data(), static_cast<unsigned int>(vertices.size()) * sizeof(VertexPos), 3 );
	m_ShaderProg = g_ShaderBank.LoadShaderProgram("../../../shader/Debug.glsl");
	m_ShaderProgRay = g_ShaderBank.LoadShaderProgram("../../../shader/DebugRay.glsl");
}

void gfx::DebugRenderer::SetViewProj(const glm::mat4& view, const glm::mat4& proj)
{
	m_ViewProj = proj * view;
	m_View  = view;
	m_Proj = proj;
}

void gfx::DebugRenderer::RenderBox(const glm::mat4& world,const glm::vec3& color)
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ShaderProg);
	prog->Apply();
	prog->SetUniformMat4("wvp", m_ViewProj * world);
	prog->SetUniformVec3("Color",color);
	m_VertexBuffer.Apply();
	GPU_PROFILE( AutoGPUProfiler gpBox("DebugRendererBox", true); );
	glDrawArrays(GL_LINES,m_BoxPos,m_BoxVertexCount);
	GPU_PROFILE( gpBox.Stop(); );
}

void gfx::DebugRenderer::RenderSphere(const glm::mat4& world,const glm::vec3& color)
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ShaderProg);
	prog->Apply();
	prog->SetUniformMat4("wvp", m_ViewProj * world);
	prog->SetUniformVec3("Color",color);
	m_VertexBuffer.Apply();

	GPU_PROFILE( AutoGPUProfiler first("DebugRendererSphere", true); );
	glDrawArrays(GL_LINE_STRIP,0,m_SphereNumvertices + 1);
	GPU_PROFILE( first.Stop(); );

	GPU_PROFILE( AutoGPUProfiler second("DebugRendererSphere", true); );
	glDrawArrays(GL_LINE_STRIP,m_SphereNumvertices + 1 ,m_SphereNumvertices + 1 );
	GPU_PROFILE( second.Stop(); );

	GPU_PROFILE( AutoGPUProfiler third("DebugRendererSphere", true); );
	glDrawArrays(GL_LINE_STRIP,(m_SphereNumvertices + 1) * 2,m_SphereNumvertices + 1);
	GPU_PROFILE( third.Stop(); );
}

void gfx::DebugRenderer::RenderDebugInfo()
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ShaderProg);
	prog->Apply();
	m_VertexBuffer.Apply();
	prog->SetUniformMat4("wvp", m_ViewProj);
	prog->SetUniformVec3("Color",glm::vec3(0.5f,0.5f,0.5f));
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_GridPos, m_GridCount);
		GPU_PROFILE( gpuProfile.Stop(); );
	}

	prog->SetUniformMat4("wvp", m_ViewProj * glm::scale(glm::vec3(10)));
	prog->SetUniformVec3("Color",glm::vec3(1,0,0));
	{
		GPU_PROFILE( AutoGPUProfiler("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_AxisesPos,m_AxisesCount);
	}
	prog->SetUniformVec3("Color",glm::vec3(0,1,0));
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_AxisesPos + m_AxisesCount,m_AxisesCount);
		GPU_PROFILE( gpuProfile.Stop(); );
	}
	prog->SetUniformVec3("Color",glm::vec3(0,0,1));
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_AxisesPos + m_AxisesCount * 2 ,m_AxisesCount);
		GPU_PROFILE( gpuProfile.Stop(); );
	}

	glm::vec4 clipSpaceCoord = glm::vec4(-0.8f, -0.8f, 0.5f, 1.0f);
	glm::mat4 invVP = glm::inverse(m_ViewProj);
	glm::vec4 homogeneousCoord = invVP * clipSpaceCoord;
	glm::vec3 worldCoords = glm::vec3(homogeneousCoord) / homogeneousCoord.w;

	prog->SetUniformMat4("wvp", m_ViewProj * glm::translate(worldCoords) * glm::scale(glm::vec3(0.1f)));
	prog->SetUniformVec3("Color",glm::vec3(1,0,0));
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_AxisesPos,m_AxisesCount);
		GPU_PROFILE( gpuProfile.Stop(); );
	}
	prog->SetUniformVec3("Color",glm::vec3(0,1,0));
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_AxisesPos + m_AxisesCount,m_AxisesCount);
		GPU_PROFILE( gpuProfile.Stop(); );
	}
	prog->SetUniformVec3("Color",glm::vec3(0,0,1));
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererInfo", true); );
		glDrawArrays(GL_LINES,m_AxisesPos + m_AxisesCount * 2 ,m_AxisesCount);
		GPU_PROFILE( gpuProfile.Stop(); );
	}
}

void gfx::DebugRenderer::RenderRay(const glm::vec3& pos, const glm::vec3& pos2, const glm::vec3& color)
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_ShaderProgRay);
	prog->Apply();
	prog->SetUniformVec3("pos1", pos);
	prog->SetUniformVec3("pos2", pos2);
	prog->SetUniformVec3("Color", color);
	prog->SetUniformMat4("vp",m_ViewProj);
	glBindVertexArray(0);
	GPU_PROFILE( AutoGPUProfiler gpuProfile("DebugRendererRay", true); );
	glDrawArrays(GL_POINTS, 0, 1);
	GPU_PROFILE( gpuProfile.Stop(); );
}
