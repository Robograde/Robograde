/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen & Henrik Johansson
***************************************************/

#include "utility/PlatformDefinitions.h"
#include "Terrain.h"
#include <fstream>
#include <profiler/AutoGPUProfiler.h>
#include "utility/Logger.h"
#include <soil2/SOIL2.h>
#include <SDL2/SDL.h>
#include "glm/gtx/transform2.hpp"
#include "LightEngine.h"

#include "BufferManager.h"

gfx::Terrain::Terrain()
{
}


gfx::Terrain::~Terrain()
{
	//Cleanup();
}

bool gfx::Terrain::LoadFromFile( const rString& heightMapFilename, const Terrain::Config& config )
{
	// Save config data
	m_ImageWidth =			config.ImageWidth;
	m_ImageHeight =			config.ImageHeight;
	m_MapScale =			config.HeightScale;
	m_WorldDimensionX =		config.MapDimensionX;
	m_WorldDimensionZ =		config.MapDimensionZ;
	m_TessellationFactor =	config.TessellationFactor;
	m_Position =			config.Position;
	m_HeightmapRepeat =		config.HeightmapRepeat;
	m_TextureRepeat =		config.TextureRepeat;
	m_WorldToImageCoordX =	m_ImageWidth / ((float)(m_WorldDimensionX) * m_HeightmapRepeat);
	m_WorldToImageCoordZ =	m_ImageHeight / ((float)(m_WorldDimensionZ) * m_HeightmapRepeat);
	m_HorizontalScale =		(float)(m_ImageWidth) / m_WorldDimensionX;

	if ( !LoadHeightpointsFromFile( heightMapFilename, config ) )
		return false;


	m_OGLRoughTexture.Init( "../../../asset/texture/Ground.png", TEXTURE_2D );

	m_HasTexture = true;

	glBindTexture( GL_TEXTURE_2D, m_OGLHeightTexture );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//m_Sky.Init("../../../asset/texture/cubemap/park_filtered.dds", CUBE_TEXTURE);
	//m_Irradiance.Init("../../../asset/texture/cubemap/Irradiance.dds", CUBE_TEXTURE);

	rString texturefolder = "../../../asset/texture/";
	m_Albedo1.Init((texturefolder + config.Albedo1Texture).c_str(), TEXTURE_2D);
	m_Albedo2.Init((texturefolder + config.Albedo2Texture).c_str(), TEXTURE_2D);
	m_Albedo3.Init((texturefolder + config.Albedo3Texture).c_str(), TEXTURE_2D);
	m_Albedo4.Init((texturefolder + config.Albedo4Texture).c_str(), TEXTURE_2D);
	m_BlendMap.Init((texturefolder + config.BlendmapTexture).c_str(),TEXTURE_2D);
	BuildVertexBuffer( config );
	GenerateNormalMap(128,glm::vec2(m_WorldDimensionX,m_WorldDimensionZ));
	return true;
}


float gfx::Terrain::GetHeightAtWorldCoord( const float x, const float z ) const
{
	if (x >= (m_WorldDimensionX - 1) || z >= (m_WorldDimensionZ - 1) || x <= 0 || z <= 0)
	{
		return 0;
	}

	int posX, posZ;
	float quadX, quadZ;
	posX = static_cast<int>(x / m_WorldDimensionX * m_ImageWidth);
	posZ = static_cast<int>(z / m_WorldDimensionZ * m_ImageHeight);

	quadX = (x / m_WorldDimensionX * m_ImageWidth) - posX;
	quadZ = (z / m_WorldDimensionZ * m_ImageHeight) - posZ;
	float h1, h2, h3, h4;
	h1 = m_HeightPoints[posX + posZ * m_ImageWidth] * (1.0f - quadX) * (1.0f - quadZ);
	h2 = m_HeightPoints[(posX + 1) + posZ * m_ImageWidth ] * (quadX) * (1.0f - quadZ);
	h3 = m_HeightPoints[posX + (posZ + 1) * m_ImageWidth] * (1.0f - quadX) * (quadZ);
	h4 = m_HeightPoints[(posX + 1) + (posZ + 1) * m_ImageWidth] * (quadX)* (quadZ);

	float height = ((h1 + h2 + h3 + h4) * m_MapScale);
	return height - (m_MapScale * 0.5f);
}

std::pair<int, int> gfx::Terrain::GetMapDimensions() const
{
	return std::pair<int, int>( m_WorldDimensionX, m_WorldDimensionZ );
}

