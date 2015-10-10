/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <GL/glew.h>
#include "Shader/ShaderBank.h"

#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

namespace gfx
{
	#define g_BufferManager BufferManager::GetInstance()
	class BufferManager
	{
	public:
		~BufferManager();
		GFX_API static BufferManager& GetInstance();
		GFX_API GLuint CreateBuffer(const rString& name, GLenum type, int size, GLenum usage);
		GLuint CreateBufferWithData(const rString& name, GLenum type, int size, GLenum usage, void* data);
		GFX_API void BindBufferToProgram( const rString& name, ShaderProgram* program, GLuint bindingpoint);
		void BindBufferWithRange( const rString& name, ShaderProgram* program, GLuint bindingpoint, int size);
		void ResizeBuffer(const rString& name, int newsize);
		GFX_API void UpdateBuffer(const rString& name, int offset, void* data, int size);
		void* MapBuffer(const rString& name, GLenum usage);
		void UnmapBuffer();
		bool IsProgramBound(ShaderProgramHandle program, GLuint bindingpoint);
		bool IsBufferBound(GLuint buffer, GLuint bindingpoint);
		GFX_API void ClearBuffers();
	private:
		BufferManager();
		rMap<rString,GLuint>				m_Buffers; //Name to buffers
		rMap<GLuint,GLenum>					m_BufferTypes; //Buffers to types
		rMap<GLuint, GLuint>				m_Bindings; //Buffers bound to bindingpoints
		rMap<ShaderProgramHandle, rVector<GLuint>>	m_ProgramBindings; //ShaderPrograms to bindingpoints
		GLint								m_CurrentMappedBuffer;
	};
}
