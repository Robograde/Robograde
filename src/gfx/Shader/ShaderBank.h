/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "ShaderDefineContainer.h"
#include "ShaderProgram.h"
#include "ShaderBitFlags.h"

//#define SHADER_BINARY_PROGRAM

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
	typedef int ShaderProgramHandle;
#define g_ShaderBank ShaderBank::GetInstance()
	class ShaderBank
	{
	public:
		GFX_API static ShaderBank& GetInstance();
		GFX_API ShaderProgramHandle LoadShaderProgram(const rString& filename);
		GFX_API ShaderProgramHandle LoadShaderProgramWithDefines(const rString& filename, ShaderDefineContainer& defines);
        GFX_API ShaderProgramHandle LoadShaderProgramWithBitFlags(const rString& filename, ShaderBitFlags bitFlags);
		GFX_API ShaderProgramHandle LoadShaderProgramFromShaders(const rVector<Shader*>& shaders);
        GFX_API ShaderProgram* GetProgramFromHandle(ShaderProgramHandle handle);
        GFX_API ShaderProgram* GetProgramFromBitFlag(ShaderBitFlags flags);
		GFX_API void RecompileAllShaders();
		GFX_API void Clear();
        GFX_API void SetDefaultShader(const rString& filename);
		GFX_API void SetDefaultFlags(ShaderBitFlags flags);
		GFX_API ShaderProgramHandle GetDefaultShader();
	private:
        ShaderBank();
        ~ShaderBank();
        rVector<ShaderProgram*>						m_Programs;
        rMap<ShaderProgramHandle, ShaderProgram*>   m_HandleToPrograms;
        rMap<ShaderBitFlags,ShaderProgram*>         m_BitFlagsToPrograms;
        ShaderProgramHandle                         m_Counter;
        rString										m_DefaultShaderFilename;
		ShaderBitFlags								m_DefaultFlags;
	};
}
