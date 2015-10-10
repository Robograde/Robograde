/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "ShaderBank.h"
#include "BitFlagToDefine.h"
#include <utility/Logger.h>

gfx::ShaderBank::ShaderBank()
{
	m_Counter = 0;
}

gfx::ShaderBank::~ShaderBank()
{
	for ( rVector<ShaderProgram *>::iterator it = m_Programs.begin(); it != m_Programs.end(); it++ )
	{
		pDelete( *it );
	}
	m_Programs.clear();
	m_HandleToPrograms.clear();
	m_BitFlagsToPrograms.clear();
}

gfx::ShaderBank& gfx::ShaderBank::GetInstance()
{
	static ShaderBank m_Bank;
	return m_Bank;
}

gfx::ShaderProgramHandle gfx::ShaderBank::LoadShaderProgram ( const rString& filename )
{
	//find if its already in bank
	for(auto& it : m_HandleToPrograms)
	{
		if(it.second->GetFilename() == filename)
		{
			//Logger::Log( "Program filename already exist in the bank, use defines or a bitflag to vary shader compilation", "ShaderBank", LogSeverity::WARNING_MSG );
			return it.first;
		}
	}
	ShaderProgram* program = pNew( ShaderProgram );
#ifdef SHADER_BINARY_PROGRAM
	
	ShaderProgramBinary header;
	if(program->LoadProgramBinaryHeader(filename,header))
	{
		if(strcmp(header.Version,(char*)glGetString(GL_VERSION)) == 0)
		{
			program->LoadProgramBinary(filename);
			ShaderProgramHandle handle = m_Counter++;
			m_Programs.push_back(program);
			m_HandleToPrograms.emplace( handle, program );
			return handle;
		}
	}
	if(program->LoadCompleteShaderProgramFromFile(filename, true))
	{
		program->SaveProgramBinary();
	}
#else
	program->LoadCompleteShaderProgramFromFile(filename, true);
#endif
	ShaderProgramHandle handle = m_Counter++;
	m_Programs.push_back(program);
	m_HandleToPrograms.emplace( handle, program );
	return handle;
}

gfx::ShaderProgramHandle gfx::ShaderBank::LoadShaderProgramWithBitFlags ( const rString& filename, gfx::ShaderBitFlags bitFlags )
{
	ShaderDefineContainer container = BitflagToDefines(bitFlags);
	ShaderProgram* program = pNew( ShaderProgram );
#ifdef SHADER_BINARY_PROGRAM
	ShaderProgramBinary header;
	if(program->LoadProgramBinaryHeader(filename,header))
	{
		if(strcmp(header.Version,(char*)glGetString(GL_VERSION)) == 0)
		{
			program->LoadProgramBinary(filename);
			ShaderProgramHandle handle = m_Counter++;
			m_Programs.push_back(program);
			m_HandleToPrograms.emplace( handle, program );
			return handle;
		}
	}
	if(program->LoadCompleteShaderProgramFromFile(filename, true,container))
	{
		program->SaveProgramBinary();
	}
#else
	
	program->LoadCompleteShaderProgramFromFile(filename, true,container);
#endif
	ShaderProgramHandle handle = m_Counter++;
	m_Programs.push_back(program);
	m_BitFlagsToPrograms.emplace( bitFlags, program );
	m_HandleToPrograms.emplace( handle, program );
	return handle;
}

gfx::ShaderProgramHandle gfx::ShaderBank::LoadShaderProgramWithDefines ( const rString& filename, ShaderDefineContainer& defines )
{
	ShaderProgram* program = pNew( ShaderProgram );
#ifdef SHADER_BINARY_PROGRAM
	
	ShaderProgramBinary header;
	if(program->LoadProgramBinaryHeader(filename,header))
	{
		if(strcmp(header.Version,(char*)glGetString(GL_VERSION)) == 0)
		{
			program->LoadProgramBinary(filename);
			ShaderProgramHandle handle = m_Counter++;
			m_Programs.push_back(program);
			m_HandleToPrograms.emplace( handle, program );
			return handle;
		}
	}
	if(program->LoadCompleteShaderProgramFromFile(filename, true,defines))
	{
		program->SaveProgramBinary();
	}
#else
	program->LoadCompleteShaderProgramFromFile(filename, true,defines);
#endif
	ShaderProgramHandle handle = m_Counter++;
	m_Programs.push_back(program);
	m_HandleToPrograms.emplace( handle, program );
	return handle;
}

gfx::ShaderProgramHandle gfx::ShaderBank::LoadShaderProgramFromShaders ( const rVector< Shader* >& shaders )
{
	ShaderProgram* program = pNew( ShaderProgram );
	program->Init(shaders,true);
	ShaderProgramHandle handle = m_Counter++;
	m_Programs.push_back(program);
	m_HandleToPrograms.emplace(handle, program);
	return handle;
}

gfx::ShaderProgram* gfx::ShaderBank::GetProgramFromHandle (gfx::ShaderProgramHandle handle )
{
    rMap<ShaderProgramHandle, ShaderProgram*>::const_iterator it = m_HandleToPrograms.find(handle);
	if(it == m_HandleToPrograms.end())
	{
		Logger::Log( "Program does not exist in this bank", "ShaderBank", LogSeverity::WARNING_MSG );
		return nullptr;
	}
	else
	{
        return m_HandleToPrograms[handle];
	}
}

gfx::ShaderProgram* gfx::ShaderBank::GetProgramFromBitFlag(gfx::ShaderBitFlags flags)
{
    rMap<ShaderBitFlags, ShaderProgram*>::const_iterator it = m_BitFlagsToPrograms.find(flags | m_DefaultFlags);
    if(it == m_BitFlagsToPrograms.end())
    {
		//Logger::Log( "Program does not exist in this bank", "ShaderBank", LogSeverity::WARNING_MSG );
		LoadShaderProgramWithBitFlags( m_DefaultShaderFilename, flags | m_DefaultFlags );
		//glorius recursiveness
		return GetProgramFromBitFlag(flags | m_DefaultFlags);
    }
    else
    {
		return it->second;
    }
}

void gfx::ShaderBank::RecompileAllShaders()
{
	for(auto& it : m_HandleToPrograms)
	{
		it.second->Recompile();
	}
	//printf("\n");
	Logger::Log( "Recompiled all shaders", "ShaderBank", LogSeverity::DEBUG_MSG );
}

void gfx::ShaderBank::Clear()
{
    //TODOHJ: Clear
}

void gfx::ShaderBank::SetDefaultShader(const rString& filename)
{
	m_DefaultShaderFilename = filename;
}

gfx::ShaderProgramHandle gfx::ShaderBank::GetDefaultShader()
{
	return LoadShaderProgram(m_DefaultShaderFilename);
}

void gfx::ShaderBank::SetDefaultFlags(ShaderBitFlags flags)
{
	m_DefaultFlags = flags;
}
