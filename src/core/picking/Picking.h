/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory/Alloc.h>
#include "../camera/Camera.h"
#include "PickingType.h"

#define PICKING_FRUSTUM_MINIMUM	4

struct SelectionBox
{
	glm::ivec2		Position	= glm::ivec2( -1, -1 );
	glm::ivec2		Size		= glm::ivec2( 0, 0 );
};

enum class PickingAction
{
	None,
	UnitSelection,
	UnitDeselect,
	Move,
	AttackMove,
	Attack,
	Gather,
	Capture
};

class Picking
{
public:
										Picking();

	void								Update					( const Camera& camera, PickingAction& outAction );

	void								CalcTerrainPosUnderMouse( const Camera& camera, glm::vec3* outPosition ) const;
	void								PickRay					( const rVector<int>& targetGroups, const Camera& camera, const glm::ivec2& pixel );
	void								PickFrustum				( const rVector<int>& targetGroups, const Camera& camera, const glm::ivec2& startPixel, const glm::ivec2& sizeInPixels );
	void								ClearUnitSelection		();

										// Window coordinates. Returns nullptr if user isn't currently box selecting.
	const SelectionBox*					GetSelectionBox			() const;
	const glm::vec3&					GetClickedPosition		() const;
	const rVector<unsigned int>&		GetUnitsSelected		() const;

	bool								WasLastPickBox			() const;
	
	bool								IsBoxSelecting			() const;

private:
	struct								Ray;
	struct								Frustum;

	void								CalculateRayFromPixel	( const glm::ivec2& pixel, const glm::ivec2& windowSize, const glm::mat4& invViewProj, Ray* outRay ) const;
	void								CalculateFrustum		( const Camera& camera, const glm::ivec2& windowSize, const glm::mat4& invViewProj, const glm::ivec2& startPixel, const glm::ivec2& sizeInPixels, Frustum* outFrustum ) const;

	bool								m_BoxSelecting			= false;
	bool								m_LastPickWasBox		= false;
	glm::ivec2							m_FirstSelectionPoint	= glm::ivec2( -1, -1 );
	glm::vec3							m_ClickedPosition		= glm::vec3( 0.0f );
	SelectionBox						m_SelectionBox;
	rVector<unsigned int>				m_UnitsSelected;

	rVector<int>						m_LeftClickGroups;
	rVector<int>						m_BoxSelectionGroups;
	rVector<int>						m_RightClickGroups;
	rVector<int>						m_AttackClickGroups;
};

struct Picking::Ray
{
	glm::vec3	Position;
	glm::vec3	Direction;
};

struct Picking::Frustum
{
	glm::vec3	Positions[6];
	glm::vec3	Normals[6];
};