/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <messaging/Subscriber.h>
#include <gui/GUIEngine.h>
#include <gfx/Texture.h>
#include "../../picking/Picking.h"
#include "../../picking/PickingType.h"

#define g_SSMiniMap SSMiniMap::GetInstance()

class SSMiniMap : public Subsystem, public Subscriber
{
#define UNIT_SPRITE_SIZE 3
#define RESOURCE_SPRITE_SIZE 2
#define RESOURCE_SPAWN_SPRITE_SIZE 12
#define CONTROL_POINT_SPRITE_SIZE 16
#define PING_SPRITE_SIZE 16
#define CAMERA_SPRITE_SIZE 2
	
#define PING_LIFE_TIME 2.0f
	
public:
	static SSMiniMap& GetInstance( );
	
	void Startup() override;
	void UpdateUserLayer( const float dt ) override;
	void Shutdown() override;
	
	void SetMapSize( int width, int height );
	
	glm::ivec2 ConvertToMapCoordinates( const glm::vec3& worldPos );
	glm::vec3 ConvertToWorldCoordinates( const glm::ivec2& mapPos );
	
	void PingLocation( glm::vec3 worldPos, glm::vec3 colour = glm::vec3( 1.0f, 1.0f, 0.0f ) );
	//void UpdateSquadRendering();

	const rString& GetMiniMapTexturePath() const;
	void SetMiniMapTexture( const rString& texturePath );
	
	
private:
	// No external instancing allowed
	SSMiniMap				() : Subsystem( "MiniMap" ), Subscriber( "MiniMap" ) {}
	SSMiniMap				( const SSMiniMap & rhs );
	~SSMiniMap				() {}
	SSMiniMap&				operator = (const SSMiniMap & rhs);

	struct PingData
	{
		glm::vec2	Position;
		glm::vec3	Colour;
		float		TimeLeft;
		float		PingLength;
	};
	
	void					SendUserPing( const glm::vec3& position );
	void					UpdatePings( float deltaTime );
	void					HandleClicks();
	void					UpdateCameraArea();
	
	int						m_MapSizeX = 0;
	int						m_MapSizeY = 0;
	
	const int 				m_PingButtonWidth = 64;
	const int 				m_PingButtonHeight = 32;
	
	GUI::Sprite*			m_FogSprite;
	GUI::Sprite*			m_BackgroundSprite;
	
	rVector<GUI::Sprite*>	m_UnitSprites;
	rVector<GUI::Sprite*>	m_SquadSprites;
	rVector<GUI::Sprite*>	m_ControlPointSprites;
	rVector<GUI::Sprite*>	m_ResourceSprites;
	rVector<GUI::Sprite*>	m_ResourceSpawnSprites;
	rVector<GUI::Sprite*>	m_CameraSprites;
	rVector<GUI::Sprite*>	m_RadarSprites;
	rVector<GUI::Sprite*>	m_PingSprites;
	Picking					m_Picking;
	rVector<int>			m_PickingTarget;
	rVector<PingData>		m_PingList;
	
	int						m_ControlPointCount = 0;
	int						m_ResourceSpawnCount = 0;
	gfx::Texture*			m_MiniMapTexture;
	rString					m_MiniMapTexturePath;
};