void gfx::Terrain::SetWireframeMode( bool active )
{
	m_UseWireframe = active;
}

void gfx::Terrain::Cleanup() 
{
	if(m_HeightPoints)
		pDeleteArray(m_HeightPoints);
}

bool gfx::Terrain::LoadHeightpointsFromFile( const rString& filename, const gfx::Terrain::Config& config )
{
	// Register for logger
	Logger::RegisterType( "Graphics" );

	int imgChannels = 0;				// Soil wants an int* for channels
	const int imgForceChannels = 1;		// But a regular int for force_channels...

	// Make a lambda expression because I need this code twice (But really I just want to try lamdba expressions!)
	auto printLoadError = []( const rString& filename )
	{
		rStringStream msg;
		msg << "Failed to load file: " << filename << std::endl;
		msg << "File: " << __FILE__ << std::endl << "Line: " << __LINE__ << std::endl;
		Logger::Log( msg.str(), "Graphics", LogSeverity::ERROR_MSG );
	};

	// Load image into memory
	unsigned char* image = SOIL_load_image( filename.c_str(), &m_ImageWidth, &m_ImageHeight, &imgChannels, SOIL_LOAD_L);
	if ( image == nullptr )
	{
		printLoadError( filename );
		return false;
	}

	m_HeightPoints = pNewArray(float, m_ImageWidth * m_ImageHeight);
	float max = std::numeric_limits<float>::min();
	float min = std::numeric_limits<float>::max();
	for (int i = 0; i < m_ImageWidth * m_ImageHeight; i++)
	{
		m_HeightPoints[i] = image[i] / 255.0f;
		max = glm::max(max ,m_HeightPoints[i]);
		min = glm::min(min, m_HeightPoints[i]);
	}
	m_MaxHeight = max;
	m_MinHeight = min;
	//filter
	auto GetNearby = [this](int x, int y)
	{
		int count = 0;
		float avg = 0;
		for (int k = y - 1; k < y + 1; ++k)
		{
			for (int l = x - 1; l < x + 1; ++l)
			{
				if (!(k < 0 || l < 0 || k >= static_cast<int>(m_ImageWidth) || l >= static_cast<int>(m_ImageHeight)))
				{
					avg += m_HeightPoints[k * m_ImageWidth + l];
					count++;
				}

			}
		}
		return avg / count;
	};

	//float* newData = new float[m_ImageWidth * m_ImageHeight];
	//int filterAmount = 0;
	//for (int i = 0; i < filterAmount; ++i)
	//{
	//	for (int y = 0; y < m_ImageHeight; ++y)
	//	{
	//		for (int x = 0; x < m_ImageWidth; ++x)
	//		{
	//			newData[y * m_ImageWidth + x] = GetNearby(x, y);
	//		}
	//	}
	//	memcpy(m_HeightPoints, newData, sizeof(float) * m_ImageWidth * m_ImageHeight);
	//}
	//delete[] newData;

	// And into OpenGL
	glGenTextures(1, &m_OGLHeightTexture);
	glBindTexture(GL_TEXTURE_2D, m_OGLHeightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_ImageWidth, m_ImageHeight, 0, GL_RED, GL_FLOAT, m_HeightPoints);

	SOIL_free_image_data( image );

	return true;
}


