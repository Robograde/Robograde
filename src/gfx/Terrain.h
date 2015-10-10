/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen & Henrik Johansson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <GL/glew.h>
#include "gfx/Vertex.h"
#include "gfx/VertexBuffer.h"
#include "gfx/Shader/ShaderProgram.h"
#include "Shader/ShaderBank.h"

#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

namespace gfx
{
	static const int DEFAULT_HEIGHTMAP_MAX = 128;

	class Terrain
	{
	public:
		struct						Config;

		GFX_API Terrain();
		GFX_API ~Terrain();

		GFX_API bool				LoadFromFile( const rString& TerrainFilename, const Terrain::Config& config );
		GFX_API float				GetHeightAtWorldCoord( const float x, const float z ) const;
		GFX_API void				Draw(const glm::mat4& Lightmat, GLuint ShadowMap ); //TODOHJ: Make the input for the terrain better
		GFX_API void				DrawDepth();
		GFX_API void				DrawDeferred( const glm::mat4& Lightmat, GLuint ShadowMap );
		//GFX_API void				AddToHeightPoints(const rVector<float> grid, const Terrain::Config& config);

		// Plain Getters & Setters
		GFX_API std::pair<int, int> GetMapDimensions() const;
		GFX_API void				SetWireframeMode( bool active );
		GFX_API void				SetFogOfWarInfo( const GLuint& textureHandle, const glm::vec2& worldSize );
		GFX_API void 				Cleanup();

		GFX_API float				GetMaxHeight(){return m_MaxHeight * m_MapScale ;};
		GFX_API float				GetMinHeight(){return m_MinHeight * m_MapScale ;};
	private:
		bool					LoadHeightpointsFromFile( const rString& filename, const Terrain::Config& config );
		void					BuildVertexBuffer( const Terrain::Config& config);
		float					GetHeightAtImageCoord( const float x, const float z ) const;
		std::pair<float, float> WorldToImageCoordinates( const float& worldX, const float& worldZ ) const;
		void					FilterHeightMap(int filterpasses);
		void					GenerateNormalMap(int resolution, glm::vec2 worldSize);

		rVector<VertexPosNormalTexTexel> m_TerrVertices;
		rVector<unsigned int>		m_TerrIndices;

		VertexBuffer		m_VertexBuffer;
		GLuint				m_IndexBuffer;
		ShaderProgramHandle	m_RenderProgram;
		ShaderProgramHandle m_DepthProgram;
		ShaderProgramHandle m_DeferredProgram;
		float				m_MapScale;
		float				m_HorizontalScale;
		int					m_ImageWidth;
		int					m_ImageHeight;
		int					m_WorldDimensionX;
		int					m_WorldDimensionZ;
		float				m_MaxHeight;
		float				m_MinHeight;
		float*				m_HeightPoints;
		GLuint				m_OGLHeightTexture;
		GLuint				m_OGLNormalTexture;
		Texture				m_OGLRoughTexture;
		GLuint				m_OGLTexture;
		GLuint				m_FogOfWarTex;
		float				m_WorldToImageCoordX;
		float				m_WorldToImageCoordZ;
		unsigned int		m_IndexBufferSize;
		unsigned int		m_TessellationFactor;
		unsigned int		m_HeightmapRepeat;
		unsigned int		m_TextureRepeat;
		bool				m_UseWireframe;
		bool				m_HasTexture;
		glm::vec3			m_Position;
		Texture				m_Sky;
		Texture				m_Irradiance;

		Texture				m_Albedo1;
		Texture				m_Albedo2;
		Texture				m_Albedo3;
		Texture				m_Albedo4;
		Texture				m_BlendMap;
		glm::vec2			m_WorldSize;
	};

	struct Terrain::Config
	{
	public:
		int				ImageWidth = 0;			// Actual image width
		int				ImageHeight = 0;		// Actual image height
		int				Channels = 4;			// Amount of image channels (R/G/B/A)
		int				VertexGranularityX = 1; // Amount of quads along the X-axis
		int				VertexGranularityZ = 1; // Amount of quads along the Z-axis
		float			HeightScale = 1.0f;		// Linear scaling factor for map height
		int				MapDimensionX = 0;		// In-game length along the X-axis
		int				MapDimensionZ = 0;		// in-game length along the Z-axis
		int				TessellationFactor = 1; // Tessellation factor used on GPU
		unsigned int	TextureRepeat = 1;
		unsigned int	HeightmapRepeat = 1;
		glm::vec3		Position = { 0, 0, 0 };
		rString			Heightmaptexture;
		rString			Albedo1Texture;
		rString			Albedo2Texture;
		rString			Albedo3Texture;
		rString			Albedo4Texture;
		rString			BlendmapTexture;
	};
}
