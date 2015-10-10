/**************************************************
Zlib Copyright 2015 Mostafa Hassan & Richard Pettersson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "gfx/Vertex.h"
#include "gfx/Model.h"
#include "Animation.h"

#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

// #define ALL_IS_ANIMATED
#define LOAD_BINARY_ROBO

namespace gfx
{
	struct PosNormTexTan
	{
		PosNormTexTan()
		{
		}

		PosNormTexTan( float* pos , float* norm , float* uv , float* tan )
		{
			memcpy( Position , pos , sizeof( Position ) );
			memcpy( Normal , norm , sizeof( Normal ) );
			memcpy( Tangent , tan , sizeof( Tangent ) );
			memcpy( TexCoord , uv , sizeof( TexCoord ) );
		}

		float Position[ 3 ];
		float Normal[ 3 ];
		float TexCoord[ 2 ];
		float Tangent[ 3 ];
	};

	struct RoboCamera
	{
		rString Name;
		bool IsOrtho;
		float EyePoint[ 3 ];
		float UpDirection[ 3 ];
		float RightDirection[ 3 ];
		float ViewDirection[ 3 ];
		float AspectRatio;
		float NearClipping;
		float FarClipping;
		float VerticalFieldOfView;
	};

	struct RoboLight
	{
		rString Name; 
		float Position[ 3 ];

		float DirectionLength;
		float Direction[ 3 ];

		float Ambient[ 4 ];
		float Diffuse[ 4 ];
		float Color[ 4 ];
		float Specular[ 4 ];
		float Intensity;
		bool UseRayTraceShadows;
	};

	struct RoboMat
	{
		float Ambient[ 4 ];
		float Color[ 4 ];
		float Specularity[ 4 ];
		float Incandescence[ 4 ];
		float Transperancy[ 4 ];

		bool HasAlbedoMap;
		bool HasRoughMap;
		bool HasBumpMap;
		bool HasMetalMap;
		bool HasGlowMap;

		rString Name;
		rString AlbedoTex;
		rString RoughTex;
		rString BumpTex;
		rString MetalTex;
		rString GlowTex;

		RoboMat()
		{
			Name = "";
			AlbedoTex = "";
			RoughTex = "";
			BumpTex = "";
			MetalTex = "";
			GlowTex = "";

			HasAlbedoMap = false;
			HasRoughMap = false;
			HasBumpMap = false;
			HasMetalMap = false;
			HasGlowMap = false;
		}
	};

	struct EntityObj
	{
	public:
		rString Name;
		rString LinkObj;
		//rString Type;
		rString parent;

		float Radius;
		float Translation[ 3 ];
		float Scale[ 3 ];
		float Rotation[ 4 ];

		EntityObj()
		{
			memset( Translation , 0 , sizeof( Translation ) );
			memset( Scale , 0 , sizeof( Scale ) );
			memset( Rotation , 0 , sizeof( Rotation ) );
			Radius = 0;
			DropRate = 0;
		}
		void SetDropRate( const int dropRate ){ DropRate = dropRate; }
		int GetDropRate()
		{
			if ( Radius > 0 )
			{
				return DropRate;
			}
			return -1;
		}
		bool HasDropRate(){ return Radius > 0.0f; }

	private:
		int DropRate;	// ONLY ENITIES WITH RADIUS MORE THAN ZERO CAN HAVE A DROPRATE!
	};

	//TODORP remove this temporary structure and move to another header
	struct Bone
	{
		rString Name;
		int parentIndex;
		VertexPos pos;
		float rotX, rotY, rotZ, rotW;
	};

	//TODORP remove this temporary structure and move to another header
	struct Skeleton
	{
		int index;
		rVector<Bone> bones;
	};

	//TODORP remove this temporary structure and move to another header
	struct SkeletonAnimation
	{
		rVector<Skeleton> skeletons;
	};

	//TODORP remove this temporary structure and move to another header
	struct BoneInfo
	{
		rString Name;
		int parentIndex;
		int boneFlag;
		int startIndex;
	};

	struct RoboScene
	{
		rString SceneName;
		rString BGMusic;

		rVector<EntityObj> SpawnPoints;
		rVector<EntityObj> ControlPoints;
		rVector<EntityObj> Props;
		rVector<EntityObj> Resources;
		rVector<EntityObj> Cameras;
		rVector<EntityObj> Lights;
		rVector<EntityObj> Locators;

		rVector<rString>   ResourceFiles;
		//rVector<rString>   PropsFiles;

		// Terrain Map stuff
		rString DiffuseTexture;
		rString HeightMap;
		rString AIMap;
		rString NormalMap;
		rString	Albedo1Texture;
		rString	Albedo2Texture;
		rString	Albedo3Texture;
		rString	Albedo4Texture;
		rString	BlendmapTexture;
		int Width;
		int Height;
		float HeightScale;

		RoboScene()
		{
			Width = 0;
			Height = 0;
			HeightScale = 0;
		}
	};

	class ObjectImporter
	{
	public:
		GFX_API ObjectImporter();
		GFX_API ~ObjectImporter();

		GFX_API void							Release();
		GFX_API bool							LoadObject( const rString fileName, Model& model );
		GFX_API bool							LoadScene( const rString fileName );
		GFX_API bool							LoadCamera( const rString fileName );
		GFX_API bool							LoadLight( const rString fileName );
		GFX_API bool							LoadAnimation( const rString filename, Animation& animation );
		GFX_API void						    RemoveScene(const rString& sceneName);
		GFX_API VertexPosNormalTexTangent*		GetVerticesStatic();
		GFX_API unsigned int*					GetIndicesStatic();
		GFX_API int								GetVerticesSizeStatic() const;
		GFX_API int								GetIndicesSizeStatic() const;

		GFX_API VertexPosNormalTexTangentJointsWeights*	GetVerticesAnimated();
		GFX_API unsigned int*					GetIndicesAnimated();
		GFX_API int								GetVerticesSizeAnimated() const;
		GFX_API int								GetIndicesSizeAnimated() const;

		// Scene has to be loaded in order to use these Get functions
		GFX_API int								GetNumOfScenes() const;

		GFX_API RoboLight*						GetLight( const int index ) const;
		GFX_API rVector<RoboLight*>				GetLights();

		GFX_API rVector<RoboMat>&				GetMaterials();

		GFX_API RoboCamera*						GetCamera( const int index ) const;
		GFX_API rVector<RoboCamera*>			GetCameras();

		GFX_API const bool						GetSceneByName( gfx::RoboScene& scenee, const rString& sceneName ) const;
		GFX_API const bool						GetSceneByIndex( gfx::RoboScene& scene, const int index ) const;
		GFX_API RoboScene*						GetScenes();

		GFX_API const rVector<EntityObj>&		GetControlPointsInScene( const int  sceneIndex )const;
		GFX_API const rVector<EntityObj>&		GetResourcesInScene( const int sceneIndex ) const;
		GFX_API const rVector<EntityObj>&		GetPropsInScene( const int sceneIndex ) const;
		GFX_API const rVector<EntityObj>&		GetCamerasInScene( const int sceneIndex ) const;
		GFX_API const rVector<EntityObj>&		GetLightsInScene( const int sceneIndex ) const;
		GFX_API const rVector<EntityObj>&		GetLocatorsInScene( const int sceneIndex )const;
		GFX_API const rVector<EntityObj>&		GetSpawnPointsInScene( const int sceneIndex )const;

		GFX_API const bool						GetControlPointsInScene( rVector<EntityObj>& cps, const rString& sceneName )const;
		GFX_API const bool						GetResourcesInScene( rVector<EntityObj>& resources, const rString& sceneName ) const;
		GFX_API const bool						GetPropsInScene( rVector<EntityObj>& props, const rString& sceneName ) const;
		GFX_API const bool						GetCamerasInScene( rVector<EntityObj>& cams, const rString& sceneName ) const;
		GFX_API const bool						GetLightsInScene( rVector<EntityObj>& lights, const rString& sceneName ) const;
		GFX_API const bool						GetLocatorsInScene( rVector<EntityObj>& loc, const rString& sceneName )const;
		GFX_API const bool						GetSpawnPointsInScene( rVector<EntityObj>& sp, const rString& sceneName )const;

		GFX_API const EntityObj					Get3DObject( const rString& name )const;
		GFX_API const bool						Get3DObjectFromScene( EntityObj& obj, const rString& name, const rString& sceneName )const;
		GFX_API const EntityObj					Get3DObjectFromSceneBySceneIndex( const rString& name, const int sceneIndex )const;
		GFX_API const EntityObj					Get3DObjectFromUnLoadedScene( const rString& sceneName, const rString& name );

		void PrintMemory();
	private:
		bool									LoadBindMesh( const rString fileName, unsigned int vertexSize, unsigned int indexSize, Model& model );
		bool									LoadBoneAnimation( const rString fileName, Animation& animation );
		bool									LoadStaticMesh( const rString fileName, unsigned int vertexSize, unsigned int indexSize, Model& model );
		bool									LoadBinaryStaticMesh( const rString fileName, unsigned int vertexSize, unsigned int indexSize, Model& model );
		bool									LoadMaterial( const rString fileName, Model& model );
		EntityObj&								ParseObjFromScene( std::ifstream& file, rString& type_str, EntityObj& obj );

		rVector<VertexPosNormalTexTangent>		m_VerticesStatic;
		rVector<unsigned int>					m_IndicesStatic;
		rVector<VertexPosNormalTexTangentJointsWeights>	m_VerticesAnimated;
		rVector<unsigned int>					m_IndicesAnimated;
		rVector<RoboMat>						m_Materials;
		rVector<RoboCamera*>					m_Cameras;
		rVector<RoboLight*>					    m_Lights;
		rVector<RoboScene>						m_Scenes;
	};
}