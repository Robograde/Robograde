/**************************************************
2015 Ola Enberg
***************************************************/

#include "Picking.h"

#include <input/Input.h>
#include <glm/glm.hpp>
#include <collisionDetection/ICollisionDetection.h>
#include "../datadriven/DenseComponentCollection.h"
#include "../datadriven/EntityManager.h"
#include "../component/CollisionComponent.h"
#include "../utility/PlayerData.h"

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

#pragma push_macro("min")
#undef min

Picking::Picking()
{
	m_BoxSelectionGroups.push_back	( PICKING_TYPE_CONTROLLABLE		);

	m_LeftClickGroups.push_back		( PICKING_TYPE_CONTROLLABLE		);
	m_LeftClickGroups.push_back		( PICKING_TYPE_TERRAIN			);
	m_LeftClickGroups.push_back		( PICKING_TYPE_FRIEND			);
	m_LeftClickGroups.push_back		( PICKING_TYPE_ENEMY			);
	m_LeftClickGroups.push_back		( PICKING_TYPE_RESOURCE			);
	m_LeftClickGroups.push_back		( PICKING_TYPE_CONTROL_POINT	);

	m_RightClickGroups.push_back	( PICKING_TYPE_TERRAIN			);
	m_RightClickGroups.push_back	( PICKING_TYPE_ENEMY			);
	m_RightClickGroups.push_back	( PICKING_TYPE_RESOURCE			);
	m_RightClickGroups.push_back	( PICKING_TYPE_CONTROL_POINT	);

	m_AttackClickGroups.push_back	( PICKING_TYPE_TERRAIN			);
	m_AttackClickGroups.push_back	( PICKING_TYPE_ENEMY			);
}

void Picking::Update( const Camera& camera, PickingAction& outAction )
{
	outAction = PickingAction::None;
	m_UnitsSelected.clear();
	ivec2 mousePosition( g_Input->GetMousePosX(), g_Input->GetMousePosY() );

	// Start boxselecting.
	if ( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
	{
		m_BoxSelecting			= true;
		m_FirstSelectionPoint	= mousePosition;
	}

	// Update selection box position and size.
	if ( m_BoxSelecting )
	{
		m_SelectionBox.Position	= ivec2( glm::min( m_FirstSelectionPoint.x, mousePosition.x ),
										glm::min( m_FirstSelectionPoint.y, mousePosition.y ) );
		m_SelectionBox.Size		= ivec2( 1 + glm::abs( m_FirstSelectionPoint.x - mousePosition.x ),
										1 + glm::abs( m_FirstSelectionPoint.y - mousePosition.y ) );
	}	

	// Do the actual picking.
	if ( g_Input->MouseUpDown( MOUSE_BUTTON_RIGHT ) )
	{
		m_BoxSelecting = false;
		this->PickRay( m_RightClickGroups, camera, mousePosition );
	}
	else if ( g_PlayerData.GetLastActionPressed() == ACTION::ACTION_AI_ATTACK )
	{
		m_BoxSelecting = false;
		this->PickRay( m_AttackClickGroups, camera, mousePosition );
	}
	else if ( g_PlayerData.GetLastActionPressed( ) == ACTION::ACTION_PING )
	{
		m_BoxSelecting = false;
	}
	else if ( m_BoxSelecting && (this->m_SelectionBox.Size.x >= PICKING_FRUSTUM_MINIMUM || this->m_SelectionBox.Size.y >= PICKING_FRUSTUM_MINIMUM) )
	{
		this->PickFrustum( m_BoxSelectionGroups, camera, m_SelectionBox.Position, m_SelectionBox.Size );
		m_LastPickWasBox = true;
	}
	else
	{
		this->PickRay( m_LeftClickGroups, camera, mousePosition );
		m_LastPickWasBox = false;
	}

	// Decide how to handle the picking result.
	if ( m_UnitsSelected.size() > 0 )
	{
		if ( g_Input->MouseUpDown( MOUSE_BUTTON_RIGHT ) )
		{
			if ( g_EntityManager.GetEntityMask( m_UnitsSelected[0] ) & DenseComponentCollection<CollisionComponent>::GetInstance().GetComponentTypeFlag() )
			{
				CollisionComponent* collisionComp = GetDenseComponent<CollisionComponent>( m_UnitsSelected[0] );

				if ( collisionComp->CollisionEntity->GetGroupID() == PICKING_TYPE_ENEMY )
				{
					outAction = PickingAction::Attack;
				}
				else if ( collisionComp->CollisionEntity->GetGroupID() == PICKING_TYPE_RESOURCE )
				{
					outAction = PickingAction::Gather;
				}
				else if ( collisionComp->CollisionEntity->GetGroupID() == PICKING_TYPE_TERRAIN )
				{
					outAction = PickingAction::Move;
					m_UnitsSelected.clear();
				}
				else if ( collisionComp->CollisionEntity->GetGroupID() == PICKING_TYPE_CONTROL_POINT )
				{
					outAction = PickingAction::Capture;
				}
			}

			//reset attack key state
			g_PlayerData.SetLastActionPressed(ACTION_SIZE);
		}
		else if ( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT ) )
		{
			EntityMask entityMask		= g_EntityManager.GetEntityMask( m_UnitsSelected[0] );
			EntityMask collisionFlag	= DenseComponentCollection<CollisionComponent>::GetInstance().GetComponentTypeFlag();

			if ( g_PlayerData.GetLastActionPressed() == ACTION::ACTION_AI_ATTACK )
			{
				if ( (entityMask & collisionFlag) && GetDenseComponent<CollisionComponent>( m_UnitsSelected[0] )->CollisionEntity->GetGroupID() == PICKING_TYPE_TERRAIN )
				{
					outAction = PickingAction::AttackMove;
					m_UnitsSelected.clear();
				}
				else
				{
					outAction = PickingAction::Attack;
				}

				if ( g_Input->KeyUp( SDL_SCANCODE_LSHIFT ) )
					g_PlayerData.SetLastActionPressed(ACTION_SIZE);
			}
			else
			{
				if ( (entityMask & collisionFlag) && GetDenseComponent<CollisionComponent>( m_UnitsSelected[0] )->CollisionEntity->GetGroupID() == PICKING_TYPE_TERRAIN )
				{
					outAction = PickingAction::UnitDeselect;
					m_UnitsSelected.clear();
				}
				else
				{
					outAction = PickingAction::UnitSelection;
				}
			}
		}
	}
	
	if ( m_BoxSelecting && g_Input->MouseDownUp( MOUSE_BUTTON_LEFT, true ) )
	{
		m_BoxSelecting = false;
	}
}

