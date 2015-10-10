/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <GL/glew.h>
#include <fstream>
#include <memory/Alloc.h>
using std::ifstream;
namespace gfx
{
	class Shader
	{
	private:
		GLuint m_Handle;
		rString m_Path;
		GLenum m_Type;
	public:
		Shader();
		~Shader();
		bool CreateFromFile( const rString& Filename, GLenum ShaderType, bool print );
		bool CreateFromString( rString ShaderCode, GLenum ShaderType, const rString& filename, bool print );
		GLuint  GetHandle();
		rString GetDir( rString filename );
		rString LoadText( const rString& filename );
		void Clear();
		void Recompile();
	};
}