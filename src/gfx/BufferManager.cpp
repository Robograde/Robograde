/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "BufferManager.h"
#include <utility/Logger.h>
gfx::BufferManager::BufferManager()
{

}

gfx::BufferManager::~BufferManager()
{
	//Release all buffers //just to be clean
}

gfx::BufferManager& gfx::BufferManager::GetInstance()
{
	static BufferManager m_BuffManager;
	return m_BuffManager;
}

GLuint gfx::BufferManager::CreateBuffer(const rString& name, GLenum type, int size, GLenum usage)
{
	auto it = m_Buffers.find(name);
	if(it != m_Buffers.end())
	{
		Logger::Log("Buffer name is already in the buffermanager. Change name or use resize function", "BufferManager",LogSeverity::WARNING_MSG);
		return m_Buffers[name];
	}
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(type, buffer);
	glBufferData(type,size,nullptr,usage);
	glBindBuffer(type, 0);

	m_Buffers[name] = buffer;
	m_BufferTypes[buffer] = type;
	return buffer;
}

GLuint gfx::BufferManager::CreateBufferWithData(const rString& name, GLenum type, int size, GLenum usage, void* data)
{
	auto it = m_Buffers.find(name);
	if(it != m_Buffers.end())
	{
		Logger::Log("Buffer name is already in the buffermanager. Change name or use resize function", "BufferManager",LogSeverity::WARNING_MSG);
		return m_Buffers[name];
	}
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(type, buffer);
	glBufferData(type,size,data,usage);
	glBindBuffer(type, 0);

	m_Buffers[name] = buffer;
	m_BufferTypes[buffer] = type;
	return buffer;
}

void gfx::BufferManager::BindBufferToProgram( const rString& name, ShaderProgram* program, GLuint bindingpoint)
{
	auto it = m_Buffers.find(name);
	if(it == m_Buffers.end())
	{
		Logger::Log("Buffer name is not in the manager", "BufferManager", LogSeverity::WARNING_MSG);
		return;
	}
	for(int i = 0; i < (int)m_ProgramBindings[program->GetHandle()].size(); i++)
	{
		if(m_ProgramBindings[program->GetHandle()][i] == bindingpoint)
			return;
	}

	GLuint buffer = it->second;
	GLenum type = m_BufferTypes[buffer];
	for(auto& it : m_ProgramBindings[program->GetHandle()])
	{
		if(it == bindingpoint)
		{
			m_Bindings[buffer] = bindingpoint;
			return;
		}
	}
	glBindBuffer(type,buffer);
	if(type == GL_SHADER_STORAGE_BUFFER)
	{
		GLint bi = glGetProgramResourceIndex(program->GetHandle(), GL_SHADER_STORAGE_BLOCK, name.c_str());
		if(bi < 0)
		{
			Logger::Log("Error getting bindingindex from shader", "BufferManager", LogSeverity::ERROR_MSG);
			return;
		}
		glShaderStorageBlockBinding(program->GetHandle(), bi , bindingpoint);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,bindingpoint ,buffer);
	}
	else if (type == GL_UNIFORM_BUFFER)
	{
		GLint bi = glGetUniformBlockIndex( program->GetHandle(), name.c_str() );
		if(bi < 0)
		{
			Logger::Log("Error getting bindingindex from shader", "BufferManager", LogSeverity::ERROR_MSG);
			return;
		}
		glUniformBlockBinding( program->GetHandle(), bi, bindingpoint );
		glBindBufferBase( GL_UNIFORM_BUFFER, bindingpoint, buffer );
	}
	else
	{
		Logger::Log("Unsupported buffer type", "BufferManager", LogSeverity::ERROR_MSG);
	}
	//update binding index
	m_ProgramBindings[program->GetHandle()].push_back(bindingpoint);
	m_Bindings[buffer] = bindingpoint;
	glBindBuffer(type,0);
}

