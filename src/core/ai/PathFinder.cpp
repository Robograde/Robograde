/**************************************************
2015 Viktor Kelkkanen & Ola Enberg
***************************************************/

#include "PathFinder.h"
#include <SDL2/SDL_timer.h>
#include <time.h>
#include "Agent.h"
#include "Terrain.h"
#include "../utility/GameData.h"

// #include <omp.h>
// #define OMP_NUM_THREADS 8

Pathfinder::Pathfinder()
{
	Init();
// 	omp_set_dynamic(1);
// 	omp_set_num_threads(8);
}

Pathfinder::~Pathfinder()
{
}

void Pathfinder::Init()
{
	//m_Nodes.clear(); // Causes desynchronization if used here. Probably dragons.

	m_Finished = true;

	m_Current = nullptr;
	m_Goal = nullptr;
	m_Start = nullptr;

	m_GridHeight = g_GameData.GetFieldHeight();
	m_GridWidth = g_GameData.GetFieldWidth();

	m_Nodes.resize( m_GridHeight );

	for ( int i = 0; i < m_GridHeight; ++i )
	{
		m_Nodes[i].resize( m_GridWidth );
	}

	m_StartFloat2 = glm::vec2( 0.0f, 0.0f );

	CleanUpLists();
}

void Pathfinder::SetGoal(Tile* _t)
{
	m_Goal = _t;
	m_Finished = false;
}

void Pathfinder::SetStart(Tile* _t)
{
	m_Start = _t;
	m_Finished = false;
}


