/**************************************************
2015 David Pejtersen
***************************************************/
#include "CameraSpline.h"
#include <glm/gtx/spline.hpp>

static SplineID NextSplineID = 0;

void CameraSpline::Update( float deltaTime )
{
	if ( m_CurrentSpline != NO_SPLINE )
	{
		Spline* spline = nullptr;
		for ( auto& s : m_Splines )
		if ( s.ID == m_CurrentSpline )
			spline = &s;

		if ( spline != nullptr && !spline->Nodes.empty( ) )
		{
			m_Time += deltaTime;

			auto getNode =[&]( int id ) -> const SplineNode&
			{
				size_t size = spline->Nodes.size( );
				if ( id < 0 )
					return spline->Nodes.front( );
				if ( id >= size )
					return spline->Nodes.back( );
				return spline->Nodes[id];
			};

			for ( int i = 0; i < (int) spline->Nodes.size( ); ++i )
			{
				if ( getNode( i ).Time <= m_Time && m_Time < getNode( i + 1 ).Time )
				{
					float interpolationValue = (m_Time - getNode( i ).Time) / (getNode( i + 1 ).Time - getNode( i ).Time); // TODODP: Add safety check for division by zero

					SetPosition(
						glm::catmullRom(
							getNode( i - 1 ).Position,
							getNode( i ).Position,
							getNode( i + 1 ).Position,
							getNode( i + 2 ).Position,
							interpolationValue
							)
						);

					SetOrientation(
						glm::slerp(
							getNode( i ).Orientation,
							getNode( i + 1 ).Orientation,
							interpolationValue
							)
						);

					break;
				}
			}

			if ( m_Time > spline->Nodes.back( ).Time )
			{
				m_Time			= 0.0f;
				m_CurrentSpline = NO_SPLINE;
			}
		}
		else
			m_CurrentSpline = NO_SPLINE;
	}
}

void CameraSpline::Run( unsigned int id, float fromTime )
{
	m_Time			= fromTime;
	m_CurrentSpline = id;
}

void CameraSpline::Stop( )
{
	m_Time			= 0.0f;
	m_CurrentSpline = NO_SPLINE;
}

SplineID CameraSpline::NewSpline( const char* name )
{
	Spline s;
	s.ID	= NextSplineID++;
	s.Name	= name;

	m_Splines.push_back( s );
	return s.ID;
}

void CameraSpline::NewNode( SplineID id, float time, const glm::vec3& position, const glm::quat& orientation )
{
	SplineNode n;
	n.Time			= time;
	n.Position		= position;
	n.Orientation	= orientation;

	for ( auto& spline : m_Splines )
	if ( spline.ID == id )
	{
		bool inserted	= false;

		for ( size_t i = 0; i < spline.Nodes.size( ); ++i )
		if ( spline.Nodes[i].Time > n.Time )
		{
			spline.Nodes.insert( spline.Nodes.begin( ) + i, n );
			inserted = true;
			break;
		}

		if ( !inserted )
			spline.Nodes.push_back( n );
	}
}

void CameraSpline::DeleteSpline( SplineID id )
{
	for ( size_t i = 0; i < m_Splines.size( ); ++i )
	if ( m_Splines[i].ID == id )
	{
		m_Splines.erase( m_Splines.begin( ) + i );
		break;
	}
}

void CameraSpline::DeleteNode( SplineID splineID, size_t nodeID )
{
	for ( auto& spline : m_Splines )
		if ( spline.ID == splineID  && nodeID < spline.Nodes.size( ) )
			spline.Nodes.erase( spline.Nodes.begin( ) + nodeID );
}

SplineID CameraSpline::GetSplineID( const char* name ) const
{
	for ( auto& spline : m_Splines )
		if ( spline.Name == name )
			return spline.ID;
	return NO_SPLINE;
}

glm::vec3 CameraSpline::GetSplinePosition( SplineID id, float time ) const
{
	for ( auto& spline : m_Splines )
	{
		if ( spline.ID == id && !spline.Nodes.empty( ) )
		{
			auto getNode =[&]( int id ) -> const SplineNode&
			{
				size_t size = spline.Nodes.size( );
				if ( id < 0 )
					return spline.Nodes.front( );
				if ( id >= size )
					return spline.Nodes.back( );
				return spline.Nodes[id];
			};

			for ( int i = 0; i < (int) spline.Nodes.size( ); ++i )
			{
				if ( getNode( i ).Time <= time && time < getNode( i + 1 ).Time )
				{
					float interpolationValue = (time - getNode( i ).Time) / (getNode( i + 1 ).Time - getNode( i ).Time); // TODODP: Add safety check for division by zero

					return
						glm::catmullRom(
						getNode( i - 1 ).Position,
						getNode( i ).Position,
						getNode( i + 1 ).Position,
						getNode( i + 2 ).Position,
						interpolationValue
						);
				}
			}

			if ( time < spline.Nodes.front( ).Time )
				return spline.Nodes.front( ).Position;
			if ( time > spline.Nodes.back( ).Time )
				return spline.Nodes.back( ).Position;
		}
	}

	return{ 0.0f, 0.0f, 0.0f };
}

