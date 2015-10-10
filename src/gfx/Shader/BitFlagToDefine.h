/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include "ShaderDefineContainer.h"
#include "ShaderBitFlags.h"
namespace gfx
{
    ShaderDefineContainer BitflagToDefines(ShaderBitFlags flags)
	{
		ShaderDefineContainer container;
		//TODOHJ: Fill out when a shader exist
		if (flags & VERTEX_DRAWID)
		{
			container.AddDefineToShaderStage("DRAWID_EXTENSION",GL_VERTEX_SHADER);
		}
        if(flags & FRAGMENT_DIFFUSEMAP)
		{
			container.AddDefineToShaderStage("USE_DIFFUSE_MAP",GL_FRAGMENT_SHADER);
        }
		if(flags & FRAGMENT_NORMALMAP)
		{
			container.AddDefineToShaderStage("USE_NORMAL_MAP" ,GL_FRAGMENT_SHADER);
        }
		if ( flags & PARTICLE_INITIAL )
		{
			container.AddDefineToShaderStage("PARTICLE_INITIAL", GL_COMPUTE_SHADER);
		}
		return container;
	}
}
