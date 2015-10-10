/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSSceneLoader.h"
#include <utility/Logger.h>
#include <gfx/GraphicsEngine.h>

SSSceneLoader& SSSceneLoader::GetInstance()
{
	static SSSceneLoader instance;
	return instance;
}

void SSSceneLoader::Startup()
{
	if( !m_Sceneloader.LoadScene( m_SceneToBeLoaded ) )
		Logger::Log( "Failed to load scene file <" + m_SceneToBeLoaded + ">", "SSSceneLoader", LogSeverity::ERROR_MSG );

	Subsystem::Startup();
}

void SSSceneLoader::Shutdown()
{
	gfx::g_GFXTerrain.Cleanup();
}

const SceneLoader SSSceneLoader::GetScene() const
{
	return m_Sceneloader;
}

rString SSSceneLoader::GetSceneName() const
{
	return m_SceneToBeLoaded;
}

void SSSceneLoader::SetSceneToBeLoaded( const rString& name )
{
	m_SceneToBeLoaded = name;
}