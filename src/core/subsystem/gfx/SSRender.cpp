/**************************************************
2015 Robograde Development Team
***************************************************/

#include "SSRender.h"
#include <iostream>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <gfx/GraphicsEngine.h>
#include <utility/Colours.h>
#include "../gamelogic/SSCamera.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/ComponentHelper.h"
#include "../../component/PlacementComponent.h"
#include "../../component/ModelComponent.h"
#include "../../component/OwnerComponent.h"
#include "../../component/ColourComponent.h"
#include "../../component/SelectionComponent.h"
#include "../../picking/Picking.h"
#include "../../picking/PickingType.h"
#include "../../component/ModelComponent.h"
#include "../../subsystem/gamelogic/SSProjectiles.h"
#include "../../subsystem/gamelogic/SSRadar.h"
#include "../../subsystem/gamelogic/SSResourceManager.h"
#include "../../subsystem/gfx/SSFogOfWar.h"
#include "../../subsystem/gfx/SSAnimation.h"
#include <gfx/RenderJobManager.h>
#include <profiler/AutoProfiler.h>

#if AI_DEBUG == 1
#include "../../ai/Terrain.h"
#endif
#if AI_RENDER_DEBUG == 1
#include "../../utility/GameData.h"
#endif

SSRender& SSRender::GetInstance( )
{
	static SSRender instance;
	return instance;
}

SSRender::SSRender( )
	: Subsystem( "Render" ) { }

void SSRender::Startup()
{
	m_DebugRenderer.Initialize();

	for ( int i = 0; i < PICKING_TYPE_SIZE; ++i )
	{
		m_CullingGroups.push_back( i );
	}
	m_RenderJobManager = gfx::g_GFXEngine.GetRenderJobManager();
	Subsystem::Startup();
}

void SSRender::Shutdown()
{
	m_CullingGroups.clear();
	m_RenderBuckets.clear();

	Subsystem::Shutdown();
}

