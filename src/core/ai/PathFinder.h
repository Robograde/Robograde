/**************************************************
Viktor Kelkkanen & Ola Enberg
***************************************************/

#pragma once
#include "Node.h"
#include <memory/Alloc.h>
#include <algorithm>
#include <glm/vec2.hpp>

class Pathfinder
{
public:
	Pathfinder();
	~Pathfinder();

	struct DirtyTile
	{
		int x;
		int y;
	};

	void				Init();
	rVector<Goal>		AStar(Tile* _currentTile, bool _avoidEntities, unsigned char _allowedCost, bool _squadPathFinding);
	void				CleanUpLists();

	bool				UnblockGoal();
	void				ConstructPath(Tile* _goal, rVector<Node> &closedList, rVector<Goal> &pathList, bool _avoidEntities, unsigned char _allowedCost);

	int					Estimate(Tile* _start, Tile* _goal);


	void				SetGoal(Tile* _t);
	void				SetStart(Tile* _t);
	void				SetStartFloat2( const glm::vec2& start ) { m_StartFloat2 = start; }

	bool				IsFinished() const { return m_Finished; }
	void				PrintListSizes();
	unsigned int		ClosedListSize();
	static Pathfinder*	GetInstance();

	//rVector<DirtyTile>& GetBadTiles();
private:
	struct ptr_less
	{
		template<class Node>
		bool operator()(const Node& left, const Node& right) const
		{
			return left.Fcost > right.Fcost;
		}
	};

	rVector<Node>											m_ClosedList;
	std::priority_queue<Node, rVector<Node>, ptr_less>		m_OpenQ;

	rVector<rVector<Node>>									m_Nodes;



	rVector<DirtyTile>										m_Dirty;
//	rVector<DirtyTile>										m_BadTiles;

	Tile*													m_Current;
	Tile*													m_Goal;
	Tile*													m_Start;

	glm::vec2												m_StartFloat2;

	bool													m_Finished;
	
	static const int										MAX_CLOSED_LIST_SIZE_SQUAD = 40000;
	static const int										MAX_CLOSED_LIST_SIZE_AGENT = 5000;
	static const int										COST_FACTOR_STRAIGHT = 100;
	static const int										COST_FACTOR_DIAGONAL = 141;
	int														m_GridWidth;
	int														m_GridHeight;
};

