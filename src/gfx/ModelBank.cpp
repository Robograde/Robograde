/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "ModelBank.h"
#include "MaterialBank.h"

gfx::ModelBank::ModelBank()
{
	m_ModeltypeOffsets[POS_NORMAL_TEX_TANGENT]					= 0;
	m_ModeltypeOffsets[POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS]	= 100000;

	m_Numerators[POS_NORMAL_TEX_TANGENT]						= 0;
	m_Numerators[POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS]			= m_ModeltypeOffsets[POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS];

	m_MeshImporter = ObjectImporter( );
}

gfx::ModelBank::~ModelBank()
{
	this->Clear();
}

gfx::ModelBank& gfx::ModelBank::GetInstance()
{
	static ModelBank m_Bank;
	return m_Bank;
}

gfx::ModelHandle gfx::ModelBank::LoadModel(const char* filename)
{
	for (auto& it : m_Models)
	{
		if (it.second.Name == filename)
			return it.first;
	}

	Model model;
	if ( m_MeshImporter.LoadObject( filename, model ) )
	{
		VertexType t = model.Type;
		ModelHandle id = ++m_Numerators[POS_NORMAL_TEX_TANGENT];
		//ModelHandle id = ++m_Numerators[t];
		model.Loaded = true;
		m_Models[id] = model;
		return id;
	}
	else
	{
		return -1;
	}
	m_MeshImporter.Release();
}

gfx::ModelHandle gfx::ModelBank::AddModel( Model& TheModel )
{
	ModelHandle id = ++m_Numerators[POS_NORMAL_TEX_TANGENT];
	//ModelHandle id = ++m_Numerators[TheModel.Type];
	m_Models[id] = TheModel;
	return id;
}

void gfx::ModelBank::DeleteModel(ModelHandle& handle)
{
	//TODOHJ: Delete model data ,vertices and indices.
	//then update all the other models after it in the memory.
	// tbh its much easier and less cumnbersome to just delete all models and load them in again.
}

const gfx::Model& gfx::ModelBank::FetchModel(ModelHandle handle)
{
	return m_Models[handle];
}

void gfx::ModelBank::Clear()
{
	m_Models.clear();

	for ( auto& bufferEntry : m_IndexBuffers )
	{
		glDeleteBuffers( 1, &bufferEntry.second );
	}
	for ( auto& bufferEntry : m_VertexBuffers )
	{
		bufferEntry.second.Release();
	}
	for ( auto& numerator : m_Numerators )
	{
		numerator.second = 0;
	}

	//glDeleteBuffers(1, &m_IndexBuffer);
	//m_VertexBuffer.Release();
	//m_Numerator = 0;
}

void gfx::ModelBank::BuildBuffers()
{
	// Vertex buffers
	m_VertexBuffers[POS_NORMAL_TEX_TANGENT].Init( POS_NORMAL_TEX_TANGENT, m_MeshImporter.GetVerticesStatic(), static_cast<int>(m_MeshImporter.GetVerticesSizeStatic()) * sizeof( VertexPosNormalTexTangent ), 0 );
	m_VertexBuffers[POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS].Init( POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS, m_MeshImporter.GetVerticesAnimated(), m_MeshImporter.GetVerticesSizeAnimated() * sizeof( VertexPosNormalTexTangentJointsWeights ), 5 );

	//IndexBuffers
	int staticId = 0, animId = 0;
	for (rMap<ModelHandle, Model>::iterator it = m_Models.begin(); it != m_Models.end(); ++it)
	{
		if ( it->second.Type == POS_NORMAL_TEX_TANGENT )
		{
			it->second.IndexHandle = staticId;
			staticId += it->second.NumIndices;
		}
		else if ( it->second.Type == POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS )
		{
			it->second.IndexHandle = animId;
			animId += it->second.NumIndices;
		}
	}

	glGenBuffers( 1, &m_IndexBuffers[POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS] );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffers[POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_MeshImporter.GetIndicesSizeAnimated() * sizeof( unsigned int ), m_MeshImporter.GetIndicesAnimated(), GL_STATIC_DRAW );

	glGenBuffers( 1, &m_IndexBuffers[POS_NORMAL_TEX_TANGENT] );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffers[POS_NORMAL_TEX_TANGENT] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_MeshImporter.GetIndicesSizeStatic() * sizeof( unsigned int ), m_MeshImporter.GetIndicesStatic(), GL_STATIC_DRAW );

	m_MeshImporter.Release();
}

void gfx::ModelBank::ApplyBuffers( VertexType type)
{
	m_VertexBuffers[type].Apply();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffers[type]);
}

gfx::VertexBuffer& gfx::ModelBank::GetVertexBuffer( VertexType modelType )
{
	return m_VertexBuffers[modelType];
}

GLuint& gfx::ModelBank::GetIndexBuffer(VertexType modelType)
{
	return m_IndexBuffers[modelType];
}

void gfx::ModelBank::UpdateModel(ModelHandle& handle, Model& model)
{
	m_Models[handle] = model;
}