void SSRender::UpdateUserLayer( const float dt )
{

	m_RenderJobManager->VIEW_MATRIX = g_SSCamera.GetActiveCamera()->GetView();
	m_RenderJobManager->PROJ_MATRIX = g_SSCamera.GetActiveCamera()->GetProjection();
	Camera* cullingCamera = g_SSCamera.GetActiveCamera();
	m_GraphicsCulling.ClearUnitSelection();
	m_GraphicsCulling.PickFrustum( m_CullingGroups, *cullingCamera, glm::ivec2( 0 ), glm::ivec2( cullingCamera->GetLens().WindowWidth, cullingCamera->GetLens().WindowHeight ) );
	m_DebugRenderer.SetViewProj( g_SSCamera.GetActiveCamera()->GetView(), g_SSCamera.GetActiveCamera()->GetProjection() );
	glm::vec4 campos = glm::vec4(cullingCamera->GetPosition().x,cullingCamera->GetPosition().y,cullingCamera->GetPosition().z,1.0f);
	m_RenderJobManager->SetSceneInputs(cullingCamera->GetView(),cullingCamera->GetProjection(), campos);

	EntityMask placementAndModelFlag	= GetDenseComponentFlag<PlacementComponent>() | GetDenseComponentFlag<ModelComponent>();
	EntityMask colourFlag				= GetDenseComponentFlag<ColourComponent>();
	EntityMask selectionFlag			= GetDenseComponentFlag<SelectionComponent>();
	for ( auto& entityID : m_GraphicsCulling.GetUnitsSelected() )
	{
		EntityMask entityMask = EntityManager::GetInstance().GetEntityMask( entityID );
		if ( !( ( entityMask & placementAndModelFlag ) == placementAndModelFlag ) )
			continue;

		if ( GetDenseComponent<ModelComponent>( entityID )->IsVisible == false )
			continue;

		glm::vec4 colour;
		if ( entityMask & colourFlag )
			colour = GetDenseComponent<ColourComponent>( entityID )->Colour;
		else
			colour = Colours::WHITE;

		PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>( entityID );
		CalculateWorldMatrix( placementComponent->Position, placementComponent->Scale, placementComponent->Orientation, placementComponent->World );
		

		int handle = 0; //TODOHJ: Document why this is 0. (Isn't that an actual model?) -because we don't have an error model
		if ( HasComponent<ModelComponent>( entityID ) )
		{
			//if ( !HasComponent<AnimationComponent>( entityID ) )
			{
				handle = GetDenseComponent<ModelComponent>( entityID )->ModelHandle;
			}
			//else
			//{
			//	AnimationComponent* animationComponent = GetDenseComponent<AnimationComponent>( entityID );
			//	int hndl = GetDenseComponent<ModelComponent>( entityID )->ModelHandle;
			//	m_RenderJobManager->RenderAnimated(hndl, placementComponent->World, colour, animationComponent->MatrixList);
			//}
		}
		if ( HasComponent<AnimationComponent>( entityID ) )
		{
			AnimationComponent* animationComponent = GetDenseComponent<AnimationComponent>( entityID );
			if ( animationComponent->MatrixList.size() > 0 )
			{
				m_RenderBuckets[handle].joints.insert(m_RenderBuckets[handle].joints.end(), animationComponent->MatrixList.begin(), animationComponent->MatrixList.end());
				m_RenderBuckets[handle].bindPose.insert(m_RenderBuckets[handle].bindPose.end(), animationComponent->AnimationRef->GetBindPoseRef().begin(), animationComponent->AnimationRef->GetBindPoseRef().end());
				m_RenderBuckets[handle].animated = true;
			}
		}
		m_RenderBuckets[handle].worlds.push_back( placementComponent->World );
		m_RenderBuckets[handle].colors.push_back( colour );

		if ( ( entityMask & selectionFlag ) )
		{
			SelectionComponent* selectionComponent = GetDenseComponent<SelectionComponent>( entityID );

			if ( selectionComponent->Selected )
			{
				selectionComponent->Selected = false;
				//glm::mat4 world = glm::translate( placementComponent->Position ) * glm::scale( 2.0f * selectionComponent->Radius * placementComponent->Scale );
				//m_DebugRenderer.RenderSphere( world, glm::vec3( 1.0f ) );
			}
			else if ( selectionComponent->MouseOvered )
			{
				selectionComponent->MouseOvered = false;
				//glm::mat4 world = glm::translate( placementComponent->Position ) * glm::scale( 2.0f * selectionComponent->Radius * placementComponent->Scale );
				//m_DebugRenderer.RenderSphere( world, glm::vec3( 0.7f ) );
			}
		}
	}
	//Please do not remove, needed to debug AI if bugs occur
#if AI_RENDER_GOALS

	EntityMask agentFlag = DenseComponentCollection<AgentComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for (auto& entityMask : EntityManager::GetInstance().GetEntityMasks())
	{
		if (entityMask & agentFlag)
		{
			Agent* agent = GetAgentPointer(entityID);
			Tile* t = agent->GetGoal();

			glm::mat4 mat = glm::translate(glm::vec3(t->X, 5, t->Y)) * glm::scale(glm::vec3(2.0f));
			m_DebugRenderer.RenderBox(mat, glm::vec3(0, 1, 1));

		}
		entityID++;
	}

#endif

#if AI_RENDER_DEBUG == 1
	int width = g_GameData.GetFieldWidth();
	int height = g_GameData.GetFieldHeight();

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			Tile* t = Terrain::GetInstance()->GetTile(j, i);

// 			if (t->Cost == 1)
// 			{
// 				glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(vec3(0.9f));
// 				m_DebugRenderer.RenderBox(mat, glm::vec3(0, 1, 0));
// 			}

			//if (t->DebugInfo == 14)
			//{
			//	glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(vec3(0.9f));
			//	m_DebugRenderer.RenderBox(mat, glm::vec3(0, 1, 0));
			//}
			//if (t->DebugInfo == 16)
			//{
			//	glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(vec3(0.9f));
			//	m_DebugRenderer.RenderBox(mat, glm::vec3(1, 1, 1));
			//}
			//if (t->DebugInfo == 15)
			//{
			//	glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(glm::vec3(1.0f));
			//	m_DebugRenderer.RenderBox(mat, glm::vec3(1, 0, 0));
			//}

#if AI_RENDER_DEBUG_TERRAIN == 1

// 			if (t->Terrain == '#')
// 			{
				glm::mat4 mat = glm::translate(glm::vec3(j, 0, i)) * glm::scale(glm::vec3(0.9f));
				m_DebugRenderer.RenderBox(mat, glm::vec3(0, t->Cost/70.0f, 0));
//			}
// 			else if (t->Terrain > 0)
// 			{
// 				glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(glm::vec3(0.9f));
// 				if (Terrain::GetInstance()->IsUnpathableTerrain(t))
// 					m_DebugRenderer.RenderBox(mat, glm::vec3(1, 1, 0));
// 				else
// 					m_DebugRenderer.RenderBox(mat, glm::vec3(t->Terrain / 9.0f, t->Terrain / 9.0f, 0));
// 			}
#endif

			if (HasComponent<DoodadComponent>(Terrain::GetInstance()->WhoIsOnTile(t)))
			{
				glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(glm::vec3(1));
				m_DebugRenderer.RenderBox(mat, glm::vec3(1, 0, 1));
			}
			else if (HasComponent<AgentComponent>(Terrain::GetInstance()->WhoIsOnTile(t)))
			{
				glm::mat4 mat = glm::translate(glm::vec3(j, 10, i)) * glm::scale(glm::vec3(1.2f));
				m_DebugRenderer.RenderBox(mat, glm::vec3(0, 1, 0));
			}
		}
	}
