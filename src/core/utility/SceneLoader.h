/**************************************************
Copyright 2015 Mostafa Hassan & Richard Pettersson
***************************************************/

# pragma once
#include <memory/Alloc.h>

class SceneLoader
{
public:
	bool LoadScene( const rString& sceneName = "" );
	//const rVector<glm::vec3>& GetSpawnPoints();

	const int GetHeightMapWidth() const;
	const int GetHeightMapHeight() const;

	const rString GetDiffuseTex() const;
	const rString GetNormalMapTex() const;
	const rString GetHeightMapTex() const;
	const rString GetAIMap( ) const;

private:
	//rVector<glm::vec3> m_SpawnPoints;

	// Height map
	int m_MapWidth;
	int m_MapHeight;
	rString m_DiffuseTexture;
	rString m_NormalMapTexture;
	rString m_HeightMapTexture;
	rString m_AIMap;
};