void Picking::CalcTerrainPosUnderMouse( const Camera& camera, glm::vec3* outPosition ) const
{
	const ivec2 mousePosition( g_Input->GetMousePosX(), g_Input->GetMousePosY() );
	const ivec2 windowSize( camera.GetLens().WindowWidth, camera.GetLens().WindowHeight );
	const mat4 invViewProj	= glm::inverse( camera.GetViewProjection() );

	Ray ray;
	CalculateRayFromPixel( mousePosition, windowSize, invViewProj, &ray );
	
	rVector<unsigned int>	unitsSelected;
	rVector<int>			targetGroups;
	targetGroups.push_back( PICKING_TYPE_TERRAIN );

	g_CollisionDetection.PickingWithRay( ray.Position, ray.Direction, targetGroups, unitsSelected, outPosition );
}

const SelectionBox* Picking::GetSelectionBox() const
{
	return m_BoxSelecting ? &m_SelectionBox : nullptr;
}

const glm::vec3& Picking::GetClickedPosition() const
{
	return m_ClickedPosition;
}

const rVector<unsigned int>& Picking::GetUnitsSelected() const
{
	return m_UnitsSelected;
}

bool Picking::WasLastPickBox() const
{
	return m_LastPickWasBox;
}

bool Picking::IsBoxSelecting() const
{
	return m_BoxSelecting;
}

void Picking::PickRay( const rVector<int>& targetGroups, const Camera& camera, const glm::ivec2& pixel )
{
	const ivec2 windowSize	= ivec2( camera.GetLens().WindowWidth, camera.GetLens().WindowHeight );
	const mat4 invViewProj	= glm::inverse( camera.GetViewProjection() );

	Ray ray;
	CalculateRayFromPixel( pixel, windowSize, invViewProj, &ray );
	g_CollisionDetection.PickingWithRay( ray.Position, ray.Direction, targetGroups, m_UnitsSelected, &m_ClickedPosition );
}

