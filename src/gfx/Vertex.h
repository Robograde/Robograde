/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <glm/glm.hpp>
namespace gfx
{
	struct VertexPosNormalTex
	{
		VertexPosNormalTex()
		{

		}
		VertexPosNormalTex( glm::vec3 p, glm::vec3 n, glm::vec2 uv )
		{
			Position = p;
			Normal = n;
			TexCoord = uv;
		}
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	struct VertexPosNormalTexTexel
	{
		VertexPosNormalTexTexel()
		{

		}
		VertexPosNormalTexTexel( glm::vec3 p, glm::vec3 n, glm::vec2 uv, glm::vec2 texelUV )
		{
			Position = p;
			Normal = n;
			TexCoord = uv;
			TexelCoord = texelUV;
		}
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec2 TexelCoord;
	};

	struct VertexPosNormalTexTangent
	{
		VertexPosNormalTexTangent()
		{

		}
		VertexPosNormalTexTangent( glm::vec3 p, glm::vec3 n, glm::vec2 uv, glm::vec3 t )
		{
			Position = glm::vec4(p,1.0f);
			Normal = glm::vec4(n,0.0f);
			TexCoord = glm::vec4(uv.x,uv.y,0.0f,0.0f);
			Tangent = glm::vec4(t,0.0f);
		}
		glm::vec4 Position;
		glm::vec4 Normal;
		glm::vec4 TexCoord;
		glm::vec4 Tangent;
	};

	struct VertexPosNormalTexTangentJointsWeights
	{
		VertexPosNormalTexTangentJointsWeights()
		{

		}
		VertexPosNormalTexTangentJointsWeights( glm::vec3 p, glm::vec3 n, glm::vec2 uv, glm::vec3 t, glm::vec4 jointids, glm::vec4 weights)
		{
			Position = glm::vec4( p, 1.0f );
			Normal = glm::vec4( n, 0.0f );
			TexCoord = glm::vec4( uv.x, uv.y, 0.0f, 0.0f );
			Tangent = glm::vec4( t, 0.0f );
			JointIDs = jointids;
			Weights = weights;
		}
		glm::vec4 Position;
		glm::vec4 Normal;
		glm::vec4 TexCoord;
		glm::vec4 Tangent;
		glm::vec4 JointIDs;
		glm::vec4 Weights;
	};

	//may be used for single color model or geometry shader based particle system
	struct VertexPos
	{
		glm::vec3 Position;
	};
	//may be used for screenaligned quad and geometry without advanced lighting model
	struct VertexPosTex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};
	//used for single colored models with lighting
	struct VertexPosNormal
	{
		glm::vec3 Position;
		glm::vec3 Normal;
	};
}