/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "LightEngine.h"
#include "BufferManager.h"
#include <sstream>
gfx::LightEngine::LightEngine()
{
	m_Buffer = "LightBuffer";
	m_PointLights.reserve( MAX_POINTLIGHTS );
	m_LineLights.reserve( MAX_LINELIGHTS );
	m_DirectionalLights.reserve( MAX_DIRLIGHTS );
}

gfx::LightEngine::~LightEngine()
{

}

gfx::LightEngine& gfx::LightEngine::GetInstance()
{
	static LightEngine instance;
	return instance;
}

void gfx::LightEngine::Initialize()
{
	g_BufferManager.CreateBuffer(m_Buffer,GL_SHADER_STORAGE_BUFFER,BUFFER_SIZE, GL_DYNAMIC_DRAW );
}

void gfx::LightEngine::AddPointLightToQueue( const Light& light )
{
	m_PointLights.push_back( light );
}

void gfx::LightEngine::AddDirLightToQueue( const Light& light )
{
	m_DirectionalLights.push_back( light );
}

void gfx::LightEngine::AddLineLightToQueue( const Light& light )
{
	m_LineLights.push_back( light );
}

void gfx::LightEngine::ClearPointLights( )
{
	m_PointLights.clear( );
}

void gfx::LightEngine::ClearDirectionalLights( )
{
	m_DirectionalLights.clear( );
}

void gfx::LightEngine::ClearLineLights( )
{
	m_LineLights.clear( );
}

void gfx::LightEngine::BindBuffer( ShaderProgramHandle handle )
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(handle);
	prog->Apply();
	g_BufferManager.BindBufferToProgram(m_Buffer, prog, 3);
}

void gfx::LightEngine::BuildBuffer()
{
	if( m_PointLights.size( ) > 0)
	{
		//update all pointlights
		g_BufferManager.UpdateBuffer(m_Buffer, 0, m_PointLights.data(), sizeof(Light) * (int)m_PointLights.size( ) );
	}
	if( m_DirectionalLights.size( ) > 0)
	{
		//dirlights
		g_BufferManager.UpdateBuffer(m_Buffer, DIRLIGHT_POS, m_DirectionalLights.data(), sizeof(Light) * (int)m_DirectionalLights.size( ) );
	}
	if( m_LineLights.size( ) > 0)
	{
		//linelights
		g_BufferManager.UpdateBuffer(m_Buffer, LINELIGHT_POS, m_LineLights.data( ), sizeof(Light) * (int)m_LineLights.size( ) );
	}
}

int gfx::LightEngine::GetPointLightCount() const
{ 
	return (int)m_PointLights.size( );
}

int gfx::LightEngine::GetDirLightCount() const
{ 
	return (int)m_DirectionalLights.size( );
}

int gfx::LightEngine::GetLineLightCount() const
{
   return (int)m_LineLights.size( );
}