void gfx::BufferManager::BindBufferWithRange( const rString& name, ShaderProgram* program, GLuint bindingpoint, int size)
{
	auto it = m_Buffers.find(name);
	if(it == m_Buffers.end())
	{
		Logger::Log("Buffer name is not in the manager", "BufferManager", LogSeverity::WARNING_MSG);
		return;
	}

	GLuint buffer = it->second;
	GLenum type = m_BufferTypes[buffer];

	for(auto& it : m_ProgramBindings[program->GetHandle()])
	{
		if(it == bindingpoint)
		{
			m_Bindings[buffer] = bindingpoint;
			return;
		}
	}
	glBindBuffer(type,buffer);
	if(type == GL_SHADER_STORAGE_BUFFER)
	{
		GLint bi = glGetProgramResourceIndex(program->GetHandle(), GL_SHADER_STORAGE_BLOCK, name.c_str());
		if(bi < 0)
		{
			Logger::Log("Error getting bindingindex from shader", "BufferManager", LogSeverity::ERROR_MSG);
			return;
		}
		glShaderStorageBlockBinding(program->GetHandle(), bi , bindingpoint);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,bindingpoint ,buffer);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingpoint, buffer, 0, size);
	}
	else if (type == GL_UNIFORM_BUFFER)
	{
		Logger::Log("Unsupported buffer type. Cant bind uniform buffer with range", "BufferManager", LogSeverity::ERROR_MSG);
		return;
	}
	else
	{
		Logger::Log("Unsupported buffer type", "BufferManager", LogSeverity::ERROR_MSG);
		return;
	}
	//update binding index

	m_ProgramBindings[program->GetHandle()].push_back(bindingpoint);
	m_Bindings[buffer]			= bindingpoint;
	glBindBuffer(type,0);
}

void gfx::BufferManager::ResizeBuffer(const rString& name, int newsize)
{
	auto it = m_Buffers.find(name);
	if(it == m_Buffers.end())
	{
		Logger::Log("Buffer name is not in the manager", "BufferManager", LogSeverity::WARNING_MSG);
		return;
	}
	GLuint buffer = it->second;
	GLenum type = m_BufferTypes[buffer];
	glBindBuffer(type,buffer);
	glBufferData(type,newsize,nullptr,GL_DYNAMIC_DRAW); //TODOHJ: Replace GL_Dynamic_draw with the proper usage
	glBindBuffer(type, 0);
}

void gfx::BufferManager::UpdateBuffer(const rString& name, int offset, void* data, int size)
{
	if(data == nullptr)
		return;

	auto it = m_Buffers.find(name);
	if(it == m_Buffers.end())
	{
		Logger::Log("Buffer name is not in the manager", "BufferManager", LogSeverity::WARNING_MSG);
		return;
	}
	GLuint buffer = it->second;
	GLenum type = m_BufferTypes[buffer];
	glBindBuffer(type,buffer);
	glBufferSubData(type,offset,size,data);
	glBindBuffer(type, 0);
}

void* gfx::BufferManager::MapBuffer(const rString& name, GLenum usage)
{
	if(m_CurrentMappedBuffer != -1)
	{
		Logger::Log("Another buffer is currently mapped", "BufferManager", LogSeverity::ERROR_MSG);
		return nullptr;
	}
	auto it = m_Buffers.find(name);
	if(it == m_Buffers.end())
	{
		Logger::Log("Buffer name is not in the manager", "BufferManager", LogSeverity::WARNING_MSG);
		return nullptr;
	}
	GLuint buffer = it->second;
	GLenum type = m_BufferTypes[buffer];
	m_CurrentMappedBuffer = buffer;
	glBindBuffer(type,buffer);
	void* data = glMapBuffer(type, usage);
	glBindBuffer(type, 0);
	return data;
}

void gfx::BufferManager::UnmapBuffer()
{
	if(m_CurrentMappedBuffer == -1)
	{
		Logger::Log("No buffer is mapped", "BufferManager", LogSeverity::ERROR_MSG);
		return;
	}
	GLenum type = m_BufferTypes[m_CurrentMappedBuffer];

	glUnmapBuffer(type);
	m_CurrentMappedBuffer = -1;
}

bool gfx::BufferManager::IsProgramBound(ShaderProgramHandle program, GLuint bindingpoint)
{
	auto it = m_ProgramBindings.find(program);
	if(it == m_ProgramBindings.end())
		return false;
	for(auto it2 : it->second)
	{
		if(it2 == bindingpoint)
			return true;
	}
	return false;
}

bool gfx::BufferManager::IsBufferBound(GLuint buffer, GLuint bindingpoint)
{
	auto it = m_Bindings.find(buffer);
	if(it == m_Bindings.end())
		return false;
	return it->second == bindingpoint;
}

void gfx::BufferManager::ClearBuffers()
{
	for( auto& it : m_Buffers)
	{
		glDeleteBuffers(1,&it.second);
	}
	m_Buffers.clear();
	m_Bindings.clear();
	m_BufferTypes.clear();
}