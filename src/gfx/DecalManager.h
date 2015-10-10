/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include "GFXLibraryDefine.h"
#include "TextureAtlas.h"

#define g_DecalManager gfx::DecalManager::GetInstance()
namespace gfx
{
#define MAX_DECALS 1024
#define DECAL_BUFFER_SIZE sizeof(Decal) * MAX_DECALS + sizeof(GPUTexture) * MAX_DECALS
	struct Decal
	{
		glm::mat4  		World;
		glm::mat4  		InvWorld;
		glm::vec4  		Tint;
		unsigned int  	Texture;
		glm::uvec3 	 	pad; // For other kind of textures later
	};

	class DecalManager
	{
		public:
				~DecalManager();
		GFX_API	static DecalManager& GetInstance();
				void Initialize();
		GFX_API	void Update();
		GFX_API	void Shutdown();
				void Apply();
				void Clear( );
		GFX_API	void AddToQueue(const Decal& decal);
		GFX_API	int GetDecalCount() const;
				TextureAtlas* GetTextureAtlas(){ return &m_TextureAtlas; };
		private:
			DecalManager();
			rString			m_Buffer;
			TextureAtlas	m_TextureAtlas;
			rVector<gfx::Decal> m_Decals;
	};
};
