/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <GL/glew.h>
#include <memory/Alloc.h>
namespace gfx
{
	class ShaderDefineContainer
	{
	public:
		ShaderDefineContainer();
		~ShaderDefineContainer();

		void AddDefine ( const rString& defineName );
		void AddDefineToShaderStage ( const rString& defineName, GLenum shaderStage );
		rString GetAllDefineLines();
		rString GetDefinesShaderStage ( GLenum shaderStage );
		bool IsEmpty();
	private:
		rVector<rString> m_VertexDefines;
		rVector<rString> m_GeometryDefines;
		rVector<rString> m_FragmentDefines;
		rVector<rString> m_ControlDefines;
		rVector<rString> m_EvaluationDefines;
		rVector<rString> m_ComputeDefines;
		rVector<rString> m_GlobalDefines;
		bool m_Empty;
	};
}