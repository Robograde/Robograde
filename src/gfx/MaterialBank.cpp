/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "MaterialBank.h"
#include <fstream>
#include <iostream>
#include "Model.h"
#include "BufferManager.h"
gfx::MaterialBank::MaterialBank()
{
	m_MaterialBuffer = "MaterialBuffer";
	m_TextureBuffer = "TextureBuffer";
}

gfx::MaterialBank::~MaterialBank()
{
	//ClearMaterials();
}

gfx::MaterialBank& gfx::MaterialBank::GetInstance()
{
	static MaterialBank m_Bank;
	return m_Bank;
}

void gfx::MaterialBank::LoadMaterials(rString filename)
{
	Model dummy;
	ObjectImporter assetImporter;
	assetImporter.LoadObject(filename, dummy);
	rVector<RoboMat> mats = assetImporter.GetMaterials();
	
	for ( auto& it : mats )
	{
		Material* mat = pNew( Material );
		mat->SetBaseColor(vec3(it.Color[0],it.Color[1],it.Color[2]));
		mat->SetSpecularColor(vec3(it.Specularity[0],it.Specularity[1],it.Specularity[2]));
		mat->SetName(it.Name);
		if(it.HasAlbedoMap)
		{
			mat->SetAlbedoTexture(it.AlbedoTex);
		}
		if(it.HasBumpMap)
		{
			mat->SetNormalTexture(it.BumpTex);
		}
		if(it.HasRoughMap)
		{
			mat->SetRoughnessTexture(it.RoughTex);
		}
		if(it.HasMetalMap)
		{
			mat->SetMetalTexture(it.MetalTex);
		}
		if(it.HasGlowMap)
		{
			mat->SetGlowTexture(it.GlowTex);
		}
		m_Materials.push_back( mat );
		m_MatMap.emplace(mat->GetName(),mat);
	}
	//TODOHJ: HAX for restarting
	m_TextureAtlasses = pNewArray(TextureAtlas,Texture_Atlas_Type::Count);
}

rString gfx::MaterialBank::GetDir(rString& filename)
{
	bool found = false;
	for (int i = static_cast<int>(filename.size()); i > 0; i--)
	{
		if (filename.c_str()[i] == '/' || filename.c_str()[i] == '\\' )
			found = true;
		if (!found)
		{
			filename.erase(i);
		}

	}
	return filename;
}

void gfx::MaterialBank::BuildMaterialBuffer(){
	//Generate GPUTextures
	rVector<GPUTexture> textures;
	int texCounter = 0;
	for(int i = 0; i < Texture_Atlas_Type::Count;i++)
	{
		for(auto& it : m_TextureAtlasses[i].GetGPUTextures())
		{
			textures.push_back(it);
		}
		for(auto& it : m_Materials)
		{
			switch ( i )
			{
				case Texture_Atlas_Type::Diffuse:
				{
					rString texName = it->GetDiffuseTexture();
					int texHandle =  m_TextureAtlasses[i].GetHandle( texName ); 
					if(texHandle >= 0)
						it->SetAlbedoTextureHandle( texCounter + texHandle );
					else
						it->SetAlbedoTextureHandle( texHandle );
					break;
				}
				case Texture_Atlas_Type::Normal:
				{
					rString texName = it->GetNormalTexture();
					int texHandle =  m_TextureAtlasses[i].GetHandle( texName ); 
					if(texHandle >= 0)
						it->SetNormalTextureHandle( texCounter + texHandle );
					else
						it->SetNormalTextureHandle( texHandle );
					break;
				}
				case Texture_Atlas_Type::Roughness:
				{
					rString texName = it->GetRoughnessTexture();
					int texHandle =  m_TextureAtlasses[i].GetHandle( texName ); 
					if(texHandle >= 0)
						it->SetRoughnessTextureHandle( texCounter + texHandle );
					else
						it->SetRoughnessTextureHandle( texHandle );
					break;
				}
				case Texture_Atlas_Type::Metal:
				{
					rString texName = it->GetMetalTexture();
					int texHandle =  m_TextureAtlasses[i].GetHandle( texName ); 
					if(texHandle > 0)
						it->SetMetalTextureHandle( texCounter + texHandle );
					else
						it->SetMetalTextureHandle( texHandle );
					break;
				}
				case Texture_Atlas_Type::Glow:
				{
					rString texName = it->GetGlowTexture();
					int texHandle =  m_TextureAtlasses[i].GetHandle( texName ); 
					if(texHandle >= 0)
						it->SetGlowTextureHandle( texCounter + texHandle );
					else
						it->SetGlowTextureHandle( texHandle );
					break;
				}
				default:
					break;
			}
		}
		texCounter += static_cast<int>(m_TextureAtlasses[i].GetGPUTextures().size());
	}
	rVector<SurfaceMaterial> mats;
	for (int i = 0; i < m_Materials.size(); ++i){
		mats.push_back(m_Materials[i]->GetSurfaceMaterial());
	}
	g_BufferManager.CreateBufferWithData(m_MaterialBuffer, GL_UNIFORM_BUFFER, static_cast<int>(mats.size() * sizeof(SurfaceMaterial)), GL_DYNAMIC_DRAW, mats.data());
	g_BufferManager.CreateBufferWithData(m_TextureBuffer, GL_UNIFORM_BUFFER, static_cast<int>(textures.size() * sizeof(GPUTexture)), GL_DYNAMIC_DRAW, textures.data());
}

void gfx::MaterialBank::ClearMaterials()
{
	for ( auto& material : m_Materials )
	{
		pDelete( material );
	}
	m_Materials.clear();
	m_MatMap.clear();
	pDeleteArray(m_TextureAtlasses);
}

void gfx::MaterialBank::LoadTextureAtlass(const rString& filename,Texture_Atlas_Type type)
{
	m_TextureAtlasses[type].LoadFile(filename);
}

rString gfx::MaterialBank::GetMaterialBuffer(){
	return m_MaterialBuffer;
}

rString gfx::MaterialBank::GetTextureBuffer()
{
	return m_TextureBuffer;
}

gfx::Material* gfx::MaterialBank::GetMaterial(int matIndex){
	if(matIndex == -1)
		return nullptr;
	return m_Materials[matIndex];
}

gfx::Material* gfx::MaterialBank::GetMaterial(const rString& name){

	rMap<rString, Material*>::iterator it = m_MatMap.find(name);
	if(it != m_MatMap.end())
	{
		return m_MatMap[name];
	}
	else
		return nullptr;
}

int gfx::MaterialBank::GetMaterialID(const rString& name)
{
	int i = 0; 
	for(auto& it : m_Materials)
	{
		if(it->GetName() == name)
			return i;
		i++;
	}
	return -1;
}

gfx::Texture* gfx::MaterialBank::GetTextureAtlasTex(Texture_Atlas_Type type)
{
	return m_TextureAtlasses[type].GetTexture();
}
