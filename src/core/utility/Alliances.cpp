/**************************************************
Copyright 2015 Viktor Kelkannen & Johan Melin
***************************************************/

#include "Alliances.h"
#include <set>
#include "GlobalDefinitions.h"

Alliances& Alliances::GetInstance()
{
	static Alliances instance;
	return instance;
}

void Alliances::Initialize( int nrOfAlliances )
{
	m_Alliances.clear();
	m_Alliances.resize( nrOfAlliances );
}

void Alliances::Clear() 
{
	m_Alliances.clear();
}

int Alliances::CreateAlliance()
{
	rVector<int> alliance;
	m_Alliances.push_back( alliance );
	return static_cast<int>( alliance.size() - 1 );
}

void Alliances::AddToAlliance( int allianceIndex, int team )
{
	assert( allianceIndex >= 0 && allianceIndex < m_Alliances.size( ) );
	m_Alliances.at( allianceIndex ).push_back( team );
}

void Alliances::RemoveFromAlliance( int team )
{
	for ( unsigned int i = 0; i < m_Alliances.size(); i++ )
	{
		for ( unsigned int j = 0; j < m_Alliances[i].size(); j++ )
		{
			if ( m_Alliances[i][j] == team )
			{
				m_Alliances[i].erase( m_Alliances[i].begin() + j );

				return;
			}
		}
	}
}

bool Alliances::AlliedVictoryCheck( int team )
{
	int possibleWinner = -1;
	for ( unsigned int i = 0; i < m_Alliances.size(); ++i )
	{
		if ( m_Alliances[i].size() > 0 )
		{
			if ( possibleWinner == -1 )
				possibleWinner = i;
			else
				return false;
		}
	}

	return possibleWinner != -1 && IsAllied( team, m_Alliances[possibleWinner][0] );
}

rVector<int> Alliances::GetAllies( int team ) const
{
	rVector<int> allies;
	allies.push_back( team );

	for ( auto& allianceTeam : m_Alliances )
		for ( auto& allianceMember : allianceTeam )
			if ( allianceMember == team )
				for ( auto& member : allianceTeam )
					allies.push_back( member );

	std::sort( allies.begin( ), allies.end( ) );
	allies.erase( std::unique( allies.begin( ), allies.end( ) ), allies.end( ) );

	return allies;
}

rVector<rVector<int>> Alliances::GetAlliances() const
{
	return m_Alliances;
}

void Alliances::SetAlliances( const rVector<rVector<int>>& alliances )
{
	m_Alliances = alliances;
}

bool Alliances::IsAllied( int team1, int team2 ) const
{
	if ( team1 == team2 )
		return true;

	for ( unsigned int i = 0; i < m_Alliances.size(); i++ )
	{
		bool found1 = false;
		bool found2 = false;
		for ( unsigned int j = 0; j < m_Alliances[i].size(); j++ )
		{
			if ( m_Alliances[i][j] == team1 )
				found1 = true;
			if ( m_Alliances[i][j] == team2 )
				found2 = true;
		}

		if ( found1 && found2 )
			return true;
		else if ( found1 != found2 )
			return false;
	}

	return false;
}