rVector<Goal> Pathfinder::AStar(Tile* _agentTile,bool _avoidEntities, unsigned char _allowedCost, bool _squadPathFinding)
{

// 	uint64_t T0, T1;           // ticks
// 
// 	uint64_t calcTimeLimit = 0;
// 	float ms = 500;
// 	m_TimeLimit = (uint64_t)(m_ClockFreq*ms / (1000));
// 	T0 = SDL_GetPerformanceCounter();
	
	int counter = 0;
	Terrain* terrain = Terrain::GetInstance();
	rVector<Goal> pathList;
	Entity agent = terrain->WhoIsOnTile(_agentTile);
	//if first run, set current = start
	if (m_ClosedList.size() == 0)
	{
		m_Current = m_Start;
		m_Nodes[m_Start->Y][m_Start->X].Fcost = 0;
		m_Nodes[m_Start->Y][m_Start->X].TilePointer = m_Current;
		m_Nodes[m_Start->Y][m_Start->X].Parent = m_Current;

		DirtyTile dt;
		dt.x = m_Start->Y;
		dt.y = m_Start->X;

		m_Dirty.push_back(dt);
		m_OpenQ.push(m_Nodes[m_Start->Y][m_Start->X]);

// #if GENERATE_AI_MAP == 1
// 		if (terrain->IsUnpathableTerrain(m_Goal))
// 		{
// 			DirtyTile dt;
// 			dt.x = m_Goal->X;
// 			dt.y = m_Goal->Y;
// 			m_BadTiles.push_back(dt);
// 			return pathList;
// 		}
// #else
		if (!UnblockGoal())
		{
			return pathList;
		}
//#endif
	}

	m_Finished = false;

	while (true)
	{
		counter++;

// #pragma omp parallel num_threads(8)
//		{
			
//  #pragma omp critical
//  			printf("ID: %d, threads: %d, cpus: %d\n", omp_get_thread_num(), omp_get_num_threads(),omp_get_num_procs());

			//#pragma omp for
			for (int i = -1; i < 2; i++)
			{
				for (int j = -1; j < 2; j++)
				{
					if (i == 0 && j == 0)
						continue;

					int x, y;

					x = (*m_Current).X + j;
					y = (*m_Current).Y + i;

					if (x < 0 || y < 0 || x > m_GridWidth - 1 || y > m_GridHeight - 1)
					{
						continue;
					}

					if (m_Nodes[x][y].Closed)
						continue;

					Tile* t = Terrain::GetInstance()->GetTile(x, y);

					//unwalkable
					if (Terrain::GetInstance()->IsUnpathableTerrain(t))
						continue;

					if(_avoidEntities)
					{
						if(Terrain::GetInstance()->IsOccupiedByNonEvasiveUnitExclude(t, agent))
							continue;
					}

					int diagonal = i*j;

					int gCost = 0;

					//use integers instead of float, multiply 100. 141 == approx sqrt(2)
					diagonal == 0 ? gCost = COST_FACTOR_STRAIGHT : gCost = COST_FACTOR_DIAGONAL;

					if (t->Cost < _allowedCost)
						gCost += ((_allowedCost - t->Cost) * COST_FACTOR_STRAIGHT) * 4;

					m_Nodes[x][y].Fcost = gCost + Estimate(t, m_Goal);
					m_Nodes[x][y].TilePointer = t;
					m_Nodes[x][y].Parent = m_Current;
					m_Nodes[x][y].Closed = true;

//#if AI_DEBUG == 1
//					Terrain::GetInstance()->GetTile(x, y)->DebugInfo = 1;
//#endif

// 					#pragma omp critical
// 					{
						m_OpenQ.push(m_Nodes[x][y]);

						DirtyTile dt;
						dt.x = x;
						dt.y = y;

						m_Dirty.push_back(dt);
			//		}
				}
			}
	//	}

		Node n = m_OpenQ.top();
		Tile* t = n.TilePointer;
		
		m_ClosedList.push_back(n);
		m_Current = n.TilePointer;
		//terrain->SetTerrain(t->x, t->y, 'C');

		m_OpenQ.pop();

// 		T1 = SDL_GetPerformanceCounter();
// 		calcTimeLimit += (T1 - T0)/**1000 / m_clockFreq */;

		if (t == m_Goal /*|| calcTimeLimit > m_TimeLimit*/ || (m_ClosedList.size() > MAX_CLOSED_LIST_SIZE_SQUAD && _squadPathFinding) || (m_ClosedList.size() > MAX_CLOSED_LIST_SIZE_AGENT && !_squadPathFinding) || m_OpenQ.empty())
		{
// #if AI_DEBUG == 1
// 			if (calcTimeLimit > m_TimeLimit)
// 				printf("Time limit break %.4fms\n", static_cast<double>((T1 - T0) * 1000) / (m_ClockFreq));
// 
// #endif
// 			if (calcTimeLimit > m_TimeLimit)
// 			{
// 				ConstructPath(m_Goal, m_ClosedList, pathList);
// 				break;
// 			}
// 			else
				m_Finished = true;

// 				if (m_ClosedList.size() > MAX_CLOSED_LIST_SIZE)
// 					printf("closed list too big\n");
				

//#if GENERATE_AI_MAP == 0
// 			if (m_OpenQ.empty())
// 				printf("NO PATH FOUND\n");
			ConstructPath(m_Goal, m_ClosedList, pathList,_avoidEntities,_allowedCost);
// #else
// 			if (m_ClosedList.size() > MAX_CLOSED_LIST_SIZE_SQUAD || m_OpenQ.empty())
// 			{
// 				//printf("A* Early exit, reached max size on closed list. %d\n", MAX_CLOSED_LIST_SIZE);
// 				DirtyTile dt;
// 				dt.x = m_Goal->X;
// 				dt.y = m_Goal->Y;
// 				m_BadTiles.push_back(dt);
// 			}
// #endif
			

			break;
		}
	}

	//cleanup
	if (m_Finished)
		CleanUpLists();

	return pathList;
}

// rVector<Pathfinder::DirtyTile>& Pathfinder::GetBadTiles()
// {
// 	return m_BadTiles;
// }

int Pathfinder::Estimate(Tile* _start, Tile* _goal)
{
	double dist = sqrt(pow(_start->X - _goal->X,2) + pow(_start->Y - _goal->Y,2));

	return (int)((dist)*COST_FACTOR_STRAIGHT);
}

