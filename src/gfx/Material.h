/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include "Texture.h"
#include "Shader/ShaderBitFlags.h"
using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::ivec4;
namespace gfx
{
    struct SurfaceMaterial{
        vec4	BaseColor;
        vec4	SpecularColor;
		ivec4	AlbedoNormalRoughMetalTexture;
		ivec4	GlowAODecalPaddTexture;
    };

    class Material
    {
    public:
        Material(){ };
        ~Material()
		{
			m_BitFlags = gfx::NONE;
			m_Material = SurfaceMaterial();
		};
        //Getters
        rString& GetName(){ return m_Name; }

        rString& GetDiffuseTexture()					{ return m_AlbedoTexture; }
        rString& GetNormalTexture()						{ return m_NormalTexture; }
        rString& GetRoughnessTexture()					{ return m_RoughnessTexture; }
        rString& GetMetalTexture()						{ return m_MetalTexture; }
		rString& GetGlowTexture()						{ return m_GlowTexture; }

        vec3 GetBaseColor()								{ return vec3(m_Material.BaseColor.x, m_Material.BaseColor.y, m_Material.BaseColor.z); }
        vec3 GetSpecularColor()							{ return vec3(m_Material.SpecularColor.x, m_Material.SpecularColor.y, m_Material.SpecularColor.z); }

        SurfaceMaterial& GetSurfaceMaterial()			{ return m_Material; }
        //Setters
        void SetName(rString name)						{ m_Name = name; }
        void SetAlbedoTexture(const rString& tex)		{ m_AlbedoTexture = tex; }
        void SetNormalTexture(const rString& tex)		{ m_NormalTexture = tex; }
        void SetRoughnessTexture(const rString& tex)    { m_RoughnessTexture = tex; }
		void SetMetalTexture(const rString& tex)		{ m_MetalTexture = tex; }
		void SetGlowTexture(const rString& tex)			{ m_GlowTexture = tex; }

		void SetBaseColor(const vec3& color)			{ m_Material.BaseColor.x = color.x; m_Material.BaseColor.y = color.y; m_Material.BaseColor.z = color.z; }
        void SetSpecularColor(const vec3& color)		{ m_Material.SpecularColor.x = color.x; m_Material.SpecularColor.y = color.y; m_Material.SpecularColor.z = color.z; }

		void SetAlbedoTextureHandle(int t)				{ m_Material.AlbedoNormalRoughMetalTexture.x = t; }
		void SetNormalTextureHandle(int t)				{ m_Material.AlbedoNormalRoughMetalTexture.y = t; }
		void SetRoughnessTextureHandle(int t)			{ m_Material.AlbedoNormalRoughMetalTexture.z = t; }
		void SetMetalTextureHandle(int t)				{ m_Material.AlbedoNormalRoughMetalTexture.w = t; }
		void SetGlowTextureHandle(int t)				{ m_Material.GlowAODecalPaddTexture.x = t; }

        void CreateBitFlags();
        gfx::ShaderBitFlags GetBitFlags()
        {
            if( m_BitFlags == gfx::NONE)
            {
                CreateBitFlags();
            }
            return m_BitFlags;
        }
    private:
        gfx::ShaderBitFlags m_BitFlags = gfx::NONE;
        SurfaceMaterial m_Material;
        rString m_AlbedoTexture = "";
        rString m_NormalTexture = "";
        rString m_RoughnessTexture = "";
        rString m_MetalTexture = "";
		rString m_GlowTexture = "";
        rString m_Name;
    };
}
