/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <messaging/Subscriber.h>
#include "../../picking/PickingType.h"
#include "../../datadriven/ComponentTypes.h"

// Conveniency access function
#define g_SSEditorToolbox SSEditorToolbox::GetInstance()

const glm::vec3 INVALID_PICKED_POSITON = glm::vec3( -1.0f );

enum class Tool
{
	None,
	MoveTool,
	SelectionTool,
	PlaceProp,
	PlaceResouce,
	PlaceControlPoint,
	PlaceSFXEmitter,
	PlaceParticleEmitter,
	TerrainEditTool
};

class Camera;
struct Ray;
class SSEditorToolbox : public Subsystem, public Subscriber
{
public:
	static SSEditorToolbox&			GetInstance();

	void							Startup() override;
	void							Shutdown() override;
	void							UpdateUserLayer( const float deltaTime ) override;
	void							UpdateSimLayer( const float timeStep ) override;
	void							Reset();

	Tool							GetSelectedTool() const;
	const glm::vec3&				GetPickedPosition() const;
	Entity							GetMouseOverEntity() const;
	Entity							GetSelectedEntity() const;

	void							SetLocalPlacementGhostVisible( bool visible );
	void							SetLocalPlacementGhostModel( const rString& modelFileName );

	bool							IsToolSelected( Tool toCompare );
	bool							IsToolSelected( const rVector<Tool>& toCompare );

	void							SelectTool( Tool newTool );

	glm::ivec2						WorldToScreen( const glm::vec3& worldPos );

private:
	// No external instancing allowed
	SSEditorToolbox() : Subsystem( "EditorToolbox" ), Subscriber( "EditorToolbox" ) {}
	SSEditorToolbox( const SSEditorToolbox& rhs );
	~SSEditorToolbox() {};
	SSEditorToolbox& operator=( const SSEditorToolbox& rhs );

	void					CalcTerrainPosUnderMouse( const Camera& camera, glm::vec3* const outPosition ) const;
	void					PickRelevantEntities( const Camera& camera, const rVector<int>& targetGroups, rVector<unsigned int>& pickedEntities );
	void					CalculateRayFromPixel( const glm::ivec2& pixel, const glm::ivec2& windowSize, const glm::mat4& invViewProj, Ray* outRay ) const;

	void					OnToolSelected( Tool tool );
	void					OnToolDeSelected( Tool tool );

	void					AddTargetGroup( ENTITY_TYPE targetGroup );
	void					RemoveTargetGroup( ENTITY_TYPE targetGroup );

	Tool					m_SelectedTool			= Tool::None;
	glm::vec3				m_PickedPosition		= INVALID_PICKED_POSITON;
	rVector<int>			m_PickingTargetGroups;
	Entity					m_MouseOverEntity		= ENTITY_INVALID;

	rMap<short, Entity>		m_PlacementGhostEntities;	// PlayerID -> PlacementGhostEntityIDs
	rMap<short, Entity>		m_SelectedEntities;			// PlayerID -> SelectedEntityIDs
};

struct Ray
{
	glm::vec3	Position;
	glm::vec3	Direction;
};