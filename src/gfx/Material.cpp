/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "Material.h"

void gfx::Material::CreateBitFlags()
{
    ShaderBitFlags flags = NONE;

    if( m_AlbedoTexture != "")
    {
        flags = flags | FRAGMENT_DIFFUSEMAP;
    }
    if(m_NormalTexture != "")
    {
        flags = flags | FRAGMENT_NORMALMAP;
    }
    if(m_RoughnessTexture != "")
    {
        flags = flags | FRAGMENT_ROUGHNESSMAP;
    }
	if(m_MetalTexture != "")
	{
		flags = flags | FRAGMENT_METALMAP;
	}
		if(m_GlowTexture != "")
	{
		flags = flags | FRAGMENT_GLOW;
	}

    m_BitFlags = flags;
}
