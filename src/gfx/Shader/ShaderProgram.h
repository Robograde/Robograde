/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "ShaderDefineContainer.h"
#include "../Texture.h"
#include "ShaderProgramBinary.h"
using std::ifstream;

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
class ShaderProgram
{
public:
	GFX_API ShaderProgram ( void );
	GFX_API ShaderProgram ( rVector<Shader*> Shaders );
	GFX_API ~ShaderProgram ( void );
	GFX_API bool Init ( rVector<Shader*> Shaders, bool print );
	GFX_API GLuint GetHandle();
	GFX_API rString& GetFilename();
	GFX_API void Apply();
	GFX_API bool Validate();
	GFX_API GLuint GetUniformLocationByName ( const rString& name );
	GFX_API bool LoadCompleteShaderProgramFromFile ( const rString& filename, bool print );
	GFX_API bool LoadCompleteShaderProgramFromFile ( const rString& filename, bool print, ShaderDefineContainer& container );
	GFX_API void SetUniformFloat ( const rString& name, const float value );
	GFX_API void SetUniformInt ( const rString& name, const int value );
	GFX_API void SetUniformUInt ( const rString& name, const unsigned int value );
	GFX_API void SetUniformBool ( const rString& name, const bool value );
	GFX_API void SetUniformVec3 ( const rString& name, const glm::vec3& value );
	GFX_API void SetUniformMat4 ( const rString& name, const glm::mat4x4& value );
	GFX_API void SetUniformVec2 ( const rString& name, const glm::vec2& value );
	GFX_API void SetUniformVec4 ( const rString& name, const glm::vec4& value );
	GFX_API void SetUniformTexture(const rString& name, Texture& tex);
	GFX_API void SetUniformTextureHandle(const rString& name, GLuint tex, int index);
	GFX_API GLint FetchUniform ( const rString& name );
	GFX_API bool Recompile();

	void SaveProgramBinary();
	void LoadProgramBinary(const rString& filename);
	bool LoadProgramBinaryHeader(const rString& filename, ShaderProgramBinary& outHeader);
private:
    GLuint                  m_Handle;
    rVector<Shader*>		m_Shaders;
    bool                    m_LoadedShaders;
    rMap<rString, GLint>    m_UniformMap;
    int                     m_TextureCount;
    bool                    m_Validated = false;
    rString                 m_Filename;
    ShaderDefineContainer   m_Defines;

    void CreateShader ( ifstream* FileStream, GLenum shaderType, const rString& filename, bool print );
    void CreateShader ( ifstream* FileStream, GLenum shaderType, const rString& filename, bool print, ShaderDefineContainer& container );
    rString GetDir ( rString filename );
    rString LoadText ( const char* filename );
};
}