void Pathfinder::ConstructPath(Tile* _goal, rVector<Node> &closedList, rVector<Goal> &pathList, bool _avoidEntities, unsigned char _allowedCost)
{
	
	Terrain* terrain = Terrain::GetInstance();
	Tile* lastTile = closedList[closedList.size() - 1].TilePointer;
	pathList.push_back(lastTile);
	for (int i = (int)(closedList.size() - 1); i > -1; i--)
	{
		if (closedList[i].Parent == nullptr)
			continue;

		int dx = abs(closedList[i].Parent->X - lastTile->X);
		int dy = abs(closedList[i].Parent->Y - lastTile->Y);

		if (dx > 1 || dy > 1)
			continue;

		lastTile = closedList[i].Parent;
		pathList.push_back(lastTile);
	}

#if AI_DEBUG == 1
	for (int i = 0; i < pathList.size(); i++)
	{
		pathList[i].Tile->DebugInfo = 14;
	}
#endif

	//cut PATH
	int costCutoff = 0;
	for ( int i = (int)(pathList.size() - 1); i > 1; --i )
	{
		for ( int j = 0; j < i - 1; ++j )
		{
			costCutoff = glm::min( _allowedCost, glm::min( pathList[i].Tile->Cost, pathList[j].Tile->Cost ) );
			if (terrain->FreePathBool(pathList[i].Tile, pathList[j].Tile, _avoidEntities, terrain->WhoIsOnTile(m_Start), costCutoff, m_StartFloat2))
			{
				// erase from (j+1) to (i-1))
				pathList.erase( pathList.begin() + (j+1), pathList.begin() + (i) );
				i = j;
				break;
			}
		}
	}

#if AI_DEBUG == 1
	for (int i = 0; i < pathList.size(); i++)
	{
		pathList[i].Tile->DebugInfo = 15;
	}
#endif
}

//attempt to find unblocked goal tile close to selected one
bool Pathfinder::UnblockGoal()
{
	int counter = 0;

	if (Terrain::GetInstance()->IsUnpathableTerrain(m_Goal) || Terrain::GetInstance()->IsOccupiedByNonEvasiveUnitExclude(m_Goal, Terrain::GetInstance()->WhoIsOnTile(m_Start)))
	{
// #if AI_DEBUG == 1
// 		printf("bad goal (%d,%d)\n",m_Goal->X,m_Goal->Y);
// #endif
		Tile* newGoal = Terrain::GetInstance()->GetMovableTile((float)m_Goal->X + 0.5f, (float)m_Goal->Y + 0.5f, Terrain::GetInstance()->WhoIsOnTile(m_Start), m_Start);
		if (newGoal == m_Goal)
		{
#if AI_DEBUG == 1
			printf("failed to unblock\n");
#endif
			return 0;
		}
		m_Goal = newGoal;
//  #if AI_DEBUG == 1
// 		printf("new goal (%d,%d)\n", m_Goal->X, m_Goal->Y);
//  #endif
	}

	if (Terrain::GetInstance()->IsUnpathableTerrain(m_Start))
	{
		m_Start = Terrain::GetInstance()->GetMovableTile((float)m_Start->X, (float)m_Start->Y, Terrain::GetInstance()->WhoIsOnTile(m_Start),m_Start);
		//printf("Start is blocked, should not happen\n");
		return 0;
	}

	return 1;
}

//cleanup all lists when goal is reached
void Pathfinder::CleanUpLists()
{
	for (int i = (int)(m_OpenQ.size() - 1); i > -1; i--)
	{
		m_OpenQ.pop();
	}

	m_ClosedList.clear();

	for (int i = 0; i < m_Dirty.size(); i++)
	{
		int x = m_Dirty[i].x;
		int y = m_Dirty[i].y;

		m_Nodes[x][y].Fcost = 0;
		m_Nodes[x][y].Parent = nullptr;
		m_Nodes[x][y].TilePointer = nullptr;
		m_Nodes[x][y].Closed = false;
	}

	m_Dirty.clear();

	m_Goal = nullptr;
	m_Start = nullptr;
}

void Pathfinder::PrintListSizes()
{
	printf("Finished: %d\n", m_Finished);
	printf("Open Queue: %d\n", static_cast<int>(m_OpenQ.size()));
	printf("Closed List: %d\n", static_cast<int>(m_ClosedList.size()));
	int nodes = 0;
	for (int i = 0; i < m_GridWidth; i++)
	{
		for (int j = 0; j < m_GridHeight; j++)
		{
			if (m_Nodes[i][j].TilePointer != nullptr)
			{
				nodes++;
			}
		}
	}
	printf("Nodes: %d\n", nodes);
}

unsigned int Pathfinder::ClosedListSize()
{
	return static_cast<unsigned int>(m_ClosedList.size());
}

Pathfinder* Pathfinder::GetInstance()
{
	static Pathfinder pf;
	return &pf;
}