/**************************************************
2015 Mostafa Hassan & Richard Pettersson
***************************************************/

#include "SceneLoader.h"
#include <gfx/GraphicsEngine.h>
#include <gfx/ObjectImporter.h>
#include "../EntityFactory.h"
#include "GameData.h"
#include "../subsystem/gamelogic/SSCamera.h"
#include "../subsystem/gamelogic/SSResourceManager.h"
#include "../subsystem/audio/SSMusicManager.h"
#include "../ai/Terrain.h"
#include "GameModeSelector.h"

bool SceneLoader::LoadScene( const rString& sceneName )
{
	if( sceneName != "" )
	{
		// Load the scene file
		gfx::ObjectImporter importer;
		bool status = importer.LoadScene( sceneName );

		// Could not load scene file
		if( status == false )
			return false;

		gfx::RoboScene scene;
		status = importer.GetSceneByName( scene, sceneName );

		m_MapWidth = scene.Width;
		m_MapHeight = scene.Height;
		g_GameData.SetFieldWidth( m_MapWidth );
		g_GameData.SetFieldHeight( m_MapHeight );
		g_GameData.SetCurrentMapName( sceneName );

		const rString auidoPath = "../../../asset/audio/script/";
		g_SSMusicManager.LoadSceneMusic( auidoPath + scene.BGMusic );
		g_SSMusicManager.Activate();

		//needed before creating entities with positions and after field w/h is read
		Terrain::GetInstance()->InitField();

		EntityFactory::CreateTerrain();
		gfx::Terrain::Config c; 
		c.Channels = 3;
		c.HeightScale = scene.HeightScale;
		c.ImageHeight = scene.Height;
		c.ImageWidth = scene.Width;
		c.MapDimensionX = scene.Width;
		c.MapDimensionZ = scene.Height;
		c.Albedo1Texture = scene.Albedo1Texture;
		c.Albedo2Texture = scene.Albedo2Texture;
		c.Albedo3Texture = scene.Albedo3Texture;
		c.Albedo4Texture = scene.Albedo4Texture;
		c.BlendmapTexture = scene.BlendmapTexture;


		c.TessellationFactor = 1;
		c.VertexGranularityX = 256;
		c.VertexGranularityZ = 256;
		gfx::g_GFXTerrain.SetWireframeMode( false );

		c.Position = { 0, 0, 0 };
		c.TextureRepeat = 20;
		c.HeightmapRepeat = 1;
		if( !g_GameData.IsDedicatedServer() )
		{
			m_HeightMapTexture = "../../../asset/maps/" + sceneName + "/" + scene.HeightMap;
			//m_NormalMapTexture = "../../../asset/texture/" + scene.NormalMap;
			m_AIMap = "../../../asset/maps/" + sceneName + "/" + scene.AIMap;

			bool result = gfx::g_GFXTerrain.LoadFromFile( m_HeightMapTexture, c );
			if( !result )
				return false;
		}

		if (g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Editor)
			return true;

		// Fetch the objects
		rVector <gfx::EntityObj>	controlPoints;
		rVector <gfx::EntityObj>	props;
		rVector <gfx::EntityObj>	resources;
		rVector <gfx::EntityObj>	spawnPoints;
		controlPoints				= importer.GetControlPointsInScene( 0 );
		props						= importer.GetPropsInScene( 0 );
		resources					= importer.GetResourcesInScene( 0 );
		spawnPoints				    = importer.GetSpawnPointsInScene( 0 );

		const char* roboExt = ".robo";
		const char* armatureExt = ".skelrob";
		const rString defaultProp = "RockShape.robo";
		const rString defaultResource = "resource2.robo";
		const rString defaultControlPoint = "ResearchBottomShape.robo";
		
		// Handle objects
		for( int i = 0; i < controlPoints.size(); ++i )
		{
			gfx::EntityObj controlPoint = controlPoints[ i ];
			// If the object is "linked" to a model, then use that mesh to render the object. If not, use a default model
			if( strcmp( controlPoint.LinkObj.c_str(), "_None_" ) == 0 )
			{
				EntityFactory::CreateControlPoint( controlPoint.Translation[ 0 ], controlPoint.Translation[ 2 ], glm::vec3( controlPoint.Scale[ 0 ], controlPoint.Scale[ 1 ], controlPoint.Scale[ 2 ] ),
					glm::quat( controlPoint.Rotation[ 3 ], controlPoint.Rotation[ 0 ], controlPoint.Rotation[ 1 ], controlPoint.Rotation[ 2 ] ), NO_OWNER_ID, defaultControlPoint );
			}

			else
			{
				EntityFactory::CreateControlPoint( controlPoint.Translation[ 0 ], controlPoint.Translation[ 2 ], glm::vec3( controlPoint.Scale[ 0 ], controlPoint.Scale[ 1 ], controlPoint.Scale[ 2 ] ),
					glm::quat( controlPoint.Rotation[ 3 ], controlPoint.Rotation[ 0 ], controlPoint.Rotation[ 1 ], controlPoint.Rotation[ 2 ] ), NO_OWNER_ID, controlPoint.LinkObj + roboExt );
			}
		}

		for ( int i = 0; i < props.size(); ++i )
		{
			gfx::EntityObj prop = props[ i ];
			if ( prop.Radius > 0 )
				continue;

			bool isStaticTree = false;

			if ( prop.LinkObj == "TreeShape" )
				isStaticTree = true;

			// TODOMH Remove hardcoded animations
			if ( prop.LinkObj == "TreeShapeAnim" )
			{
				EntityFactory::CreateAnimatedProp( prop.Translation[ 0 ] , prop.Translation[ 2 ] , glm::vec3( prop.Scale[ 0 ] , prop.Scale[ 1 ] , prop.Scale[ 2 ] ) ,
					glm::quat( prop.Rotation[ 3 ] , prop.Rotation[ 0 ] , prop.Rotation[ 1 ] , prop.Rotation[ 2 ] ) , true , prop.LinkObj + armatureExt , "TreeAnimation.roboanim" , 1 );
				continue;
			}

			else if ( prop.LinkObj == "Snowman1Shape" )
			{
				EntityFactory::CreateAnimatedProp( prop.Translation[ 0 ] , prop.Translation[ 2 ] , glm::vec3( prop.Scale[ 0 ] , prop.Scale[ 1 ] , prop.Scale[ 2 ] ) ,
					glm::quat( prop.Rotation[ 3 ] , prop.Rotation[ 0 ] , prop.Rotation[ 1 ] , prop.Rotation[ 2 ] ) , true , prop.LinkObj + armatureExt , "SnowManAnimation.roboanim" , 1 );
				continue;
			}

			// If the object is "linked" to a model, then use that mesh to render the object. If not, use a default model
			else if ( strcmp( prop.LinkObj.c_str() , "_None_" ) == 0 )
			{
				EntityFactory::CreateProp( prop.Translation[ 0 ] , prop.Translation[ 2 ] , glm::vec3( prop.Scale[ 0 ] , prop.Scale[ 1 ] , prop.Scale[ 2 ] ) ,
					glm::quat( prop.Rotation[ 3 ] , prop.Rotation[ 0 ] , prop.Rotation[ 1 ] , prop.Rotation[ 2 ] ) , true , defaultProp , 1 , isStaticTree ); // TODODB: Set the last parameter when it gets exported
				continue;
			}

			else if ( strcmp( prop.LinkObj.c_str() , "Grass" ) == 0 )
			{
				EntityFactory::CreateCombinedProp( prop.Translation[ 0 ] , prop.Translation[ 1 ], prop.Translation[ 2 ] , glm::vec3( prop.Scale[ 0 ] , prop.Scale[ 1 ] , prop.Scale[ 2 ] ) ,
					glm::quat( prop.Rotation[ 3 ] , prop.Rotation[ 0 ] , prop.Rotation[ 1 ] , prop.Rotation[ 2 ] ) , prop.LinkObj + roboExt );
				continue;
			}

			// DEFAULT PRPOP
			else
			{
				EntityFactory::CreateProp( prop.Translation[ 0 ] , prop.Translation[ 2 ] , glm::vec3( prop.Scale[ 0 ] , prop.Scale[ 1 ] , prop.Scale[ 2 ] ) ,
					glm::quat( prop.Rotation[ 3 ] , prop.Rotation[ 0 ] , prop.Rotation[ 1 ] , prop.Rotation[ 2 ] ) , true , prop.LinkObj + roboExt , 1 , isStaticTree );
			}
		}

		for ( auto& model : scene.ResourceFiles )
		{
			gfx::g_ModelBank.LoadModel( (model + roboExt).c_str( ) );
			g_SSResourceManager.AddModelPath( (model + roboExt).c_str( ) );
		}

		for( int i = 0; i < resources.size(); ++i )
		{
			gfx::EntityObj resource = resources[ i ];
			
			g_SSResourceManager.AddResource( glm::vec2( resource.Translation[0], resource.Translation[2] ), glm::vec3( resource.Scale[0], resource.Scale[1], resource.Scale[2] ), glm::quat( resource.Rotation[3], resource.Rotation[0], resource.Rotation[1], resource.Rotation[2] ), (resource.LinkObj + roboExt).c_str(), resource.Radius > 0.0f, resource.Radius, resource.GetDropRate( ) );

			//if( resource.Radius > 0 )
			//	continue;

			//// If the object is "linked" to a model, then use that mesh to render the object. If not, use a default model
			//if( strcmp( resource.LinkObj.c_str(), "_None_" ) == 0 )
			//{
			//	EntityFactory::CreateResource( resource.Translation[ 0 ], resource.Translation[ 2 ], glm::vec3( resource.Scale[ 0 ], resource.Scale[ 1 ], resource.Scale[ 2 ] ),
			//		glm::quat( resource.Rotation[ 3 ], resource.Rotation[ 0 ], resource.Rotation[ 1 ], resource.Rotation[ 2 ] ), defaultResource );
			//}

			//else
			//{
			//	EntityFactory::CreateResource( resource.Translation[ 0 ], resource.Translation[ 2 ], glm::vec3( resource.Scale[ 0 ], resource.Scale[ 1 ], resource.Scale[ 2 ] ),
			//		glm::quat( resource.Rotation[ 3 ], resource.Rotation[ 0 ], resource.Rotation[ 1 ], resource.Rotation[ 2 ] ), resource.LinkObj + roboExt );
			//}
		}

		for( int i = 0; i < spawnPoints.size(); ++i )
		{
			gfx::EntityObj spawn = spawnPoints[ i ];
			glm::vec3 point = glm::vec3( spawn.Translation[ 0 ], 1, spawn.Translation[ 2 ] );

			g_GameData.AddSpawnPoint( point );
		}
		importer.Release();
		return true;
	}
	return false;

	/*else
	{
	EntityFactory::CreateTerrain();

	m_MapWidth = 257;
	m_MapHeight = 257;

	g_GameData.SetFieldWidth( m_MapWidth );
	g_GameData.SetFieldHeight( m_MapHeight );

	for ( int i = -3; i < 3; ++i )
	{
	for ( int j = -3; j < 3; j++ )
	{
	float x = m_MapWidth / 2.0f + ( i * 8.0f );
	float y = m_MapHeight / 2.0f + ( j * 8.0f );

	EntityFactory::CreateResource( x, y, DEFAULT_SCALE, DEFAULT_ORIENTATION );
	EntityFactory::CreateResource( x + 2, y + 2, DEFAULT_SCALE, DEFAULT_ORIENTATION );
	EntityFactory::CreateResource( x + 1, y + 1, DEFAULT_SCALE, DEFAULT_ORIENTATION );
	EntityFactory::CreateResource( x, y + 3, DEFAULT_SCALE, DEFAULT_ORIENTATION );
	}
	}

	EntityFactory::CreateProp( 5.0f, 7.0f, DEFAULT_SCALE, DEFAULT_ORIENTATION, true, "Rock.robo", 1 );
	EntityFactory::CreateProp( 10.0f, 7.0f, DEFAULT_SCALE, DEFAULT_ORIENTATION, true, "Rock.robo", 2 );
	EntityFactory::CreateProp( 20.0f, 7.0f, DEFAULT_SCALE, DEFAULT_ORIENTATION, true, "Rock.robo", 3 );
	EntityFactory::CreateProp( 30.0f, 7.0f, DEFAULT_SCALE, DEFAULT_ORIENTATION, true, "Rock.robo", 4 );

	EntityFactory::CreateControlPoint( 20.0f, 20.0f, glm::vec3( 3.0f ), DEFAULT_ORIENTATION, 0, "resource2.robo" );
	EntityFactory::CreateControlPoint( g_GameData.GetFieldWidth() - 20.0f, 20.0f, glm::vec3( 3.0f ), DEFAULT_ORIENTATION, 1, "resource2.robo" );
	EntityFactory::CreateControlPoint( g_GameData.GetFieldWidth() - 20.0f, g_GameData.GetFieldHeight() - 20.0f, glm::vec3( 3.0f ), DEFAULT_ORIENTATION, 2, "resource2.robo" );
	EntityFactory::CreateControlPoint( 20.0f, g_GameData.GetFieldHeight() - 20.0f, glm::vec3( 3.0f ), DEFAULT_ORIENTATION, 3, "resource2.robo" );

	EntityFactory::ScriptInitializeBindings();

	gfx::Terrain::Config c;
	c.Channels = 3;
	c.HeightScale = 0.02f;
	c.ImageHeight = m_MapWidth;
	c.ImageWidth = m_MapHeight;

	c.MapDimensionX = m_MapWidth;
	c.MapDimensionZ = m_MapHeight;
	c.TessellationFactor = 20;

	c.VertexGranularityX = 5;
	c.VertexGranularityZ = 5;
	c.Position = { 0, 0, 0 };
	c.TextureRepeat = 8;
	c.HeightmapRepeat = 1;

	m_HeightMapTexture = "../../../asset/texture/heightmap.png";
	m_NormalMapTexture =  "../../../asset/texture/normal3.png";
	m_DiffuseTexture = "../../../asset/texture/grass.png";
	bool result = gfx::g_GFXTerrain.LoadFromFile( m_HeightMapTexture , m_NormalMapTexture ,
	m_DiffuseTexture, c );

	if ( !result )
	return false;
	return true;
	}*/
}

// const rVector<glm::vec3>& SceneLoader::GetSpawnPoints()
// {
// 	return m_SpawnPoints;
// }

const int SceneLoader::GetHeightMapWidth() const
{
	return m_MapWidth;
}

const int SceneLoader::GetHeightMapHeight() const
{
	return m_MapHeight;
}

const rString SceneLoader::GetDiffuseTex( ) const
{
	return m_DiffuseTexture;
}

const rString SceneLoader::GetNormalMapTex() const
{
	return m_NormalMapTexture;
}

const rString SceneLoader::GetHeightMapTex() const
{
	return m_HeightMapTexture;
}

const rString SceneLoader::GetAIMap() const
{
	return m_AIMap;
}