void gfx::Terrain::BuildVertexBuffer( const gfx::Terrain::Config& config )
{
	// Split the entire map dimensions into the desired amount of quads
	float quadWidth = config.MapDimensionX / (float)(config.VertexGranularityX - 1);
	float quadHeight = config.MapDimensionZ / (float)(config.VertexGranularityZ - 1);

	VertexPosNormalTexTexel terrVertex;

	// Start coordinates for the world
	float startX = 0;
	float startZ = 0;

	// Steps for UV-coordinates. We want them to range from 0 to 1, so we divide 1 by the amount of vertices in each dimension.
	float stepX = 1.0f / ((float)config.VertexGranularityX - 1.0f);
	float stepZ = 1.0f / ((float)config.VertexGranularityZ - 1.0f);

	float finalX = 0.0f;
	// Loop over each vertex point
	for ( int z = 0; z < config.VertexGranularityZ; z++ )
	{
		for ( int x = 0; x < config.VertexGranularityX; x++ )
		{
			finalX = stepX * x;

			float worldX = startX + quadWidth * x;
			float worldZ = startZ + quadHeight * z;

			terrVertex.Position = glm::vec3( startX + quadWidth * x, 0, startZ + quadHeight * z );
			terrVertex.Normal = glm::vec3( 0, 1, 0 );
			terrVertex.TexCoord = glm::vec2( stepX * x, stepZ * z );
			std::pair<float,float> imgCoords = WorldToImageCoordinates( worldX, worldZ );
			terrVertex.TexelCoord = glm::vec2( imgCoords.first, imgCoords.second );
			m_TerrVertices.push_back( terrVertex );
		}
	}

	// N * M quads means 6 * N * M indices //screw that we are using quads now!
	int sizeX = config.VertexGranularityX;
	for ( int z = 0; z < config.VertexGranularityZ-1; z++ )
	{
		for ( int x = 0; x < config.VertexGranularityX-1; x++ )
		{

			m_TerrIndices.push_back( (z) * sizeX + (x) );
			m_TerrIndices.push_back( (z + 1) * sizeX + (x) );
			m_TerrIndices.push_back( (z) * sizeX + (x + 1) );
			m_TerrIndices.push_back( (z + 1) * sizeX + (x + 1) );
		}
	}
	m_IndexBufferSize = static_cast<unsigned int>(m_TerrIndices.size());

	m_VertexBuffer.Init( POS_NORMAL_TEX_TEXEL, m_TerrVertices.data(), static_cast<unsigned int>(m_TerrVertices.size() * sizeof( VertexPosNormalTexTexel )), 0 );
	glGenBuffers( 1, &m_IndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_TerrIndices.size() * sizeof( unsigned int ), m_TerrIndices.data(), GL_STATIC_DRAW );

	//init shaders
	m_RenderProgram = g_ShaderBank.LoadShaderProgram( "../../../shader/Tessellation.glsl" );
	m_DeferredProgram = g_ShaderBank.LoadShaderProgram("../../../shader/DeferredTessellation.glsl");

	g_BufferManager.BindBufferToProgram("SceneInputs",g_ShaderBank.GetProgramFromHandle(m_RenderProgram),0);
	g_BufferManager.BindBufferToProgram("LightIndexBuffer", g_ShaderBank.GetProgramFromHandle(m_RenderProgram), 5);

	m_DepthProgram = g_ShaderBank.LoadShaderProgram("../../../shader/TessellationDepth.glsl");
	g_BufferManager.BindBufferToProgram("SceneInputs", g_ShaderBank.GetProgramFromHandle(m_DepthProgram), 0);

	// Clear the vertex vector
	m_TerrVertices.clear();
	m_TerrVertices.shrink_to_fit();
	m_TerrIndices.clear();
	m_TerrIndices.shrink_to_fit();
}

float gfx::Terrain::GetHeightAtImageCoord( const float x, const float z ) const
{
	int xLow = (int)x;
	int xHigh = std::min( xLow + 1, m_ImageWidth );
	int zLow = (int)z;
	int zHigh = std::min( zLow + 1, m_ImageHeight );
	float diffX = x - xLow;
	float diffZ = z - zLow;

	if ( xHigh >= m_ImageWidth || zHigh >= m_ImageHeight || xLow < 0 || zLow < 0 )
	{
		return 0;
	}
	float bl = m_HeightPoints[ xLow + zLow * m_ImageWidth ];
	float br = m_HeightPoints[xHigh + zLow * m_ImageWidth ];
	float tl = m_HeightPoints[xLow + zHigh * m_ImageWidth ];
	float tr = m_HeightPoints[xHigh + zHigh * m_ImageWidth ];

	float avgZLow = bl * (1.0f - diffX) + br * diffX;
	float avgZHigh = tl * (1.0f - diffX) + tr * diffX;
	float avg = avgZLow * (1.0f - diffZ) + avgZHigh * diffZ;

	return avg;
}

std::pair<float, float> gfx::Terrain::WorldToImageCoordinates( const float& worldX, const float& worldZ ) const
{
	return std::pair<float, float>( (worldX - m_Position.x)  * m_WorldToImageCoordX, (worldZ - m_Position.z)  * m_WorldToImageCoordZ );
}

