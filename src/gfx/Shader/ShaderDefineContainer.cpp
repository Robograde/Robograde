/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "ShaderDefineContainer.h"
#include <sstream>
gfx::ShaderDefineContainer::ShaderDefineContainer()
{
	m_Empty = true;
}

gfx::ShaderDefineContainer::~ShaderDefineContainer()
{

}
//Defines should be name and value only.
//#define will be added automaticly
void gfx::ShaderDefineContainer::AddDefine ( const rString& defineName )
{
    m_GlobalDefines.push_back ( defineName );
	m_Empty = false;
}

void gfx::ShaderDefineContainer::AddDefineToShaderStage ( const rString& defineName, GLenum shaderStage )
{
    switch ( shaderStage )
    {
    case GL_VERTEX_SHADER:
    {
        m_VertexDefines.push_back ( defineName );
    }
    break;
    case GL_GEOMETRY_SHADER:
    {
        m_GeometryDefines.push_back ( defineName );
    }
    break;
    case GL_TESS_CONTROL_SHADER:
    {
        m_ControlDefines.push_back ( defineName );
    }
    break;
    case GL_TESS_EVALUATION_SHADER:
    {
        m_EvaluationDefines.push_back ( defineName );
    }
    break;
    case GL_FRAGMENT_SHADER:
    {
        m_FragmentDefines.push_back ( defineName );
    }
    break;
    case GL_COMPUTE_SHADER:
    {
        m_ComputeDefines.push_back ( defineName );
    }
    break;
    }
    m_Empty = false;
}

rString gfx::ShaderDefineContainer::GetDefinesShaderStage ( GLenum shaderStage )
{
    rStringStream ss;
    switch ( shaderStage )
    {
    case GL_VERTEX_SHADER:
    {
        for ( auto& it: m_VertexDefines )
        {
            ss << "\n #define " << it << " \n";
        }
    }
    break;
    case GL_GEOMETRY_SHADER:
    {
        for ( auto& it: m_GeometryDefines )
        {
            ss << "\n #define " << it << " \n";
        }
    }
    break;
    case GL_TESS_CONTROL_SHADER:
    {
        for ( auto& it: m_ControlDefines )
        {
            ss << "\n #define " << it << " \n";
        }
    }
    break;
    case GL_TESS_EVALUATION_SHADER:
    {
        for ( auto& it: m_EvaluationDefines )
        {
            ss << "\n #define " << it << " \n";
        }
    }
    break;
    case GL_FRAGMENT_SHADER:
    {
        for ( auto& it: m_FragmentDefines )
        {
            ss << "\n #define " << it << " \n";
        }
    }
    break;
    case GL_COMPUTE_SHADER:
    {
        for ( auto& it: m_ComputeDefines )
        {
            ss << "\n #define " << it << " \n";
        }
    }
    break;
    }
    return rString(ss.str().c_str());
}

rString gfx::ShaderDefineContainer::GetAllDefineLines()
{
    rStringStream ss;
    for ( auto& it: m_GlobalDefines )
    {
        ss << "\n #define " << it << " \n";
    }
    return rString(ss.str().c_str());
}

bool gfx::ShaderDefineContainer::IsEmpty()
{
	return m_Empty;
}
