/**************************************************
2015 David Pejtersen
***************************************************/
#pragma once
#include <memory/Alloc.h>
#include <glm/glm.hpp>
#include "Camera.h"

#define NO_SPLINE ~0

typedef unsigned int SplineID;

class CameraSpline : public Camera
{
public:
	struct SplineInfo
	{
		SplineID ID;
		rString Name;
	};

	CameraSpline( ) { }
	~CameraSpline( ) { }

	void				Update( float deltaTime ) override;

	void				Run( unsigned int id, float fromTime = 0.0f );
	bool				Running( ) { return m_CurrentSpline != NO_SPLINE; }
	
	void				Stop( );

	SplineID			NewSpline( const char* name );
	void				NewNode( SplineID id, float time, const glm::vec3& position, const glm::quat& orientation );

	void				DeleteSpline( SplineID id );
	void				DeleteNode( SplineID splineID, size_t nodeID );

	glm::vec3			GetSplinePosition( SplineID id, float time ) const;
	glm::quat			GetSplineOrientation( SplineID id, float time ) const;

	SplineID			GetSplineID( const char* name ) const;
	float				GetMaxTime( SplineID id ) const;
	float				GetRunTime( ) const { return m_Time; }

	size_t				GetNumSplines( ) const;
	const SplineInfo&	GetSplineInfoFromIndex( size_t index ) const;

	size_t				GetNumNodes( SplineID spline ) const;
	float				GetNodeTime( SplineID spline, size_t index ) const;
	int					SetNodeTime( SplineID spline, size_t index, float time );

	glm::vec3			GetNodePosition( SplineID id, size_t index ) const;
	glm::quat			GetNodeOrientation( SplineID id, size_t index ) const;

	void				SetSplineName( SplineID id, const char* name );

private:
	struct SplineNode
	{
		float		Time;
		glm::vec3	Position;
		glm::quat	Orientation;
	
		bool operator==(const SplineNode& rhs) const
		{
			return this->Time == rhs.Time && this->Position == rhs.Position && this->Orientation == rhs.Orientation;
		}
	};

	struct Spline : public SplineInfo
	{
		rVector<SplineNode> Nodes;
	};

	rVector<Spline>	m_Splines;
	SplineID		m_CurrentSpline = NO_SPLINE;
	float			m_Time = 0.0f;
};