#endif
	g_SSProjectiles.Render( m_RenderBuckets );
	g_SSRadar.Render( m_RenderBuckets );
	g_SSResourceManager.Render( m_RenderBuckets );
	for(auto& it : m_RenderBuckets)
	{
		if ( it.second.worlds.size() > 0 )
		{
			//if(it.second.bindPose.size() > 0)
			if ( it.second.joints.size() > 0 )
			{
				m_RenderJobManager->RenderInstancedAnimated( it.first, it.second.worlds, it.second.colors, it.second.joints, it.second.bindPose );
				// m_RenderJobManager->RenderInstanced( it.first, it.second.worlds, it.second.colors );
			}
			else
			{
				m_RenderJobManager->RenderInstanced( it.first, it.second.worlds, it.second.colors );
			}
			it.second.worlds.clear();
			it.second.colors.clear();
			it.second.joints.clear();
			it.second.bindPose.clear();
		}
	}
	m_RenderJobManager->Render( 1, glm::translate(glm::vec3(-10000)),glm::vec4(1));
}

void SSRender::RenderLine(const glm::vec3& origin, const glm::vec3& destination, const glm::vec4& color, bool dashed, float animationProgres)
{
	m_RenderJobManager->RenderLine(origin, destination, color, dashed, animationProgres);
}

void SSRender::CalculateWorldMatrix( const glm::vec3& position, const glm::vec3& scale, glm::quat& outOrientation, glm::mat4& outWorld )
{
	glm::normalize( outOrientation );
	glm::mat4x4 rotation = glm::toMat4( outOrientation );

	glm::vec4 newAxisX = rotation * glm::vec4( scale.x,		0.0f,		0.0f,		0.0f );
	glm::vec4 newAxisY = rotation * glm::vec4( 0.0f,		scale.y,	0.0f,		0.0f );
	glm::vec4 newAxisZ = rotation * glm::vec4( 0.0f,		0.0f,		scale.z,	0.0f );

	outWorld[0][0] = newAxisX.x;	outWorld[0][1] = newAxisX.y;	outWorld[0][2] = newAxisX.z;	outWorld[0][3] = 0.0f;
	outWorld[1][0] = newAxisY.x;	outWorld[1][1] = newAxisY.y;	outWorld[1][2] = newAxisY.z;	outWorld[1][3] = 0.0f;
	outWorld[2][0] = newAxisZ.x;	outWorld[2][1] = newAxisZ.y;	outWorld[2][2] = newAxisZ.z;	outWorld[2][3] = 0.0f;
	outWorld[3][0] = position.x;	outWorld[3][1] = position.y;	outWorld[3][2] = position.z;	outWorld[3][3] = 1.0f;
}

const rVector<unsigned int>& SSRender::GetGraphicsCulling( ) const
{
	return m_GraphicsCulling.GetUnitsSelected( );
}
