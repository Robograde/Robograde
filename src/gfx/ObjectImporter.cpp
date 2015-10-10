/**************************************************
Zlib Copyright 2015 Mostafa Hassan & Richard Pettersson
***************************************************/

#include "ObjectImporter.h"
#include <fstream>
#include <utility/Logger.h>
#include <glm/gtc/quaternion.hpp>
#include "Animation.h"
#include "../core/ai/stdafx.h"
#include "gfx/VertexBuffer.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> 


gfx::ObjectImporter::ObjectImporter()
{
}


gfx::ObjectImporter::~ObjectImporter( )
{
	Release();
}

void gfx::ObjectImporter::Release()
{
	// Release allocated data here

	// Scene
	int numScenes = static_cast< int >( m_Scenes.size() );
	for ( int i = 0; i < numScenes; i++ )
	{
		m_Scenes[ i ].ControlPoints.clear();
		m_Scenes[ i ].Resources.clear();
		m_Scenes[ i ].Props.clear();
		m_Scenes[ i ].SpawnPoints.clear();
		m_Scenes[ i ].Cameras.clear();
		m_Scenes[ i ].Lights.clear();
		m_Scenes[ i ].Locators.clear();
	}

	// Cameras
	int numCameras = static_cast< int > ( m_Cameras.size() );
	for ( int i = 0; i < numCameras; i++ )
	{
		tDelete( m_Cameras[ i ] );
		m_Cameras[ i ] = nullptr;
	}

	// Lights
	int numLights = static_cast< int > ( m_Lights.size( ) );
	for ( int i = 0; i < numLights; i++ )
	{
		tDelete( m_Lights[ i ] );
		m_Lights[ i ] = nullptr;
	}

	m_IndicesStatic.clear();
	m_IndicesAnimated.clear();
	m_VerticesStatic.clear();
	m_VerticesAnimated.clear();
}

/*
*	This function will need a fileName (not the whole path!)
*	After checking what type of object this is, 
*	-	it will load another function to actually load the object
*	The path for models is: "../../../asset/model/"
*	The path for materials is: "../../../asset/material/"
*/
bool gfx::ObjectImporter::LoadObject( const rString fileName, Model& model )
{
	//const rString path = fileName;

	int extensionStart = static_cast< int > ( fileName.find( "." ) );
	rString extension = fileName.substr( extensionStart + 1, strlen( fileName.c_str() ) );

	bool status = false;
	if( strcmp( extension.c_str(), "robo" ) == 0 )
	{
#ifdef ALL_IS_ANIMATED
		if( LOAD_BINARY_ROBO == true )
		{
			status = LoadBinaryStaticMesh( /*path +*/ fileName,
				static_cast< unsigned int > (m_VerticesAnimated.size()),
				static_cast< unsigned int > (m_IndicesAnimated.size()), model );
			model.Name = fileName;
			model.Type = POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS;
		}
		else
		{
			status = LoadStaticMesh( /*path +*/ fileName,
				static_cast< unsigned int > ( m_VerticesAnimated.size() ),
				static_cast< unsigned int > ( m_IndicesAnimated.size() ), model );
			model.Name = fileName;
			model.Type = POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS;
		}
#else
#ifdef LOAD_BINARY_ROBO
			status = LoadBinaryStaticMesh( /*path +*/ fileName,
				static_cast< unsigned int > ( m_VerticesStatic.size() ),
				static_cast< unsigned int > ( m_IndicesStatic.size() ), model );
			model.Name = fileName;
			model.Type = POS_NORMAL_TEX_TANGENT;
#else
			status = LoadStaticMesh( /*path +*/ fileName,
				static_cast< unsigned int > ( m_VerticesStatic.size() ),
				static_cast< unsigned int > ( m_IndicesStatic.size() ), model );
			model.Name = fileName;
			model.Type = POS_NORMAL_TEX_TANGENT;
#endif
#endif	
	}

	else if( strcmp( extension.c_str(), "matrob" ) == 0 )
	{
		status = LoadMaterial(/* path +*/ fileName, model );
	}

	else if( strcmp( extension.c_str(), "skelrob" ) == 0 )
	{
		status = LoadBindMesh(/* path + */fileName,
			static_cast< unsigned int > (m_VerticesAnimated.size()),
			static_cast< unsigned int > (m_IndicesAnimated.size()), model );
		model.Name = fileName;
		model.Type = POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS;
	}

	else if( strcmp( extension.c_str(), "roboanim" ) == 0 )
	{
		Animation anim;
		status = LoadBoneAnimation( fileName, anim );
	}

	return status;
}