glm::quat CameraSpline::GetSplineOrientation( SplineID id, float time ) const
{
	for ( auto& spline : m_Splines )
	{
		if ( spline.ID == id && !spline.Nodes.empty( ) )
		{
			auto getNode =[&]( int id ) -> const SplineNode&
			{
				size_t size = spline.Nodes.size( );
				if ( id < 0 )
					return spline.Nodes.front( );
				if ( id >= size )
					return spline.Nodes.back( );
				return spline.Nodes[id];
			};

			for ( int i = 0; i < (int) spline.Nodes.size( ); ++i )
			{
				if ( getNode( i ).Time <= time && time < getNode( i + 1 ).Time )
				{
					float interpolationValue = (time - getNode( i ).Time) / (getNode( i + 1 ).Time - getNode( i ).Time); // TODODP: Add safety check for division by zero

					return
						glm::slerp(
						getNode( i ).Orientation,
						getNode( i + 1 ).Orientation,
						interpolationValue
						);
				}
			}

			if ( time < spline.Nodes.front( ).Time )
				return spline.Nodes.front( ).Orientation;
			if ( time > spline.Nodes.back( ).Time )
				return spline.Nodes.back( ).Orientation;
		}
	}

	return{ 0.0f, 0.0f, 0.0f, 1.0f };
}

float CameraSpline::GetMaxTime( SplineID id ) const
{
	for ( auto& spline : m_Splines )
		if ( spline.ID == id && !spline.Nodes.empty( ) )
			return spline.Nodes.back( ).Time;
	return 0.0f;
}

size_t CameraSpline::GetNumSplines( ) const
{
	return m_Splines.size( );
}

const CameraSpline::SplineInfo& CameraSpline::GetSplineInfoFromIndex( size_t index ) const
{
	static const CameraSpline::SplineInfo defaultSplineInfo = { NO_SPLINE, "" };

	if ( index >= m_Splines.size( ) )
		return defaultSplineInfo;
	return m_Splines[index];
}

size_t CameraSpline::GetNumNodes( SplineID spline ) const
{
	for ( auto& it : m_Splines )
		if ( it.ID == spline )
			return it.Nodes.size( );
	return 0ULL;
}

float CameraSpline::GetNodeTime( SplineID spline, size_t index ) const
{
	for ( auto& it : m_Splines )
		if ( it.ID == spline && index < it.Nodes.size( ) )
			return it.Nodes[index].Time;
	return 0.0f;
}

int CameraSpline::SetNodeTime( SplineID spline, size_t index, float time )
{
	for (auto& it : m_Splines )
	if ( it.ID == spline && index < it.Nodes.size( ) )
	{
		// TODODP: OPTIMIZE!!!
		SplineNode n = it.Nodes[index];
		DeleteNode( spline, index );
		NewNode( spline, time, n.Position, n.Orientation );

		n.Time = time;
		size_t size = it.Nodes.size( );
		for ( size_t i = 0; i < size; ++i )
			if ( n == it.Nodes[i] )
				return (int) i;
	}

	return 0;
}

glm::vec3 CameraSpline::GetNodePosition( SplineID spline, size_t index ) const
{
	for ( auto& it : m_Splines )
		if ( it.ID == spline && index < it.Nodes.size( ) )
			return it.Nodes[index].Position;
	return glm::vec3( 0.0f, 0.0f, 0.0f );
}

glm::quat CameraSpline::GetNodeOrientation( SplineID spline, size_t index ) const
{
	for ( auto& it : m_Splines )
		if ( it.ID == spline && index < it.Nodes.size( ) )
			return it.Nodes[index].Orientation;
	return glm::quat( 0.0f, 0.0f, 0.0f, 1.0f );
}

void CameraSpline::SetSplineName( SplineID id, const char* name )
{
	for ( auto& it : m_Splines )
		if ( it.ID == id )
			it.Name = name;
}