void Picking::PickFrustum( const rVector<int>& targetGroups, const Camera& camera, const glm::ivec2& startPixel, const glm::ivec2& sizeInPixels )
{
	const ivec2 windowSize	= ivec2( camera.GetLens().WindowWidth, camera.GetLens().WindowHeight );
	const mat4 invViewProj	= glm::inverse( camera.GetViewProjection() );

	Frustum frustum;
	CalculateFrustum( camera, windowSize, invViewProj, startPixel, sizeInPixels, &frustum );
	g_CollisionDetection.PickingWithFrustum( frustum.Positions, frustum.Normals, targetGroups, m_UnitsSelected) ;
}

void Picking::ClearUnitSelection()
{
	m_UnitsSelected.clear();
}

void Picking::CalculateRayFromPixel( const glm::ivec2& pixel, const glm::ivec2& windowSize, const glm::mat4& invViewProj, Ray* outRay ) const
{
	// Clip space coordinates for the pixel. (-1,-1) in lower left corner, (-1,1) upper left corner, (1,-1) lower right corner. 
	const vec2	mousePosNorm	= vec2( -1.0f + 2.0f * (pixel.x / static_cast<float>(windowSize.x)),
											1.0f - 2.0f * (pixel.y / static_cast<float>(windowSize.y)) );

	// Translating pixel at near plane and far plane to world coordinates. Z-coordinate is depth into the screen (values between -1 and 1 are in view of camera).
	const vec4 nearHomogeneous	= invViewProj * vec4( mousePosNorm.x, mousePosNorm.y, 0.0f, 1.0f );
	const vec4 farHomogeneous	= invViewProj * vec4( mousePosNorm.x, mousePosNorm.y, 1.0f, 1.0f );
	const vec3 nearWorld		= vec3( nearHomogeneous ) / nearHomogeneous.w;
	const vec3 farWorld			= vec3( farHomogeneous ) / farHomogeneous.w;

	outRay->Position			= nearWorld;
	outRay->Direction			= glm::normalize( farWorld - nearWorld );
}

void Picking::CalculateFrustum( const Camera& camera, const glm::ivec2& windowSize, const glm::mat4& invViewProj, const glm::ivec2& startPixel, const glm::ivec2& sizeInPixels, Frustum* outFrustum ) const
{
	Ray cornerRays[4];

	// Specific order of corner rays makes creating frustum planes in loops easier for me.
	const std::pair<int,int> order[4] =
	{
		std::pair<int,int>( 0, 0 ),
		std::pair<int,int>( 1, 0 ),
		std::pair<int,int>( 1, 1 ),
		std::pair<int,int>( 0, 1 ),
	};

	// Create a ray for each corner of the box selection frustum.
	for ( int i = 0; i < 4; ++i )
	{
		const int x = order[i].first;
		const int y = order[i].second;
		const ivec2 pixel = startPixel + ivec2( x * sizeInPixels.x, y * sizeInPixels.y);
		CalculateRayFromPixel( pixel, windowSize, invViewProj, &cornerRays[i] );
	}

	// Calculate side frustum planes from the corner rays values.
	for ( int i = 0; i < 4; ++i )
	{
		const int neighbourIndex	= (i+1) % 4;
		const vec3 v1				= cornerRays[neighbourIndex].Position - cornerRays[i].Position;
		const vec3 v2				= cornerRays[i].Direction;

		outFrustum->Normals[i]		= glm::normalize( glm::cross( v1, v2 ) );
		outFrustum->Positions[i]	= cornerRays[i].Position;
	}

	// Calculate near frustrum plane from the corner rays values.
	{
		const vec3 v1				= cornerRays[0].Position - cornerRays[1].Position;
		const vec3 v2				= cornerRays[2].Position - cornerRays[1].Position;

		outFrustum->Normals[4]		= glm::normalize( glm::cross( v1, v2 ) );
		outFrustum->Positions[4]	= cornerRays[1].Position;
	}

	// Calculate far frustrum plane from the corner rays values.
	{
		const float cameraFar = camera.GetLens().Far;

		const vec3 farPoint[3] =
		{
			cornerRays[0].Position + cameraFar * cornerRays[0].Direction,
			cornerRays[1].Position + cameraFar * cornerRays[1].Direction,
			cornerRays[2].Position + cameraFar * cornerRays[2].Direction,
		};

		const vec3 v1				= farPoint[2] - farPoint[1];
		const vec3 v2				= farPoint[0] - farPoint[1];

		outFrustum->Normals[5]		= glm::normalize( glm::cross( v1, v2 ) );
		outFrustum->Positions[5]	= farPoint[1];
	}
}

#pragma pop_macro("min")