void gfx::Terrain::Draw(const glm::mat4& Lightmat, GLuint ShadowMap )
{
	glm::vec3 color = { 0.5f, 0.5f, 0.5f }; // r, g, b, strength
	glm::mat4 world = glm::translate( m_Position ) * glm::scale( glm::vec3( 1 ) );
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_RenderProgram);
	prog->Apply();
	prog->SetUniformMat4( "world", world );
	prog->SetUniformVec3( "color", color );
	prog->SetUniformInt("numPLights", g_LightEngine.GetPointLightCount());
	prog->SetUniformInt("numDLights", g_LightEngine.GetDirLightCount());
	prog->SetUniformFloat( "heightScale", m_MapScale );
	prog->SetUniformFloat( "horizontalScale", m_HorizontalScale );
	prog->SetUniformVec4( "tessFactor", glm::vec4( static_cast<float>(m_TessellationFactor) ) );
	prog->SetUniformBool( "hasTexture", m_HasTexture );
	prog->SetUniformInt( "textureRepeat", m_TextureRepeat );
	prog->SetUniformInt( "heightmapRepeat", m_HeightmapRepeat );
	prog->SetUniformVec2( "g_WorldSize", m_WorldSize);
	prog->SetUniformInt( "mapX", m_WorldDimensionX );
	prog->SetUniformInt( "mapZ", m_WorldDimensionZ );
	prog->SetUniformMat4( "gShadowMat", Lightmat );

	m_VertexBuffer.Apply();
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );

	// bind heightmap
	GLuint loc = prog->FetchUniform( "heightMap" );
	glUniform1i( loc, 0 );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_OGLHeightTexture );

	// bind normalmap
	loc = prog->FetchUniform( "normalMap" );
	glUniform1i( loc, 1 );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, m_OGLNormalTexture );

	// bind texture
	if ( m_HasTexture )
	{
		loc = prog->FetchUniform( "testTex" );
		glUniform1i( loc, 2 );
		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D, m_OGLTexture );
	}

	loc = prog->FetchUniform( "roughTex" );
	m_OGLRoughTexture.Apply(loc,3);

	loc = prog->FetchUniform( "FogOfWarTex" );
	glUniform1i( loc, 4 );
	glActiveTexture( GL_TEXTURE4 );
	glBindTexture( GL_TEXTURE_2D, m_FogOfWarTex );

	loc = prog->FetchUniform( "g_ShadowMap" );
	glUniform1i( loc, 5 );
	glActiveTexture( GL_TEXTURE5 );
	glBindTexture( GL_TEXTURE_2D, ShadowMap );

	loc = prog->FetchUniform( "g_LightCubeTex" );
	m_Sky.Apply(loc,7);
	loc = prog->FetchUniform( "g_IrradianceCube" );
	m_Irradiance.Apply(loc,8);

	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("Terrain", true) );
		glDrawElements( GL_PATCHES, m_IndexBufferSize, GL_UNSIGNED_INT, (GLvoid*)0 );
		GPU_PROFILE( gpuProfile.Stop() );
	}

	if ( m_UseWireframe )
	{
		prog->SetUniformBool( "hasTexture", false );
		glPolygonMode( GL_FRONT, GL_LINE );
		{
			GPU_PROFILE( AutoGPUProfiler gpuProfile("TerrainWireFrame", true) );
			glDrawElements( GL_PATCHES, m_IndexBufferSize, GL_UNSIGNED_INT, (GLvoid*)0 );
			GPU_PROFILE( gpuProfile.Stop() );
		}
		glPolygonMode( GL_FRONT, GL_FILL );
	}
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void gfx::Terrain::DrawDepth()
{
	glm::mat4 world = glm::translate(m_Position) * glm::scale(glm::vec3(1));
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_RenderProgram);
	prog->Apply();
	prog->SetUniformMat4("world", world);
	prog->SetUniformFloat("heightScale", m_MapScale);
	prog->SetUniformFloat("horizontalScale", m_HorizontalScale);
	prog->SetUniformVec4("tessFactor", glm::vec4(static_cast<float>(m_TessellationFactor)));
	prog->SetUniformInt("heightmapRepeat", m_HeightmapRepeat);

	m_VertexBuffer.Apply();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);

	// bind heightmap
	GLuint loc = prog->FetchUniform("heightMap");
	glUniform1i(loc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_OGLHeightTexture);

	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("TerrainDepth", true) );
		glDrawElements(GL_PATCHES, m_IndexBufferSize, GL_UNSIGNED_INT, (GLvoid*)0);
		GPU_PROFILE( gpuProfile.Stop() );
	}
	glUseProgram(0);
}

void gfx::Terrain::SetFogOfWarInfo( const GLuint& textureHandle, const glm::vec2& worldSize )
{
	m_FogOfWarTex = textureHandle;
	m_WorldSize = worldSize;
}

