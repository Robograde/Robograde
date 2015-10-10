/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include "Model.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "gfx/ObjectImporter.h"
#define g_ModelBank ModelBank::GetInstance()
#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif
/*
This class loads and store all models and vertices and indices.
It also generates vertex and index buffers
*/
namespace gfx
{
	typedef int ModelHandle;
	class ModelBank{
	public:
		GFX_API ~ModelBank();
		GFX_API static ModelBank& GetInstance();
		GFX_API const Model& FetchModel(const char* filename);
		GFX_API const Model& FetchModel(ModelHandle handle);
		GFX_API ModelHandle LoadModel(const char* filename);
		GFX_API ModelHandle AddModel(Model& TheModel);
		GFX_API ModelHandle AddModel( rVector<VertexPosNormalTexTangent>& vertices, rVector<unsigned int>& indices, Model& outModel );
		GFX_API ModelHandle AddModel( rVector<VertexPosNormalTexTangentJointsWeights>& vertices, rVector<unsigned int>& indices, Model& outModel );
		GFX_API void UpdateModel(ModelHandle& handle, Model& model);
		GFX_API void BuildBuffers();
		GFX_API void DeleteModel(ModelHandle& handle);
		GFX_API void ApplyBuffers( VertexType vertexType );
		GFX_API void Clear();
		GFX_API VertexBuffer& GetVertexBuffer( VertexType modelType );
		GFX_API GLuint& GetIndexBuffer( VertexType modelType );
	private:
		ModelBank();

		rMap<VertexType, int>			m_ModeltypeOffsets;
		rMap<VertexType, VertexBuffer>	m_VertexBuffers;
		rMap<VertexType, GLuint>		m_IndexBuffers;
		rMap<VertexType, int>			m_Numerators;

		ObjectImporter			 m_MeshImporter;
		rMap<ModelHandle, Model> m_Models;
	};
}
