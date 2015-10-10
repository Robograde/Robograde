/**************************************************
2015 Henrik Johansson
***************************************************/

#include "SSShadow.h"
#include "../gamelogic/SSCamera.h"
#include <gfx/GraphicsEngine.h>
#include <glm/gtx/transform.hpp>
#include <utility/ConfigManager.h>
SSShadow& SSShadow::GetInstance( )
{
    static SSShadow instance;
    return instance;
}

void SSShadow::Startup( )
{
	//read the config for shadow quality
	CallbackConfig* gfxConfig = g_ConfigManager.GetConfig( "graphics.cfg" );
	int shadowSize = gfxConfig->GetInt("ShadowQuality", 1024,"Resolution of the shadowmap");
	m_DirLight.Position = vec3( 80, 400, 200 );
	m_DirLight.Direction = glm::normalize(vec3(200, 0, 200) - m_DirLight.Position);
	m_DirLight.Color = vec4(1.0f);

	m_ShadowMap = pNew(gfx::ShadowMap);
	m_ShadowMap->Initialize(shadowSize);
	gfx::g_GFXEngine.SetShadowMap(m_ShadowMap);
}

void SSShadow::UpdateUserLayer( const float deltaTime )
{
	m_Picking.ClearUnitSelection( );

	//Get camera frustrum point position
	glm::ivec2 screenSize( g_SSCamera.GetActiveCamera()->GetEditableLens().WindowWidth, g_SSCamera.GetActiveCamera()->GetEditableLens().WindowHeight );
	rVector<int> pickingTargets;
	pickingTargets.push_back(PICKING_TYPE_TERRAIN);
	m_Picking.PickRay(pickingTargets,*g_SSCamera.GetActiveCamera(),screenSize * glm::ivec2(0,0));
	glm::vec3 pos1 = m_Picking.GetClickedPosition();
	m_Picking.PickRay(pickingTargets,*g_SSCamera.GetActiveCamera(),screenSize * glm::ivec2(1,0));
	glm::vec3 pos2 = m_Picking.GetClickedPosition();
	m_Picking.PickRay(pickingTargets,*g_SSCamera.GetActiveCamera(),screenSize * glm::ivec2(0,1));
	glm::vec3 pos3 = m_Picking.GetClickedPosition();
	m_Picking.PickRay(pickingTargets,*g_SSCamera.GetActiveCamera(),screenSize * glm::ivec2(1,1));
	glm::vec3 pos4 = m_Picking.GetClickedPosition();

	glm::vec3 middle = (pos1 + pos2 + pos3 + pos4) / 4.0f;
	float xMin,xMax,zMin,zMax;
	xMax = glm::max(glm::max(pos1.x,pos2.x), glm::max(pos3.x,pos4.x));
	xMin = glm::min(glm::min(pos1.x,pos2.x), glm::min(pos3.x,pos4.x));
	zMax = glm::max(glm::max(pos1.z,pos2.z), glm::max(pos3.z,pos4.z));
	zMin = glm::min(glm::min(pos1.z,pos2.z), glm::min(pos3.z,pos4.z));

	float frustrumSize = glm::max(xMax - xMin, zMax - zMin) * 0.7f;
	m_DirLight.Position = middle + (-m_DirLight.Direction * 60.0f);
	m_DirLight.Position.y = 60.0f;
	m_ShadowMap->GenerateMatrixFromLight(m_DirLight, frustrumSize, 1.0f, 180.0f);
	gfx::g_LightEngine.AddDirLightToQueue( m_DirLight );
}

void SSShadow::Shutdown( )
{
    // Perform Cleanup here (Don't forget to set shutdown order priority!)
	pDelete(m_ShadowMap);
}