void gfx::Terrain::DrawDeferred( const glm::mat4& Lightmat, GLuint ShadowMap )
{
	glm::vec3 color = { 0.5f, 0.5f, 0.5f }; // r, g, b, strength
	glm::mat4 world = glm::translate( m_Position ) * glm::scale( glm::vec3( 1 ) );
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_DeferredProgram);
	prog->Apply();
	prog->SetUniformMat4( "world", world );
	prog->SetUniformVec3( "color", color );
	prog->SetUniformFloat( "heightScale", m_MapScale);
	prog->SetUniformFloat( "horizontalScale", m_HorizontalScale );
	prog->SetUniformVec4( "tessFactor", glm::vec4( static_cast<float>(m_TessellationFactor) ) );
	prog->SetUniformBool( "hasTexture", m_HasTexture );
	prog->SetUniformInt( "textureRepeat", m_TextureRepeat );
	prog->SetUniformInt( "heightmapRepeat", m_HeightmapRepeat );
	prog->SetUniformInt( "mapX", m_WorldDimensionX );
	prog->SetUniformInt( "mapZ", m_WorldDimensionZ );

	m_VertexBuffer.Apply();
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );

	// bind heightmap
	GLuint loc = prog->FetchUniform( "heightMap" );
	glUniform1i( loc, 0 );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_OGLHeightTexture );

	// bind normalmap
	loc = prog->FetchUniform( "normalMap" );
	glUniform1i( loc, 1 );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, m_OGLNormalTexture );

	// bind texture
	if ( m_HasTexture )
	{
		m_Albedo1.Apply(prog->FetchUniform("albedo1"), 2);
		m_Albedo2.Apply(prog->FetchUniform("albedo2"), 7);
		m_Albedo3.Apply(prog->FetchUniform("albedo3"), 8);
		m_Albedo4.Apply(prog->FetchUniform("albedo4"), 9);
		m_BlendMap.Apply(prog->FetchUniform("Blendmap"),10);
	}

	loc = prog->FetchUniform( "roughTex" );
	m_OGLRoughTexture.Apply(loc,3);

	prog->SetUniformTextureHandle( "g_ShadowMap" ,ShadowMap, 4);
	prog->SetUniformMat4("gShadowMat", Lightmat);

	m_Sky.Apply(prog->FetchUniform("g_LightCubeTex"),5);
	m_Irradiance.Apply(prog->FetchUniform("g_IrradianceCube"),6);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	GPU_PROFILE( AutoGPUProfiler gpuProfile("TerrainDeferred", true) );
	glDrawElements( GL_PATCHES, m_IndexBufferSize, GL_UNSIGNED_INT, (GLvoid*)0 );
	GPU_PROFILE( gpuProfile.Stop() );
	if ( m_UseWireframe )
	{
		prog->SetUniformBool( "hasTexture", false );
		glPolygonMode( GL_FRONT, GL_LINE );
		glDrawElements( GL_PATCHES, m_IndexBufferSize, GL_UNSIGNED_INT, (GLvoid*)0 );
		glPolygonMode( GL_FRONT, GL_FILL );
	}
	glUseProgram(0);
}

void gfx::Terrain::GenerateNormalMap(int resolution, glm::vec2 worldSize)
{
	float delta = 1.0f / resolution;
	float worldDeltaX = worldSize.x * delta;
	float worldDeltaY = worldSize.y * delta;

	glm::vec3 normal, p1,p2,p3,s1,s2;
	glm::vec3* normalMap = fNewArray(glm::vec3, resolution * resolution);

	for(int y = 0; y < resolution; y++)
	{
		for ( int x = 0; x < resolution; x++ )
		{
			p1 = glm::vec3( worldDeltaX * x, 0.0, worldDeltaY * y );
			p2 = glm::vec3( worldDeltaX * (x + 1), 0.0, worldDeltaY * y );
			p3 = glm::vec3( worldDeltaX * x, 0.0, worldDeltaY * (y + 1) );

			p1.y = GetHeightAtWorldCoord( p1.x, p1.z );
			p2.y = GetHeightAtWorldCoord( p2.x, p2.z );
			p3.y = GetHeightAtWorldCoord( p3.x, p3.z );

			s1 = p3 - p1;
			s2 = p3 - p2;

			normal = glm::cross( s2, s1 );
			normalMap[y * resolution + x] = glm::normalize(normal);
		}
	}
	if(m_OGLNormalTexture > 0) glDeleteTextures(1, &m_OGLNormalTexture);

	glGenTextures(1,&m_OGLNormalTexture);
	glBindTexture(GL_TEXTURE_2D, m_OGLNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT,normalMap);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	fDeleteArray(normalMap);

}
