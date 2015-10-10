/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include "../../utility/SceneLoader.h"

// Conveniency access function
#define g_SSSceneLoader SSSceneLoader::GetInstance()

class SSSceneLoader : public Subsystem
{
public:
	static SSSceneLoader& GetInstance();

	void Startup() override;
	void Shutdown() override;

	const SceneLoader GetScene() const;
	rString	GetSceneName() const;

	void SetSceneToBeLoaded( const rString& name );
	
private:
	// No external instancing allowed
	SSSceneLoader() : Subsystem( "SceneLoader" , "Scene Loader" ) { m_SceneToBeLoaded = "RoboWar"; }
	SSSceneLoader( const SSSceneLoader& rhs );
	~SSSceneLoader() {};
	SSSceneLoader& operator=( const SSSceneLoader& rhs );

	rString 	m_SceneToBeLoaded;
	SceneLoader m_Sceneloader;
};