/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include "Texture.h"
#include <glm/glm.hpp>
#include "gfx/VertexBuffer.h"
namespace gfx
{
	struct Mesh{
		unsigned int VertexBufferOffset;
		unsigned int IndexBufferOffset;
		int Indices;
		int Size;
		rString Material;
	};

	struct Model{
	public:
		unsigned int IndexHandle; //place in ModelIndexBuffer
		unsigned int VertexHandle; //place in ModelVertexBuffer
		rString Name;
		int NumVertices;
		int NumIndices;
		bool Loaded;
		VertexType Type;
		glm::vec3 Min;
		glm::vec3 Max;
		rVector<Mesh> Meshes;
	};
}