/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include "Shader/ShaderBitFlags.h"
#include <memory/Alloc.h>
#include <glm/glm.hpp>
namespace gfx 
{

	#define MAX_JOINTS 32   
    struct ShaderInput
    {
        glm::mat4 World;
        glm::vec4 Color;
		glm::ivec4 MatIDPadd3;
    };	

	struct SkeletonInput
	{
		glm::mat4 Joints[MAX_JOINTS];
	};

	struct IndirectDrawCall{
		unsigned int Count; //nvertices
		unsigned int InstanceCount; //ninstances
		unsigned int FirstIndex; //indexbuffer offset
		unsigned int BaseVertex;
		unsigned int BaseInstance;
		IndirectDrawCall(){}
		IndirectDrawCall(unsigned int count, unsigned int instanceCount, unsigned int firstIndex, unsigned int baseVertex, unsigned int baseInstance)
		{
			Count = count; InstanceCount = instanceCount; FirstIndex = firstIndex; BaseInstance = baseInstance; BaseVertex = baseVertex;
		}
    };
	struct RenderJob
    {
        rVector<ShaderInput> shaderInputs;
		rVector<glm::mat4> jointList;
		rVector<glm::mat4> bindPose;
        ShaderBitFlags  ShaderFlags;
        IndirectDrawCall Command;
	};
}