bool gfx::ObjectImporter::LoadScene( const rString fileName )
{
	int numScenes = static_cast< int >( m_Scenes.size() );

	const int maxErrorCount = 100000;
	int errorCount = 0;

	char* ext = ".scene";
	rString pathScene = "../../../asset/maps/" + fileName + "/" + fileName + ext;

	//int extensionStart = static_cast< int > ( fileName.find( "." ) );
	//rString extension = fileName.substr( extensionStart + 1, strlen( fileName.c_str() ) );

	if( fileName != "" )
	{
		std::ifstream file;
		file.open( ( pathScene ).c_str() );

		if( !file.is_open() )
		{
			Logger::Log( ( fileName + " could not be opened!" ).c_str(), "ObjectImporter", LogSeverity::ERROR_MSG );
			return false;
		}

		m_Scenes.resize( numScenes + 1 );

		rString type_str;

		file >> type_str;
		while( !file.eof() )
		{
			file >> type_str;

			if( type_str == "#" || type_str == "s" )
			{
				// Ignoring comments for now
			}

			else if ( type_str == "BackGroundMusic" )
			{
				file >> m_Scenes[ numScenes ].BGMusic;
			}

			else if( type_str == "AlbedoMap1" )
			{
				// Diffuse texture
				file >> m_Scenes[ numScenes ].Albedo1Texture;
			}

			else if( type_str == "AlbedoMap2" )
			{
				// Diffuse texture
				file >> m_Scenes[ numScenes ].Albedo2Texture;
			}

			else if( type_str == "AlbedoMap3" )
			{
				// Diffuse texture
				file >> m_Scenes[ numScenes ].Albedo3Texture;
			}

			else if( type_str == "AlbedoMap4" )
			{
				// Diffuse texture
				file >> m_Scenes[ numScenes ].Albedo4Texture;
			}

			else if( type_str == "BlendMap" )
			{
				// Diffuse texture
				file >> m_Scenes[ numScenes ].BlendmapTexture;
			}

			else if( type_str == "HeightMap" )
			{
				// Height map 
				file >> m_Scenes[ numScenes ].HeightMap;
			}

			else if( type_str == "AIMap" )
			{
				// AI map 
				file >> m_Scenes[ numScenes ].AIMap;
			}

			else if( type_str == "NormalMap" )
			{
				// Normal map 
				file >> m_Scenes[ numScenes ].NormalMap;
			}

			else if( type_str == "HeightMapScale" )
			{
				// height scale
				file >> m_Scenes[ numScenes ].HeightScale;
			}

			else if( type_str == "Width" )
			{
				file >> m_Scenes[ numScenes ].Width;
			}

			else if( type_str == "Height" )
			{
				file >> m_Scenes[ numScenes ].Height;
			}

			else if( type_str == "SpawnPoints" )
			{
				int amountOfSpawns = 0;
				file >> amountOfSpawns;
				EntityObj obj;
				m_Scenes[ numScenes ].SpawnPoints.resize( amountOfSpawns );

				for( int i = 0; i < amountOfSpawns; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].SpawnPoints[ i ] = obj;
				}
			}

			else if( type_str == "ControlPoints" )
			{
				// Control point count
				int amountOfCPs = 0;
				file >> amountOfCPs;
				EntityObj obj;

				m_Scenes[ numScenes ].ControlPoints.resize( amountOfCPs );
				for( int i = 0; i < amountOfCPs; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].ControlPoints[ i ] = obj;
				}
			}

			else if( type_str == "Resources" )
			{
				// Resource count
				int amountOfCPs = 0;
				file >> amountOfCPs;
				EntityObj obj;

				m_Scenes[ numScenes ].Resources.resize( amountOfCPs );
				for( int i = 0; i < amountOfCPs; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].Resources[ i ] = obj;
				}
			}

			else if( type_str == "Props" )
			{
				// Prop count
				int amountOfCPs = 0;
				file >> amountOfCPs;
				EntityObj obj;

				m_Scenes[ numScenes ].Props.resize( amountOfCPs );
				for( int i = 0; i < amountOfCPs; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].Props[ i ] = obj;
				}
			}

			else if( type_str == "Cameras" )
			{
				// Mesh count
				int amountOfObjs = 0;
				file >> amountOfObjs;
				EntityObj obj;

				m_Scenes[ numScenes ].Cameras.resize( amountOfObjs );
				for( int i = 0; i < amountOfObjs; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].Cameras[ i ] = obj;
				}
			}

			else if( type_str == "Lights" )
			{
				// Mesh count
				int amountOfObjs = 0;
				file >> amountOfObjs;
				EntityObj obj;

				m_Scenes[ numScenes ].Lights.resize( amountOfObjs );
				for( int i = 0; i < amountOfObjs; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].Lights[ i ] = obj;
				}
			}

			else if( type_str == "Locators" )
			{
				// Mesh count
				int amountOfObjs = 0;
				file >> amountOfObjs;
				EntityObj obj;

				m_Scenes[ numScenes ].Locators.resize( amountOfObjs );
				for( int i = 0; i < amountOfObjs; i++ )
				{
					obj = ParseObjFromScene( file, type_str, obj );
					m_Scenes[ numScenes ].Locators[ i ] = obj;
				}
			}

			else if ( type_str == "ResourceFiles" )
			{
				int resourceFiles = 0;
				file >> resourceFiles;
				for ( int i = 0; i < resourceFiles; i++ )
				{
					rString fileName;
					file >> fileName;
					m_Scenes[ numScenes ].ResourceFiles.push_back( fileName );
				}
			}
			/*
			else if ( type_str == "PropsFiles" )
			{
				int propsFiles = 0;
				file >> propsFiles;
				for ( int i = 0; i < propsFiles; i++ )
				{
					rString fileName;
					file >> fileName;
					m_Scenes[ numScenes ].PropsFiles.push_back( fileName );
				}
			}
			*/

			else
				errorCount += 1;

			// The error count has become to high, this is only possible if stuck in a loop
			if( errorCount > maxErrorCount )
			{
				Logger::Log( ( fileName + " could not be loaded!" ).c_str( ), "ObjectImporter", LogSeverity::ERROR_MSG );
				return false;
			}
		}

		m_Scenes[ numScenes ].SceneName = fileName;
		file.close();
	}

	else
	{
		Logger::Log( ( "Scene: " + fileName + " could not be found!" ).c_str(), "ObjectImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	return true;
}

bool gfx::ObjectImporter::LoadCamera( const rString fileName )
{
	int numCameras = static_cast< int >( m_Cameras.size() );

	rString pathScene = "../../../asset/camera/";

	int extensionStart = static_cast< int > ( fileName.find( "." ) );
	rString extension = fileName.substr( extensionStart + 1, strlen( fileName.c_str() ) );

	if( strcmp( extension.c_str(), "camrob" ) == 0 )
	{
		std::ifstream file;
		file.open( ( pathScene + fileName ).c_str() );

		if( !file.is_open() )
		{
			Logger::Log( ( fileName + " could not be opened!" ).c_str(), "ObjectImporter", LogSeverity::ERROR_MSG );
			return false;
		}

		m_Cameras.resize( numCameras + 1 );
		m_Cameras[ numCameras ] = tNew( RoboCamera );

		rString type_str;

		file >> type_str;
		while( !file.eof() )
		{
			file >> type_str;

			if( type_str == "#" || type_str == "s" )
			{
				// Ignoring comments for now
			}

			if( type_str == "Camera" )
			{
				// New camera detected
				rString name = "";
				file >> name;

				m_Cameras[ numCameras ]->Name = name;
			}

			if( type_str == "EyePoint" )
			{
				file >> m_Cameras[ numCameras ]->EyePoint[ 0 ]
					>> m_Cameras[ numCameras ]->EyePoint[ 1 ]
					>> m_Cameras[ numCameras ]->EyePoint[ 2 ];
			}

			if( type_str == "RightDirection" )
			{
				file >> m_Cameras[ numCameras ]->RightDirection[ 0 ]
					>> m_Cameras[ numCameras ]->RightDirection[ 1 ]
					>> m_Cameras[ numCameras ]->RightDirection[ 2 ];
			}

			if( type_str == "ViewDirection" )
			{
				file >> m_Cameras[ numCameras ]->ViewDirection[ 0 ]
					>> m_Cameras[ numCameras ]->ViewDirection[ 1 ]
					>> m_Cameras[ numCameras ]->ViewDirection[ 2 ];
			}

			if( type_str == "UpDirection" )
			{
				file >> m_Cameras[ numCameras ]->UpDirection[ 0 ]
					>> m_Cameras[ numCameras ]->UpDirection[ 1 ]
					>> m_Cameras[ numCameras ]->UpDirection[ 2 ];
			}

			if( type_str == "AspectRatio" )
				file >> m_Cameras[ numCameras ]->AspectRatio;

			if( type_str == "FarClipping" )
				file >> m_Cameras[ numCameras ]->FarClipping;

			if( type_str == "NearClipping" )
				file >> m_Cameras[ numCameras ]->NearClipping;

			if( type_str == "IsOrtho" )
			{
				int boolValue = 0;

				file >> boolValue;

				if( boolValue == 1 )
					m_Cameras[ numCameras ]->IsOrtho = true;

				else
					m_Cameras[ numCameras ]->IsOrtho = false;
			}

			if( type_str == "FOV" )
				file >> m_Cameras[ numCameras ]->VerticalFieldOfView;

		}

		file.close();
	}

	else
	{
		Logger::Log( ( "Camera: " + fileName + " could not be found!" ).c_str(), "ObjectImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	return true;
}

bool gfx::ObjectImporter::LoadLight( const rString fileName )
{
	int numLights = static_cast< int > ( m_Lights.size() );

	rString pathScene = "../../../asset/light/";

	int extensionStart = static_cast< int > ( fileName.find( "." ) );
	rString extension = fileName.substr( extensionStart + 1, strlen( fileName.c_str() ) );

	if( strcmp( extension.c_str(), "lightrob" ) == 0 )
	{
		std::ifstream file;
		file.open( ( pathScene + fileName ).c_str() );

		if( !file.is_open() )
		{
			Logger::Log( ( fileName + " could not be opened!" ).c_str(), "ObjectImporter", LogSeverity::ERROR_MSG );
			return false;
		}

		// File exists, new light
		m_Lights.resize( numLights + 1 );
		m_Lights[ numLights ] = tNew( RoboLight );

		rString type_str;

		file >> type_str;
		while( !file.eof() )
		{
			file >> type_str;

			if( type_str == "#" || type_str == "s" )
			{
				// Ignoring comments for now
			}

			if( type_str == "Light" )
			{
				// New light detected
				rString name = "";
				file >> name;

				m_Lights[ numLights ]->Name = name;
			}

			if( type_str == "Position" )
			{
				file >> m_Lights[ numLights ]->Position[ 0 ]
					>> m_Lights[ numLights ]->Position[ 1 ]
					>> m_Lights[ numLights ]->Position[ 2 ];
			}

			if( type_str == "DirectionLength" )
			{
				file >> m_Lights[ numLights ]->DirectionLength;
			}

			if( type_str == "Direction" )
			{
				file >> m_Lights[ numLights ]->Direction[ 0 ]
					>> m_Lights[ numLights ]->Direction[ 1 ]
					>> m_Lights[ numLights ]->Direction[ 2 ];
			}

			if( type_str == "Ambient" )
			{
				file >> m_Lights[ numLights ]->Ambient[ 0 ]
					>> m_Lights[ numLights ]->Ambient[ 1 ]
					>> m_Lights[ numLights ]->Ambient[ 2 ]
					>> m_Lights[ numLights ]->Ambient[ 3 ];
			}

			if( type_str == "Diffuse" )
			{
				file >> m_Lights[ numLights ]->Diffuse[ 0 ]
					>> m_Lights[ numLights ]->Diffuse[ 1 ]
					>> m_Lights[ numLights ]->Diffuse[ 2 ]
					>> m_Lights[ numLights ]->Diffuse[ 3 ];
			}

			if( type_str == "Color" )
			{
				file >> m_Lights[ numLights ]->Color[ 0 ]
					>> m_Lights[ numLights ]->Color[ 1 ]
					>> m_Lights[ numLights ]->Color[ 2 ]
					>> m_Lights[ numLights ]->Color[ 3 ];
			}

			if( type_str == "Specular" )
			{
				file >> m_Lights[ numLights ]->Specular[ 0 ]
					>> m_Lights[ numLights ]->Specular[ 1 ]
					>> m_Lights[ numLights ]->Specular[ 2 ]
					>> m_Lights[ numLights ]->Specular[ 3 ];
			}

			if( type_str == "UseRayTraceShadows" )
			{
				int UseRayTraceShadows = 0;
				file >> UseRayTraceShadows;

				if( UseRayTraceShadows == 1 )
					m_Lights[ numLights ]->UseRayTraceShadows = true;

				else
					m_Lights[ numLights ]->UseRayTraceShadows = false;
			}

			if( type_str == "Intensity" )
			{
				file >> m_Lights[ numLights ]->Intensity;
			}

		}

		file.close();
	}

	else
	{
		Logger::Log( ( "Camera: " + fileName + " could not be found!" ).c_str(), "ObjectImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	return true;
}


bool gfx::ObjectImporter::LoadAnimation( const rString filename, Animation& animation )
{
	return LoadBoneAnimation( filename, animation );
}

void gfx::ObjectImporter::RemoveScene(const rString& sceneName)
{
	for (int i = 0; i < m_Scenes.size(); i++)
	{
		if (strcmp(sceneName.c_str(), m_Scenes[i].SceneName.c_str()) == 0)
			m_Scenes.erase(m_Scenes.begin() + i);
	}
}


gfx::VertexPosNormalTexTangent* gfx::ObjectImporter::GetVerticesStatic()
{
	return m_VerticesStatic.data();
}

unsigned int* gfx::ObjectImporter::GetIndicesStatic()
{
	return m_IndicesStatic.data();
}

int gfx::ObjectImporter::GetVerticesSizeStatic() const
{
	return static_cast< int >(m_VerticesStatic.size());
}

int gfx::ObjectImporter::GetIndicesSizeStatic() const
{
	return static_cast< int >(m_IndicesStatic.size());
}

gfx::VertexPosNormalTexTangentJointsWeights* gfx::ObjectImporter::GetVerticesAnimated()
{
	return m_VerticesAnimated.data();
}

unsigned int* gfx::ObjectImporter::GetIndicesAnimated()
{
	return m_IndicesAnimated.data();
}

int gfx::ObjectImporter::GetVerticesSizeAnimated() const
{
	return static_cast< int >(m_VerticesAnimated.size());
}

int gfx::ObjectImporter::GetIndicesSizeAnimated() const
{
	return static_cast<int>(m_IndicesAnimated.size());
}

bool gfx::ObjectImporter::LoadBindMesh( const rString fileName, unsigned int vertexSize, unsigned int indexSize, Model& model )
{
	std::ifstream fin;
	char inString[ 128 ];
	int meshVertexSize = 0;

	rString pathModel = "../../../asset/model/" + fileName;

	fin.open( pathModel.c_str( ) );//Open File

	if( !fin.is_open( ) )
	{
		Logger::Log( ( pathModel + " could not be opened!" ).c_str( ), "ObjectImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	fin >> inString;
	while( !fin.eof( ) )
	{
		if( strcmp( inString, "totalJoints" ) == 0 )
		{
			int numJoints = 0;
			fin >> numJoints;//amount of joints in skeleton
			fin >> inString;//"bone"
			for( int i = 0; i < numJoints; i++ )
			{
				int parentID = -1;
				float x, y, z;
				fin >> inString;//joint name
				fin >> parentID;//parent joint index
				fin >> x >> y >> z;// (x, y, z) position
				fin >> x >> y >> z;// rotation (quaternion)
				fin >> inString;//"bone"
			}
		}

		if( strcmp( inString, "totalMeshes" ) == 0 )
		{
			int amount = 0;
			fin >> amount;//amount of submeshes
		}

		if( strcmp( inString, "mesh" ) == 0 )
		{
			int amount = 0;
			fin >> inString;//submesh name
			fin >> inString;//total verts
			fin >> amount;//amount of vertices in submesh

			m_VerticesAnimated.resize( vertexSize + amount );
			/*
			for ( int i = 0; i < amount; i++ ){
			VertexPosNormalTexTangent vertex;
			m_Vertices.push_back( vertex );
			}
			*/

			VertexPosNormalTexTangentJointsWeights vertex;

			fin >> inString;//"vert"
			for( int i = 0; i < amount; i++ )
			{
				int ID = -1;
				int numWeights = 0;
				float x, y, z;

				fin >> ID;//vertex ID
				ID += vertexSize;//Offset ID if vector is not empty
				fin >> x >> y >> z;//vertex position
				vertex.Position.x = x;	// TODOMW : Remove hardcoded *0.33f when models are rescaled
				vertex.Position.y = y;	// TODOMW : Remove hardcoded *0.33f when models are rescaled
				vertex.Position.z = z;	// TODOMW : Remove hardcoded *0.33f when models are rescaled
				fin >> x >> y >> z;//vertex normal
				vertex.Normal.x = x;
				vertex.Normal.y = y;
				vertex.Normal.z = z;
				fin >> x >> y >> z;//vertex tangent
				vertex.Tangent.x = x;
				vertex.Tangent.y = y;
				vertex.Tangent.z = z;
				fin >> x >> y;//vertex UV
				vertex.TexCoord.x = x;
				vertex.TexCoord.y = ( y );
				fin >> numWeights;//amount of weights
				vertex.Weights = { 0, 0, 0, 0 };
				vertex.JointIDs = { 0, 0, 0, 0 };
				for( int j = 0; j < numWeights && j < 4; j++ )
				{
					fin >> x;//parent joint index
					vertex.JointIDs[j] = (float)(int)x;
					fin >> x;//weight value
					vertex.Weights[j] = x;
				}
			
				m_VerticesAnimated[ID] = vertex;

				fin >> inString;//"vert"
			}
		}

		if( strcmp( inString, "totalMaterial" ) == 0 )
		{
			int x, y, z, numTriangles, numMaterial;

			fin >> numMaterial;//Amount of material assigned to faces on mesh
			for( int j = 0; j < numMaterial; j++ )
			{
				Mesh submesh;
				submesh.VertexBufferOffset = vertexSize;
				submesh.IndexBufferOffset = indexSize;
				fin >> inString;//"material"
				fin >> submesh.Material;// = materialName;
			    //fin >> inString;//material name
				fin >> inString;//"totalTris"

				fin >> numTriangles;//amount of triangles
				indexSize += ( numTriangles * 3 );
				submesh.Indices = ( numTriangles * 3 );
				//submesh.Size = static_cast< int >( m_Vertices.size() );
				submesh.Size = meshVertexSize;
				//submesh.Material = "";
				for( int i = 0; i < numTriangles; i++ )
				{
					fin >> inString;//"tri"
					fin >> x >> y >> z;//indices
					m_IndicesAnimated.push_back( x + vertexSize );//Vert 0
					m_IndicesAnimated.push_back( y + vertexSize );//Vert 1
					m_IndicesAnimated.push_back( z + vertexSize );//vert 2

				}
				model.Meshes.push_back( submesh );
			}
		}

		if( strcmp( inString, "BoundingBox" ) == 0 )
		{
			float x, y, z;

			fin >> x >> y >> z;//minimum value
			model.Min = glm::vec3( x , y , z );
			fin >> x >> y >> z;//maximum value
			model.Max = glm::vec3( x , y , z );
		}
		fin >> inString;
	}

	fin.close( );//Close File

	model.NumVertices = static_cast< int >(m_VerticesAnimated.size());
	model.NumIndices = static_cast< int >(m_IndicesAnimated.size());
	model.VertexHandle = static_cast< int >(m_VerticesAnimated.size());
	model.Name = fileName;
	model.Loaded = true;

	return true;
}

bool gfx::ObjectImporter::LoadBoneAnimation( const rString fileName, Animation& animation )
{
	std::ifstream fin;
	char inString[ 128 ];
	float* frameData;
	float* baseFrameData;
	rVector<BoneInfo> boneInfo;
	rVector<Skeleton> skeletonAnimation;
	int totalFrames, totalComponents;
	float frameRate = 0.0f;
	rString pathAnim = "../../../asset/animation/" + fileName;
	fin.open( pathAnim.c_str( ) );//Open File

	if( !fin.is_open( ) )
	{
		Logger::Log( ( fileName + " could not be opened!" ).c_str( ), "ObjectImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	fin >> inString;
	while( !fin.eof( ) )
	{
		if( strcmp( inString, "skeleton" ) == 0 )
		{
			int numBones, index, boneFlag, startIndex;
			float x, y, z;
			fin >> inString;//"totalBones"
			fin >> numBones;//Amount of bones in hiearchy

			for( int i = 0; i < numBones; i++ )
			{
				BoneInfo tempBoneInfo;
				fin >> tempBoneInfo.Name;//bone name
				fin >> index;//index of the bones parent
				tempBoneInfo.parentIndex = index;
				fin >> boneFlag;//Determines which component of the bone is being animated, in order to cut down size of file
				tempBoneInfo.boneFlag = boneFlag;
				fin >> startIndex;//Indicates where to start in the array of frame data
				tempBoneInfo.startIndex = startIndex;
				boneInfo.push_back( tempBoneInfo );
			}

			fin >> inString;
			//if ( strcmp( inString, "bindPose" ) == 0)
			//{
			//	for ( int i = 0; i < numBones; i++ )
			//	{
			//		fin >> inString; // bone
			//		fin >> inString; // jointX
			//		fin >> x; // parent
			//		fin >> x >> y >> z; // position
			//		glm::vec3 pos = glm::vec3( x, y, z );
			//		fin >> x >> y >> z; // rotation
			//		glm::quat rotQuat;
			//		rotQuat.x = x;
			//		rotQuat.y = y;
			//		rotQuat.z = z;

			//		float t = 1.0f - (rotQuat.x * rotQuat.x) - (rotQuat.y * rotQuat.y) - (rotQuat.z * rotQuat.z);
			//		if ( t < 0.0f )
			//		{
			//			rotQuat.w = 0.0f;
			//		}
			//		else
			//		{
			//			rotQuat.w = -sqrtf( t );
			//		}

			//		// Create world matrix
			//		glm::mat4 translation = glm::translate( glm::mat4( 1.0f ), pos );
			//		glm::mat4 rotation = glm::mat4_cast( rotQuat );
			//		//rVector<glm::mat4>& bindPoseMatrices = animation.GetEditableBindPoseList();
			//		//glm::mat4 world = translation * rotation;
			//		//bindPoseMatrices.push_back( world );
			//	}
			//	fin >> inString;
			//}
			// else inString == "totalFrames"
			fin >> totalFrames;//amount of frames
			fin >> inString;//"frameRate"
			fin >> frameRate;//FrameRate Data
			fin >> inString;//"totalComponents"
			fin >> totalComponents;//amount of components


			//frameData = new float[totalComponents];
			//baseFrameData = new float[totalComponents];

			frameData = fNewArray( float, totalComponents );
			baseFrameData = fNewArray( float, totalComponents );

			//totalComponents /= 6;


			for( int i = 0; i < totalFrames; i++ )
			{
				fin >> inString;//"frame"
				fin >> index;//time value

				if( i == 0 )
				{
					for( int j = 0; j < totalComponents; j += 6 )
					{
						fin >> x >> y >> z;//Position
						baseFrameData[ j ] = x;
						baseFrameData[ j + 1 ] = y;
						baseFrameData[ j + 2 ] = z;
						fin >> x >> y >> z;//Rotation (Quaternion)
						baseFrameData[ j + 3 ] = x;
						baseFrameData[ j + 4 ] = y;
						baseFrameData[ j + 5 ] = z;
						frameData[ j ] = baseFrameData[ j ];
						frameData[ j + 1 ] = baseFrameData[ j + 1 ];
						frameData[ j + 2 ] = baseFrameData[ j + 2 ];
						frameData[ j + 3 ] = baseFrameData[ j + 3 ];
						frameData[ j + 4 ] = baseFrameData[ j + 4 ];
						frameData[ j + 5 ] = baseFrameData[ j + 5 ];
					}
				}
				else
				{
					for( int j = 0; j < totalComponents; j += 6 )
					{
						fin >> x >> y >> z;//Position
						frameData[ j ] = x;
						frameData[ j + 1 ] = y;
						frameData[ j + 2 ] = z;
						fin >> x >> y >> z;//Rotation (Quaternion)
						frameData[ j + 3 ] = x;
						frameData[ j + 4 ] = y;
						frameData[ j + 5 ] = z;
					}
				}

				Skeleton tempSkeleton;
				tempSkeleton.index = index;

				for( int j = 0; j < numBones; j++ )
				{
					int k = 0;

					Bone tempBone;

					tempBone.Name = boneInfo[ j ].Name;
					tempBone.parentIndex = boneInfo[ j ].parentIndex;

					//Detemines whether a component is animated. if it isn't, use default value(First frame). 
					if( boneInfo[ j ].boneFlag & 1 )
					{
						tempBone.pos.Position.x = frameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					else
					{
						tempBone.pos.Position.x = baseFrameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					if( boneInfo[ j ].boneFlag & 2 )
					{
						tempBone.pos.Position.y = frameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					else
					{
						tempBone.pos.Position.y = baseFrameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					if( boneInfo[ j ].boneFlag & 4 )
					{
						tempBone.pos.Position.z = frameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					else
					{
						tempBone.pos.Position.z = baseFrameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					if( boneInfo[ j ].boneFlag & 8 )
					{
						tempBone.rotX = frameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					else
					{
						tempBone.rotX = baseFrameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					if( boneInfo[ j ].boneFlag & 16 )
					{
						tempBone.rotY = frameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					else
					{
						tempBone.rotY = baseFrameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					if( boneInfo[ j ].boneFlag & 32 )
					{
						tempBone.rotZ = frameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}
					else
					{
						tempBone.rotZ = baseFrameData[ boneInfo[ j ].startIndex + ( k++ ) ];
					}

					//Calculate w-component of rotation
					float t = 1.0f - ( tempBone.rotX * tempBone.rotX )
						- ( tempBone.rotY * tempBone.rotY )
						- ( tempBone.rotZ * tempBone.rotZ );
					if( t < 0.0f )
					{
						tempBone.rotW = 0.0f;
					}
					else
					{
						tempBone.rotW = -sqrtf( t );
					}

					//Calculate orientation of the bone in the skeleton
					if( tempBone.parentIndex >= 0 )
					{
						Bone tempParentBone = tempSkeleton.bones[ tempBone.parentIndex ];

						glm::quat parentBoneOrientation = glm::quat( tempParentBone.rotW, tempParentBone.rotX, tempParentBone.rotY, tempParentBone.rotZ );
						glm::quat childBoneOrientation = glm::quat( tempBone.rotW, tempBone.rotX, tempBone.rotY, tempBone.rotZ );

						glm::vec3 tempPos = glm::vec3( tempBone.pos.Position.x, tempBone.pos.Position.y, tempBone.pos.Position.z );
						glm::vec3 rotatedPosition = ( parentBoneOrientation * tempPos );

						tempBone.pos.Position.x = rotatedPosition.x + tempParentBone.pos.Position.x;
						tempBone.pos.Position.y = rotatedPosition.y + tempParentBone.pos.Position.y;
						tempBone.pos.Position.z = rotatedPosition.z + tempParentBone.pos.Position.z;

						childBoneOrientation = parentBoneOrientation * childBoneOrientation;

						glm::normalize( childBoneOrientation );

						tempBone.rotX = childBoneOrientation.x;//Imaginary
						tempBone.rotY = childBoneOrientation.y;//Imaginary
						tempBone.rotZ = childBoneOrientation.z;//Imaginary
						tempBone.rotW = childBoneOrientation.w;//real
					}

					tempSkeleton.bones.push_back( tempBone );
				}

				// Add skeleton to a list of skeletons which represent the animation of the skeleton at different frames
				if ( index == -1 )
				{

				}
				skeletonAnimation.push_back( tempSkeleton );
				tempSkeleton.bones.clear( );
			}
		}
		fin >> inString;
	}

	// Copy to SkeletonList

	rVector<SkeletonFrame>& frameList = animation.GetEditableSkeletonList( );
	SkeletonFrame bindPose;
	for( Skeleton& s : skeletonAnimation )
	{
		rVector<Joint> frame;
		for( Bone& b : s.bones )
		{
			frame.push_back( Joint( b.parentIndex, b.pos.Position, glm::vec4( b.rotX, b.rotY, b.rotZ, b.rotW ) ) );  // TODOMW : Remove hardcoded *0.33f when models are rescaled
		}
		if ( s.index == -1 )
		{
			bindPose.Skeleton = frame;
		}
		else
		{
			frameList.push_back( SkeletonFrame( frame, s.index ) );
		}
	}
	//animation.CreateBindPoseMatrices();
	animation.SetBindpose( bindPose );
	animation.SetDefaultFPS( frameRate );


	////////////
	//Clean up
	////////////
	if( frameData )
	{
		fDeleteArray( frameData );
		frameData = 0;
	}

	if( baseFrameData )
	{
		fDeleteArray( baseFrameData );
		baseFrameData = 0;
	}

	skeletonAnimation.clear( );
	boneInfo.clear( );

	fin.close( );

	return true;
}

/*
*	This function will push back meshes to the model.
*	Each material will become a new mesh which has indices
*/
#pragma region Load Robo Files

bool gfx::ObjectImporter::LoadStaticMesh( const rString fileName, unsigned int vertexSize, unsigned int indexSize, Model& model )
{
	const int maxErrorCount = 500;
	int errorCount = 0;

	int groupIndex = 0;
	int meshVertexCount = 0;
	rString pathModel = "../../../asset/model/" + fileName;
	std::ifstream file;
	file.open( pathModel.c_str( ) );

	if( !file.is_open( ) )
	{
		Logger::Log( ( pathModel + " could not be opened!" ).c_str( ), "ObjectImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	rString type_str;

	file >> type_str;
	while( !file.eof( ) )
	{
		file >> type_str;

		if( type_str == "#" || type_str == "s" )
		{
			// Ignoring comments for now
		}

		else if( strcmp( type_str.c_str( ), "Mesh" ) == 0 )
		{
			rString meshName = "";
			file >> meshName;

			//mesh = new MeshObject( );	// Create new mesh
			//mesh->Name = meshName;
		}

		else if( strcmp( type_str.c_str( ), "Vertices" ) == 0 )
		{
			int vertexCount = 0;
			file >> vertexCount;
			meshVertexCount = vertexCount;

			VertexPosNormalTexTangent vert;
#ifdef ALL_IS_ANIMATED
			VertexPosNormalTexTangentJointsWeights vertA;
#endif
			for( int i = 0; i < vertexCount; i++ )
			{
				file >> type_str; // v
				file >> vert.Position.x
					>> vert.Position.y
					>> vert.Position.z;

				file >> type_str; // vt
				file >> vert.TexCoord.x
					>> vert.TexCoord.y;

				file >> type_str; // vn
				file >> vert.Normal.x
					>> vert.Normal.y
					>> vert.Normal.z;

				file >> type_str; // tan
				file >> vert.Tangent.x
					>> vert.Tangent.y
					>> vert.Tangent.z;

#ifdef ALL_IS_ANIMATED
				vertA.Position = vert.Position;
				vertA.Normal = vert.Normal;
				vertA.TexCoord = vert.TexCoord;
				vertA.Tangent = vert.Tangent;
				vertA.JointIDs = glm::vec4( 0 );
				vertA.Weights = glm::vec4( 1, 0, 0, 0 );
				m_VerticesAnimated.push_back( vertA );
#else
				m_VerticesStatic.push_back( vert );
#endif
			}
		}

		else if( strcmp( type_str.c_str( ), "Groups" ) == 0 )
		{
			int groupCount = 0;
			file >> groupCount;
		}

		else if( strcmp( type_str.c_str( ), "Mat" ) == 0 )
		{
			Mesh mesh;
			mesh.VertexBufferOffset = vertexSize;
			mesh.IndexBufferOffset = indexSize;
			rString materialName = "";
			file >> materialName;

			int vertexCountInGroup = 0;

			file >> type_str;
			file >> vertexCountInGroup;

			for( int i = 0; i < vertexCountInGroup; i++ )
			{
				file >> type_str; // vIndex
				unsigned int index = 0;
				file >> index;

#ifdef ALL_IS_ANIMATED
				m_IndicesAnimated.push_back( index + vertexSize );
#else
				m_IndicesStatic.push_back( index + vertexSize );
#endif
			}

			indexSize += vertexCountInGroup;
			mesh.Indices = vertexCountInGroup;
			mesh.Size = meshVertexCount;
			mesh.Material = materialName;
			model.Meshes.push_back( mesh );
		}

		else if( strcmp( type_str.c_str( ), "Transformation" ) == 0 )
		{
			// Get transformation here
		}

		else if( strcmp( type_str.c_str( ), "BoundingBox" ) == 0 )
		{
			// Bounding box has min and max value (vec3s)
			float min[ 3 ];
			float max[ 3 ];

			file >> type_str >> min[ 0 ] >> min[ 1 ] >> min[ 2 ];	// Min
			file >> type_str >> max[ 0 ] >> max[ 1 ] >> max[ 2 ];	// Max
			model.Min = glm::vec3( min[ 0 ], min[ 1 ], min[ 2 ] );
			model.Max = glm::vec3( max[ 0 ], max[ 1 ], max[ 2 ] );
		}

		// If nothing of the above could be found then increase the error count
		else
			errorCount += 1;

		// The error count has become to high, this is only possible if stuck in a loop
		if( errorCount > maxErrorCount )
		{
			Logger::Log( ( pathModel + " could not be loaded!" ).c_str( ), "ObjectImporter", LogSeverity::ERROR_MSG );
			model.Loaded = false;
			return false;
		}
	}

	file.close( );

	// DEBUG

	//m_VerticesAnimated.clear();
	//m_IndicesAnimated.clear();

	model.NumVertices = static_cast< int >(m_VerticesStatic.size());
	model.NumIndices = static_cast< int >(m_IndicesStatic.size());
	model.VertexHandle = static_cast< int >(m_VerticesStatic.size());

#ifdef ALL_IS_ANIMATED
	model.NumVertices = static_cast< int >(m_VerticesAnimated.size());
	model.NumIndices = static_cast< int >(m_IndicesAnimated.size());
	model.VertexHandle = static_cast< int >(m_VerticesAnimated.size());
#endif

	model.Name = fileName;
	model.Loaded = true;

	return true;
}

#pragma endregion

#pragma region Load Binary Robo Files
bool gfx::ObjectImporter::LoadBinaryStaticMesh( const rString fileName, unsigned int vertexSize, unsigned int indexSize, Model& model )
{
	rString pathModel = "../../../asset/model/" + fileName;

	std::ifstream file;
	file.open( pathModel.c_str() , std::ios::in | std::ios::binary );
	if ( !file.is_open() )
	{
		Logger::Log( ( pathModel + " could not be opened!" ).c_str() , "ObjectImporter" , LogSeverity::ERROR_MSG );
		return false;
	}

	// Mesh Name
	int meshNameLength = 0;
	file.read( ( char* )&meshNameLength , sizeof( meshNameLength ) );
	char* meshName = fNewArray( char , meshNameLength );
	file.read( meshName , meshNameLength );

	// VERTEX SIZE INT
	int vertSize = 0;
	file.read( ( char* )&vertSize , sizeof( vertSize ) );

	// Vert Pos Norm Tex Tan
	PosNormTexTan* verts = fNewArray( PosNormTexTan , vertSize );

	VertexPosNormalTexTangent v;
#ifdef ALL_IS_ANIMATED
	VertexPosNormalTexTangentJointsWeights vertA;
#endif
	for ( int i = 0; i < vertSize; i++ )
	{
		file.read( ( char* )&verts[ i ] , sizeof( PosNormTexTan ) );
		v.Position.x = verts[ i ].Position[ 0 ];
		v.Position.y = verts[ i ].Position[ 1 ];
		v.Position.z = verts[ i ].Position[ 2 ];

		v.Normal.x = verts[ i ].Normal[ 0 ];
		v.Normal.y = verts[ i ].Normal[ 1 ];
		v.Normal.z = verts[ i ].Normal[ 2 ];

		v.TexCoord.x = verts[ i ].TexCoord[ 0 ];
		v.TexCoord.y = verts[ i ].TexCoord[ 1 ];

		v.Tangent.x = verts[ i ].Tangent[ 0 ];
		v.Tangent.y = verts[ i ].Tangent[ 1 ];
		v.Tangent.z = verts[ i ].Tangent[ 2 ];

#ifdef ALL_IS_ANIMATED
		vertA.Position = vert.Position;
		vertA.Normal = vert.Normal;
		vertA.TexCoord = vert.TexCoord;
		vertA.Tangent = vert.Tangent;
		vertA.JointIDs = glm::vec4( 0 );
		vertA.Weights = glm::vec4( 1 , 0 , 0 , 0 );
		m_VerticesAnimated.push_back( vertA );
#else
		m_VerticesStatic.push_back( v );
#endif
	}

	// GROUP COUNT
	int groupCount = 0;
	file.read( ( char* )&groupCount , sizeof( groupCount ) );
	
	for ( int i = 0; i < groupCount; i++ )
	{
		gfx::Mesh mesh;
		mesh.VertexBufferOffset = vertexSize;
		mesh.IndexBufferOffset = indexSize;

		// MATERIAL NAME
		int matNameLength = 0;
		file.read( ( char* )&matNameLength , sizeof( matNameLength ) ); // Mat name length
		char* materialName = fNewArray( char , matNameLength );
		file.read( ( char* )materialName , matNameLength ); // Mat name

		// INDEX COUNT
		unsigned int groupIndicesCount = 0;
		file.read( ( char* )&groupIndicesCount , sizeof( groupIndicesCount ) );

		unsigned int* indices = fNewArray( unsigned int , groupIndicesCount );

		// READ INDICES
		unsigned int currentIndex = 0;
		for ( unsigned int i = 0; i < groupIndicesCount; i++ )
		{
			file.read( ( char* )&indices[ i ] , sizeof( unsigned int ) );
			currentIndex = indices[ i ];
			
#ifdef ALL_IS_ANIMATED
			m_IndicesAnimated.push_back( currentIndex + vertexSize );
#else
			m_IndicesStatic.push_back( currentIndex + vertexSize );
#endif
		}

		indexSize += groupIndicesCount;
		mesh.Indices = static_cast<int>( groupIndicesCount );
		mesh.Size = vertSize;
		mesh.Material = materialName;
		model.Meshes.push_back( mesh );

		fDeleteArray( materialName );
		fDeleteArray( indices );
	}

	//float Transformation[ 4 ][ 4 ];


	// MIN & MAX
	file.read( ( char* )&model.Min , sizeof( model.Min ) );
	file.read( ( char* )&model.Max , sizeof( model.Max ) );


	model.NumVertices = static_cast< int >( m_VerticesStatic.size() );
	model.NumIndices = static_cast< int >( m_IndicesStatic.size() );
	model.VertexHandle = static_cast< int >( m_VerticesStatic.size() );

#ifdef ALL_IS_ANIMATED
	model.NumVertices = static_cast< int >( m_VerticesAnimated.size() );
	model.NumIndices = static_cast< int >( m_IndicesAnimated.size() );
	model.VertexHandle = static_cast< int >( m_VerticesAnimated.size() );
#endif

	model.Name = meshName;
	model.Loaded = true;

	fDeleteArray( meshName );
	fDeleteArray( verts ); 
	file.close();
	return true;
}
#pragma endregion

bool gfx::ObjectImporter::LoadMaterial( const rString fileName, Model& model )
{
	rString pathMaterial = "../../../asset/material/" + fileName;

	int currentMat = 0;
	std::ifstream file;
	file.open( pathMaterial.c_str( ) );

	// If file failed to open, create, or what ever
	if( !file.is_open( ) )
	{
		Logger::Log( ( pathMaterial + " could not be opened!" ).c_str( ), "MaterialImporter", LogSeverity::ERROR_MSG );
		return false;
	}

	rString stream_line;

	while( std::getline( file, stream_line ) )
	{
		rStringStream str_stream( stream_line.c_str( ) );
		rString type_str;
		str_stream >> type_str;

		if( type_str == "#" )
		{
			// Ignoring comments for now
		}

		// New material is loaded here
		else if( type_str == "Mat" )
		{
			if( m_Materials.size( ) > 0 )
			{
				// Go to next material
				currentMat++;
			}

			rString materialName = "";
			str_stream >> materialName;

			RoboMat mat = RoboMat( );

			mat.Name = materialName;

			m_Materials.push_back( mat );
		}

		else if( type_str == "Color" )
		{
			float col[ 4 ] = { 0, 0, 0, 0 };
			str_stream >> col[ 0 ] >> col[ 1 ] >> col[ 2 ] >> col[ 3 ];
			memcpy( m_Materials[ currentMat ].Color, col, sizeof( col ) );
		}

		else if( type_str == "Ambient" )
		{
			float amb[ 4 ] = { 0, 0, 0, 0 };
			str_stream >> amb[ 0 ] >> amb[ 1 ] >> amb[ 2 ] >> amb[ 3 ];
			memcpy( m_Materials[ currentMat ].Ambient, amb, sizeof( amb ) );
		}

		else if( type_str == "Specularity" )
		{
			float spec[ 4 ] = { 0, 0, 0, 0 };
			str_stream >> spec[ 0 ] >> spec[ 1 ] >> spec[ 2 ] >> spec[ 3 ];
			memcpy( m_Materials[ currentMat ].Specularity, spec, sizeof( spec ) );
		}

		else if( type_str == "Transperancy" )
		{
			float trans[ 4 ] = { 0, 0, 0, 0 };
			str_stream >> trans[ 0 ] >> trans[ 1 ] >> trans[ 2 ] >> trans[ 3 ];
			memcpy( m_Materials[ currentMat ].Transperancy, trans, sizeof( trans ) );
		}

		else if( type_str == "Incandescence" )
		{
			float Incan[ 4 ] = { 0, 0, 0, 0 };
			str_stream >> Incan[ 0 ] >> Incan[ 1 ] >> Incan[ 2 ] >> Incan[ 3 ];
			memcpy( m_Materials[ currentMat ].Incandescence, Incan, sizeof( Incan ) );
		}

		else if( type_str == "MapDiffuse" )
		{
			rString filePath;
			str_stream >> filePath;

			m_Materials[ currentMat ].AlbedoTex = filePath;
			m_Materials[ currentMat ].HasAlbedoMap = true;
		}

		else if( type_str == "MapRoughness" )
		{
			rString filePath;
			str_stream >> filePath;

			m_Materials[ currentMat ].RoughTex = filePath;
			m_Materials[ currentMat ].HasRoughMap = true;
		}

		else if( type_str == "MapMetal" )
		{
			rString filePath;
			str_stream >> filePath;

			m_Materials[ currentMat ].MetalTex = filePath;
			m_Materials[ currentMat ].HasMetalMap = true;
		}

		else if( type_str == "MapGlow" )
		{
			rString filePath;
			str_stream >> filePath;

			m_Materials[ currentMat ].GlowTex = filePath;
			m_Materials[ currentMat ].HasGlowMap = true;
		}

		else if( type_str == "MapBump" )
		{
			rString filePath;
			str_stream >> filePath;

			m_Materials[ currentMat ].BumpTex = filePath;
			m_Materials[ currentMat ].HasBumpMap = true;
		}
	}

	file.close( );
	return true;
}

/*
*	This function will read data from one object
*	The data is stored inside the obj that was sent in
*/
gfx::EntityObj& gfx::ObjectImporter::ParseObjFromScene( std::ifstream& file, rString& type_str, EntityObj& obj )
{
	// NAME
	file >> type_str;
	file >> obj.Name;

	file >> type_str;
	file >> obj.Radius;

	// If radius is more than zero, than there is a drop rate to be read
	if ( obj.Radius > 0 )
	{
		file >> type_str;
		int dropRate;
		file >> dropRate;
		obj.SetDropRate( dropRate );
	}

	file >> type_str;
	file >> obj.LinkObj;

	file >> type_str;
	file >> obj.Translation[ 0 ] >> obj.Translation[ 1 ] >> obj.Translation[ 2 ];

	file >> type_str;
	file >> obj.Scale[ 0 ] >> obj.Scale[ 1 ] >> obj.Scale[ 2 ];

	file >> type_str;
	file >> obj.Rotation[ 0 ] >> obj.Rotation[ 1 ] >> obj.Rotation[ 2 ] >> obj.Rotation[ 3 ];

	file >> type_str;
	file >> obj.parent;

	return obj;
}

//gfx::VertexPosNormalTexTangent* gfx::ObjectImporter::GetVerticesStatic()
//{
//	return m_VerticesStatic.data();
//}
//
//unsigned int* gfx::ObjectImporter::GetIndicesStatic()
//{
//	return m_IndicesStatic.data();
//}
//
//int gfx::ObjectImporter::GetVerticesSizeStatic() const
//{
//	return static_cast< int >( m_VerticesStatic.size( ) );
//}
//
//int gfx::ObjectImporter::GetIndicesSizeStatic() const
//{
//	return static_cast< int >( m_IndicesStatic.size() );
//}

int gfx::ObjectImporter::GetNumOfScenes() const
{
	return static_cast< int >( m_Scenes.size() );
}

rVector<gfx::RoboMat>& gfx::ObjectImporter::GetMaterials()
{
	return m_Materials;
}

gfx::RoboCamera* gfx::ObjectImporter::GetCamera( const int index ) const
{
	return m_Cameras[ index ];
}

rVector<gfx::RoboCamera*> gfx::ObjectImporter::GetCameras()
{
	return m_Cameras;
}

gfx::RoboLight* gfx::ObjectImporter::GetLight( const int index ) const
{
	return m_Lights[ index ];
}

rVector<gfx::RoboLight*> gfx::ObjectImporter::GetLights()
{
	return m_Lights;
}

/*
*						SCENE FILE
*/
const bool gfx::ObjectImporter::GetSceneByName( gfx::RoboScene& scene, const rString& sceneName ) const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if( strcmp( m_Scenes[ i ].SceneName.c_str(), sceneName.c_str() ) == 0 )
		{
			scene = m_Scenes[ i ];
			return true;
		}
	}
	return false;
}

const bool gfx::ObjectImporter::GetSceneByIndex( gfx::RoboScene& scene, const int index ) const
{
	scene = m_Scenes[ index ];
	return true;
}

gfx::RoboScene* gfx::ObjectImporter::GetScenes()
{
	return m_Scenes.data();
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetControlPointsInScene( const int sceneIndex )const
{
	return m_Scenes[ sceneIndex ].ControlPoints;
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetResourcesInScene( const int sceneIndex )const
{
	return m_Scenes[ sceneIndex ].Resources;
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetPropsInScene( const int sceneIndex )const
{
	return m_Scenes[ sceneIndex ].Props;
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetCamerasInScene( const int sceneIndex )const
{
	return m_Scenes[ sceneIndex ].Cameras;
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetLightsInScene( const int sceneIndex )const
{
	return m_Scenes[ sceneIndex ].Lights;
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetLocatorsInScene( const int sceneIndex )const
{
	return m_Scenes[ sceneIndex ].Locators;
}

const rVector<gfx::EntityObj>& gfx::ObjectImporter::GetSpawnPointsInScene( const int sceneIndex ) const
{
	return m_Scenes[ sceneIndex ].SpawnPoints;
}

const bool gfx::ObjectImporter::GetControlPointsInScene( rVector<gfx::EntityObj>& cps, const rString& sceneName )const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if ( strcmp( m_Scenes[ i ].SceneName.c_str() , sceneName.c_str() ) == 0 )
		{
			cps = m_Scenes[ i ].ControlPoints;
			return true;
		}
	}

	return false;
}

const bool gfx::ObjectImporter::GetResourcesInScene( rVector<gfx::EntityObj>& resources, const rString& sceneName )const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if ( strcmp( m_Scenes[ i ].SceneName.c_str() , sceneName.c_str() ) == 0 )
		{
			resources = m_Scenes[ i ].Resources;
			return true;
		}
	}
	return false;
}

const bool gfx::ObjectImporter::GetPropsInScene( rVector<gfx::EntityObj>& props, const rString& sceneName )const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if( strcmp( m_Scenes[ i ].SceneName.c_str(), sceneName.c_str() ) == 0 )
		{
			props = m_Scenes[ i ].Props;
			return true;
		}
	}
	return false;
}

const bool gfx::ObjectImporter::GetCamerasInScene( rVector<gfx::EntityObj>& cams, const rString& sceneName )const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if ( strcmp( m_Scenes[ i ].SceneName.c_str() , sceneName.c_str() ) == 0 )
		{
			cams = m_Scenes[ i ].Cameras;
			return true;
		}
	}
	return false;
}

const bool gfx::ObjectImporter::GetLightsInScene( rVector<gfx::EntityObj>& lights, const rString& sceneName )const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if ( strcmp( m_Scenes[ i ].SceneName.c_str() , sceneName.c_str() ) == 0 )
		{
			lights = m_Scenes[ i ].Lights;
			return true;
		}
	}
	return false;
}

const bool gfx::ObjectImporter::GetLocatorsInScene( rVector<gfx::EntityObj>& locs, const rString& sceneName )const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if ( strcmp( m_Scenes[ i ].SceneName.c_str() , sceneName.c_str() ) == 0 )
		{
			locs = m_Scenes[ i ].Locators;
			return true;
		}
	}
	return false;
}

const bool gfx::ObjectImporter::GetSpawnPointsInScene( rVector<gfx::EntityObj>& sp, const rString& sceneName ) const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if ( strcmp( m_Scenes[ i ].SceneName.c_str() , sceneName.c_str() ) == 0 )
		{
			sp = m_Scenes[ i ].SpawnPoints;
			return true;
		}
	}
	return false;
}


/*
*	These functions will get object by name
*	Iterating through all types in EACH scene until it finds the object
*	If you know in what scene the object (type) is... use Get3DObjectFromSceneBySceneIndex() instead
*/
const gfx::EntityObj gfx::ObjectImporter::Get3DObject( const rString& name )const
{
	EntityObj object;
	// Find object in scene
	int sceneObjectSize = static_cast< int >( m_Scenes.size() );
	for( int i = 0; i < sceneObjectSize; i++ )
	{
		// Get 3DObject and look for it on all scenes!
		object = Get3DObjectFromSceneBySceneIndex( name, i );

		// Check if it found object by comparing to an empty string
		if( strcmp( object.Name.c_str(), "" ) != 0 )
			return object;
	}

	return object;
}

const bool gfx::ObjectImporter::Get3DObjectFromScene( gfx::EntityObj& obj, const rString& name, const rString& sceneName ) const
{
	for( int i = 0; i < m_Scenes.size(); i++ )
	{
		if( strcmp( m_Scenes[ i ].SceneName.c_str(), sceneName.c_str() ) == 0 )
		{
			obj = Get3DObjectFromSceneBySceneIndex( name, i );
			return true;
		}
	}
	return false;
}

/*
*	These functions will get object by name
*	Iterating through all types in ONE (sceneIndex) scene until it finds the object
*/
const gfx::EntityObj gfx::ObjectImporter::Get3DObjectFromSceneBySceneIndex( const rString& name, const int sceneIndex )const
{
	EntityObj object;

	// Find types in scenes

	// CONTROL POINTS
	int cpSize = static_cast< int >( m_Scenes[ sceneIndex ].ControlPoints.size() );
	for( int i = 0; i < cpSize; i++ )
	{
		// Check if it's a control point
		if( strcmp( name.c_str(), m_Scenes[ sceneIndex ].ControlPoints[ i ].Name.c_str() ) == 0 )
		{
			object = m_Scenes[ sceneIndex ].ControlPoints[ i ];
			return object;
		}
	}

	// RESOURCES
	int resSize = static_cast< int >( m_Scenes[ sceneIndex ].Resources.size() );
	for( int i = 0; i < resSize; i++ )
	{
		// Check if it's a resource
		if( strcmp( name.c_str(), m_Scenes[ sceneIndex ].Resources[ i ].Name.c_str() ) == 0 )
		{
			object = m_Scenes[ sceneIndex ].Resources[ i ];
			return object;
		}
	}

	// PROPS
	int propSize = static_cast< int >( m_Scenes[ sceneIndex ].Props.size() );
	for( int i = 0; i < propSize; i++ )
	{
		// Check if it's a prop
		if( strcmp( name.c_str(), m_Scenes[ sceneIndex ].Props[ i ].Name.c_str() ) == 0 )
		{
			object = m_Scenes[ sceneIndex ].Props[ i ];
			return object;
		}
	}

	// CAMERA
	int cameraSize = static_cast< int >( m_Scenes[ sceneIndex ].Cameras.size() );
	for( int i = 0; i < cameraSize; i++ )
	{
		if( strcmp( name.c_str(), m_Scenes[ sceneIndex ].Cameras[ i ].Name.c_str() ) == 0 )
		{
			object = m_Scenes[ sceneIndex ].Cameras[ i ];
			return object;
		}
	}

	// LIGHT
	int lightSize = static_cast< int >( m_Scenes[ sceneIndex ].Lights.size() );
	for( int i = 0; i < lightSize; i++ )
	{
		if( strcmp( name.c_str(), m_Scenes[ sceneIndex ].Lights[ i ].Name.c_str() ) == 0 )
		{
			object = m_Scenes[ sceneIndex ].Lights[ i ];
			return object;
		}
	}

	// LOCATOR
	int locatorSize = static_cast< int >( m_Scenes[ sceneIndex ].Locators.size() );
	for( int i = 0; i < locatorSize; i++ )
	{
		if( strcmp( name.c_str(), m_Scenes[ sceneIndex ].Locators[ i ].Name.c_str() ) == 0 )
		{
			object = m_Scenes[ sceneIndex ].Locators[ i ];
			return object;
		}
	}

	return object;
}

/*
*	This function will first find and load the scene.
*	After it has loaded the scene, it will start looking for the object in loaded scene
*/
const gfx::EntityObj gfx::ObjectImporter::Get3DObjectFromUnLoadedScene( const rString& sceneName, const rString& name )
{
	EntityObj object;

	if( LoadScene( sceneName ) )
	{
		Get3DObjectFromScene( object, name, sceneName );
	}

	return object;
}

void gfx::ObjectImporter::PrintMemory()
{
	size_t memSize = 0;

	memSize += m_VerticesStatic.size() * sizeof(gfx::VertexPosNormalTexTangent);
	memSize += m_VerticesAnimated.size() * sizeof(gfx::VertexPosNormalTexTangentJointsWeights);
	memSize += m_IndicesStatic.size() * sizeof(unsigned int);
	memSize += m_IndicesAnimated.size() * sizeof(unsigned int);

	printf("size of object importer %u \n\n", memSize);
}
