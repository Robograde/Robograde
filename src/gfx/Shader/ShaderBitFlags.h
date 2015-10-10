/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <algorithm>
namespace gfx
{
    //these define a shader
    //they will be sorted before render
    //so opaque objects should be last in the enum
    //to make sure they will be rendered last etc
    enum ShaderBitFlags
    {
        NONE                        = 0,
        UNIQUE                      = 1 << 0,
        VERTEX_STANDARD             = 1 << 1,
        VERTEX_ANIMATED             = 1 << 2,
		VERTEX_DRAWID				= 1 << 3,
        FRAGMENT_DIFFUSEMAP         = 1 << 4,
        FRAGMENT_NORMALMAP          = 1 << 5,
        FRAGMENT_ROUGHNESSMAP       = 1 << 6,
		FRAGMENT_METALMAP			= 1 << 7,
		FRAGMENT_GLOW				= 1 << 8,
        FRAGMENT_OPAQUE             = 1 << 9,
		

		PARTICLE_INITIAL			= 1 << 16,
    };

    inline ShaderBitFlags operator|(ShaderBitFlags a, ShaderBitFlags b)
    {
        return static_cast<ShaderBitFlags>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline bool ShaderBitsCmp(ShaderBitFlags first, ShaderBitFlags second)
    {
        return static_cast<int>(first) < static_cast<int>(second);
    }

    inline rVector<ShaderBitFlags> SortShaderBits(rVector<ShaderBitFlags> list)
    {
        std::sort(list.begin(), list.end(),ShaderBitsCmp);
        return list;
    }
}
