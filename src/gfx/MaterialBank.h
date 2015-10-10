/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "Material.h"
#include "TextureAtlas.h"
#include "ObjectImporter.h"
#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

#define g_MaterialBank MaterialBank::GetInstance()
typedef int TextureHandle;
namespace gfx
{

enum Texture_Atlas_Type
{
	Diffuse,
	Normal,
	Roughness,
	Metal,
	Glow,
	Count
};
class MaterialBank
{
public:
	GFX_API ~MaterialBank();
	GFX_API static MaterialBank& GetInstance();
	GFX_API void LoadTextureAtlass(const rString& filename,Texture_Atlas_Type type);
	GFX_API void LoadMaterials(rString filename);
	GFX_API void BuildMaterialBuffer();
	GFX_API void ClearMaterials();
	GFX_API rString GetMaterialBuffer();
	GFX_API Material* GetMaterial(int matIndex);
	GFX_API Material* GetMaterial(const rString& name);
	GFX_API int GetMaterialID(const rString& name);
	rString GetTextureBuffer();
	gfx::Texture* GetTextureAtlasTex(Texture_Atlas_Type type);
private:
	MaterialBank();
	rString GetDir(rString& filename);
	int							m_Numerator = 0;
	rString						m_MaterialBuffer;
	rVector<Material*>			m_Materials;
	rMap<rString, Material*>	m_MatMap;
	TextureAtlas*				m_TextureAtlasses;
	rString						m_TextureBuffer;
	
};
}
