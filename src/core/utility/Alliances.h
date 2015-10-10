/**************************************************
Copyright 2015 Viktor Kelkannen & Johan Melin
***************************************************/

#pragma once
#include <memory/Alloc.h>

#define g_Alliances		Alliances::GetInstance()

class Alliances
{
public:
	static Alliances& GetInstance();

	void					Initialize( int nrOfAlliances );
	void 					Clear();
	int 					CreateAlliance();
	void 					AddToAlliance( int allianceIndex, int team );
	void					RemoveFromAlliance( int team );
	bool					AlliedVictoryCheck( int team );

	rVector<int>			GetAllies( int team ) const;
	rVector<rVector<int>>	GetAlliances() const;

	void					SetAlliances( const rVector<rVector<int>>& alliances );

	bool					IsAllied( int team1, int team2 ) const;
	
private:
	// No external instancing allowed
	Alliances() { }
	Alliances(const Alliances& rhs);
	~Alliances() { }
	Alliances& operator= (const Alliances& rhs);

	rVector<rVector<int>> m_Alliances;
};