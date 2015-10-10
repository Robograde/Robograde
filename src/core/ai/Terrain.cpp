/**************************************************
2015 Viktor Kelkkanen & Ola Enberg
***************************************************/

#include <ostream>
#include "Terrain.h"
#include <time.h>
#include "../subsystem/gamelogic/SSAI.h"
#include "../subsystem/gamelogic/SSControlPoint.h"
#include "../subsystem/gfx/SSFogOfWar.h"
#include "../utility/GameData.h"
#include "../utility/Alliances.h"
#include <gfx/GraphicsEngine.h>

#if AI_DEBUG == 1
	#include <profiler/Profiler.h>
	#include <profiler/AutoProfiler.h>
#endif

#if GENERATE_AI_MAP == 1
#include <iostream>
#include <fstream>
#include "PathFinder.h"
#include <omp.h>
#endif

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

//size only makes the plus bigger at the moment, not a circle
void Terrain::UpdateSensor(Entity _entity, Tile* _tile, Tile* _prevTile, int _size, bool _destroy)
{
	m_UpdateID++;

	//Manage resource list
	if (HasComponent<ResourceComponent>(_entity))
	{
		if (!_destroy)
		{
			//some TA put resources on non-walkable terrain, don't add them
			if (!IsUnpathableTerrain(_tile))
				m_Resources.push_back(GetDenseComponent<ResourceComponent>(_entity));
		}
		else
		{
			for (int i = 0; i < m_Resources.size(); i++)
			{
				if (m_Resources[i]->EntityID == _entity)
				{
					m_Resources.erase(m_Resources.begin()+i);
				}
			}
		}
	}

	int x = 0;
	int y = 0;

	//remove completely from sensor
	if (_destroy)
	{
		m_Entities[_tile->Y][_tile->X] = ENTITY_INVALID;
		
		for (int i = 0; i < _size * 2 + 1; i++)
		{
			for (int j = 0; j < _size * 2 + 1; j++)
			{
				x = _tile->X - _size + j;
				y = _tile->Y - _size + i;

				if (x < 0 || y < 0 || x > m_GridWidth - 1 || y > m_GridHeight - 1)
					continue;

				if (j + i >= (_size) && j <= (_size + i) && i <= (_size + j) && j + i <= (_size * 3))
					if (m_Entities[y][x] == _entity) 
						m_Entities[y][x] = ENTITY_INVALID;
			}
		}
		return;
	}



	//update previous tile
	if (m_Entities[_prevTile->Y][_prevTile->X] == _entity) m_Entities[_prevTile->Y][_prevTile->X] = ENTITY_INVALID;

	for (int i = 0; i < _size * 2 + 1; i++)
	{
		for (int j = 0; j < _size * 2 + 1; j++)
		{
			if (j + i >= (_size) && j <= (_size + i) && i <= (_size + j) && j + i <= (_size * 3))
			{
				x = _prevTile->X - _size + j;
				y = _prevTile->Y - _size + i;

				if (x < 0 || y < 0 || x > m_GridWidth - 1 || y > m_GridHeight - 1)
					continue;

				if (m_Entities[y][x] == _entity)
					m_Entities[y][x] = ENTITY_INVALID;
			}
		}
	}

	//update current tile
	if (m_Entities[_tile->Y][_tile->X] == ENTITY_INVALID) m_Entities[_tile->Y][_tile->X] = _entity;

	for (int i = 0; i < _size * 2 + 1; i++)
	{
		for (int j = 0; j < _size * 2 + 1; j++)
		{
			if (j + i >= (_size) && j <= (_size + i) && i <= (_size + j) && j + i <= (_size * 3))
			{
				x = _tile->X - _size + j;
				y = _tile->Y - _size + i;

				if (x < 0 || y < 0 || x > m_GridWidth - 1 || y > m_GridHeight - 1)
					continue;

				if (m_Entities[y][x] == ENTITY_INVALID)
					m_Entities[y][x] = _entity;
			}
		}
	}
}

Entity Terrain::WhoIsOnTile(Tile* _tile)
{
	return m_Entities[_tile->Y][_tile->X];
}

void Terrain::InitField()
{
	m_UpdateID = 0;

	m_GridHeight = g_GameData.GetFieldHeight();
	m_GridWidth = g_GameData.GetFieldWidth();

	m_Field.clear();

	m_Field.resize(m_GridHeight);

	for (int i = 0; i < m_GridHeight; ++i)
	{
		m_Field[i].clear();
		m_Field[i].resize(m_GridWidth);
	}

	m_Entities.clear();
	m_Entities.resize(m_GridHeight);

	for (int i = 0; i < m_GridHeight; ++i)
	{
		m_Entities[i].clear();
		m_Entities[i].resize(m_GridWidth);
	}

#if AI_GENERATE_MAP == 1
	m_Heights.clear();
	m_Heights.resize(m_GridHeight);

	for (int i = 0; i < m_GridHeight; ++i)
	{
		m_Heights[i].clear();
		m_Heights[i].resize(m_GridWidth);
	}
#endif

	m_Resources.clear();

#if GENERATE_AI_MAP == 1
	Terrain::GetInstance()->WriteAIMapToFile();
#else
	Terrain::GetInstance()->LoadAIMap();
#endif
}

Terrain* Terrain::GetInstance()
{
	static Terrain terrain;
	return &terrain;
}

char Terrain::GetTerrain(int _x, int _y)
{
	Tile* t = GetTile(_x, _y);

	return t->Terrain;
}

void Terrain::SetTerrain(int _x, int _y, char _terrain)
{
	m_Field[_y][_x].Terrain = _terrain;
}

Tile* Terrain::GetTile(float _x, float _y)
{
	_x = floor(_x + 0.5f);
	_y = floor(_y + 0.5f);

    if (_x >= m_GridWidth)
		_x = (float)m_GridWidth-1.0f;

    if (_x < 0)
		_x = 0;

    if (_y >= m_GridHeight)
		_y = (float)m_GridHeight - 1.0f;

	if (_y < 0)
		_y = 0;

	return &m_Field[static_cast<int>(_y)][static_cast<int>(_x)];
}

Tile* Terrain::GetTile(int _x, int _y)
{
	if (_x >= m_GridWidth)
		_x = m_GridWidth - 1;

	if (_x < 0)
		_x = 0;

	if (_y >= m_GridHeight)
		_y = m_GridHeight - 1;

	if (_y < 0)
		_y = 0;

	return &m_Field[_y][_x];
}

bool Terrain::OutOfBounds(float _x, float _y)
{
	if (static_cast<int>(_x) > m_GridWidth-1)
		return true;

	if (static_cast<int>(_x) < 0)
		return true;

	if (static_cast<int>(_y) > m_GridHeight-1)
		return true;

	if (static_cast<int>(_y) < 0)
		return true;

	return false;
}

bool Terrain::AdjacentWallsBool(int _x, int _y, bool _avoidEntities)
{
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			int x, y;

			x = _x + j;
			y = _y + i;
			Tile* t = GetTile(x, y);
			if (IsUnpathableTerrain(t) || (_avoidEntities && IsOccupiedByNonEvasiveUnit(t)))
				return true;
		}
	}

	return false;
}

bool Terrain::AdjacentCostBool(int _x, int _y,int _cost)
{
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			int x, y;

			x = _x + j;
			y = _y + i;
			Tile* t = GetTile(x, y);
			if (t->Cost <= _cost)
				return true;
		}
	}

	return false;
}

int Terrain::AdjacentWallsInt(int _x, int _y)
{
	int neighbours = 0;

	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			if (j == 0 && i == 0)
				continue;

			int x, y;

			x = _x + j;
			y = _y + i;

			Tile* t = GetTile(x, y);
			if (IsUnpathableTerrain(t) || IsOccupiedByNonEvasiveUnit(t))
				neighbours++;
		}
	}

	return neighbours;
}

// bool Terrain::FreePathBool(Tile* _start, Tile* _goal, bool _avoidUnits, Entity _entity, char _allowedCost)
// {
// 	int sx = _start->X;
// 	int sy = _start->Y;
// 	int gx = _goal->X;
// 	int gy = _goal->Y;
// 
// 	//vertical
// 	if (sx == gx)
// 	{
// 		int sign = 1;
// 		int lim = gy - sy;
// 		if (lim < 0)
// 		{
// 			sign = -1;
// 			lim *= (-1);
// 		}
// 		for (int i = 0; i < lim; i++)
// 		{
// 			Tile* t = GetTile(_start->X, _start->Y + i*sign);
// 			if (IsUnpathableTerrain(t) || IsOccupiedByNonEvasiveUnit(t) || (_avoidUnits && IsOccupiedByUnitExclude(t, _entity)))
// 				return false;
// 		}
// 		return true;
// 	}
// 	//horizontal
// 	else if (sy == gy)
// 	{
// 		int sign = 1;
// 		int lim = gx - sx;
// 		if (lim < 0)
// 		{
// 			sign = -1;
// 			lim *= (-1);
// 		}
// 		for (int i = 0; i < lim; i++)
// 		{
// 			Tile* t = GetTile(_start->X + i*sign, _start->Y);
// 			if (IsUnpathableTerrain(t) || IsOccupiedByNonEvasiveUnit(t) || (_avoidUnits && IsOccupiedByUnitExclude(t, _entity)))
// 				return false;
// 		}
// 		return true;
// 	}
// 	//diagonal
// 	else if (abs(gx - sx) == abs(gy - sy))
// 	{
// 		int limx = gx - sx;
// 		int limy = gy - sy;
// 		int signx = 1;
// 		int signy = 1;
// 		if (limx < 0)
// 		{
// 			signx = -1;
// 			limx *= (-1);
// 		}
// 		if (limy < 0)
// 		{
// 			signy = -1;
// 			limy *= (-1);
// 		}
// 		for (int i = 0; i < limx; i++)
// 		{
// 			Tile* t = GetTile(_start->X + i*signx, _start->Y + i*signy);
// 			if (IsUnpathableTerrain(t) || IsOccupiedByNonEvasiveUnit(t)/* || (_avoidUnits && IsOccupiedByUnitExclude(t, _entity))*/)
// 				return false;
// 		}
// 		return true;
// 	}
// 	else
// 	{
// 		Tile* t = _start;
// 
// 		while (true)
// 		{
// 			int x = 0, y = 0;
// 			if (_goal->X != t->X)
// 				x = _goal->X - t->X;
// 			if (_goal->Y != t->Y)
// 				y = _goal->Y - t->Y;
// 
// 			if (x > 1)
// 				x = 1;
// 			if (x < -1)
// 				x = -1;
// 
// 			if (y > 1)
// 				y = 1;
// 			if (y < -1)
// 				y = -1;
// 
// 			t = GetTile(t->X + x, t->Y + y);
// 
// 			if (IsUnpathableTerrain(t) || (_avoidUnits && IsOccupiedByUnitExclude(t, _entity)))
// 				return false;
// 
// 			if (x == 0 && y == 0)
// 				return true;
// 		}
// 	}
// 
// 	return false;
// }

//no obstacles between _start and _goal
bool Terrain::FreePathBool(Tile* _start, Tile* _goal, bool _avoidUnits, Entity _entity, char _allowedCost, const glm::vec2& startPosition)
{
	const float stepSize = 0.2f;

	Tile*		startPosTile	= GetTile( startPosition.x, startPosition.y );
	glm::vec2	basePosition	= _start == startPosTile ? startPosition : glm::vec2( _start->X, _start->Y );

 	glm::vec2	distance( _goal->X - basePosition.x, _goal->Y - basePosition.y );
	float		lengthToGoal = glm::length( distance );

	if ( lengthToGoal == 0.0f )
		return true;

	glm::vec2	direction = distance / lengthToGoal;

	float		currentDistance = 0.0f;
	glm::vec2	currentPosition = basePosition;
	Tile*		previousTile	= nullptr;

	while ( currentDistance < lengthToGoal + 1.0f )
	{
		currentDistance		+= stepSize;
		currentPosition		= basePosition + currentDistance * direction;
		Tile* currentTile	= GetTile( currentPosition.x, currentPosition.y );

		if ( currentTile == previousTile )
			continue;

		if ( currentTile == _goal )
			return true;

		if (IsUnpathableTerrain(currentTile) || currentTile->Cost < _allowedCost || (_avoidUnits && IsOccupiedByNonEvasiveUnit(currentTile)) || (_avoidUnits && IsOccupiedByUnitExclude(currentTile,_entity)))
			return false;

		previousTile = currentTile;
	}
	return true;

//#if AI_DEBUG == 1
//	PROFILE(AutoProfiler AIFreePathBool("Terrain_FreePathBool",Profiler::PROFILER_CATEGORY_STANDARD,true));
//#endif
//
//	const glm::ivec2	start( _start->X, _start->Y );
//	const glm::ivec2	goal ( _goal->X, _goal->Y );
//	const glm::ivec2	distance = goal - start;
//	const glm::ivec2	absDist( glm::abs(goal.x - start.x), glm::abs(goal.y - start.y) );
//	const glm::ivec2	direction( distance.x > 0.0f ? 1 : -1, distance.y > 0.0f ? 1 : -1 );
//
//	rVector<std::pair<float, glm::ivec2>> directionOrder;
//	directionOrder.reserve( absDist.x + absDist.y );
//
//	for ( int i = 1; i < absDist.x; ++i )
//	{
//		directionOrder.push_back( { i / static_cast<float>(absDist.x), glm::ivec2( direction.x, 0 ) } );
//	}
//	for ( int i = 1; i < absDist.y; ++i )
//	{
//		directionOrder.push_back( { i / static_cast<float>(absDist.y), glm::ivec2( 0, direction.y ) } );
//	}
//	std::sort( directionOrder.begin(), directionOrder.end(), [](std::pair<float, glm::ivec2> a, std::pair<float, glm::ivec2> b) -> bool { return a.first < b.first; } );
//
//	Tile* currentTile = GetTile(	start.x + (direction.x > 0 ? 0 : 1),
//									start.y + (direction.y > 0 ? 0 : 1) );
//
//	if (IsUnpathableTerrain(currentTile) || currentTile->Cost < _allowedCost || IsOccupiedByNonEvasiveUnit(currentTile) || (_avoidUnits && IsOccupiedByUnitExclude(currentTile,_entity)))
//	{
//#if AI_DEBUG == 1
//		PROFILE(AIFreePathBool.Stop());
//#endif
//		return false;
//	}
//
//	for ( auto& direction : directionOrder )
//	{
//		currentTile = GetTile( currentTile->X + direction.second.x, currentTile->Y + direction.second.y );
//		if (IsUnpathableTerrain(currentTile) || currentTile->Cost < _allowedCost || IsOccupiedByNonEvasiveUnit(currentTile) || (_avoidUnits && IsOccupiedByUnitExclude(currentTile,_entity)))
//		{
//#if AI_DEBUG == 1
//			PROFILE(AIFreePathBool.Stop());
//#endif
//			return false;
//		}
//	}
//#if AI_DEBUG == 1
//	PROFILE(AIFreePathBool.Stop());
//#endif
//	return true;
}

//starts in a stuck environment and gets the first unblocked tile towards goal
Tile* Terrain::FreePathTile(Tile* _start, Tile* _goal, Entity _entity)
{
	int xx = _goal->X;
	int yy = _goal->Y;

	if (!IsUnpathableTerrain(_goal) && !IsOccupiedByNonEvasiveUnitExclude(_goal, _entity))
	{
		return _goal;
	}

	int x = 0;
	int y = 0;
	int d = 1;
	int L = 1;
	bool found = false;
	int temp;
	int costSqrd = 65000;
	Tile* closestTile;
	Tile* t;

	while (true)
	{
		for (int i = 0; i < L; ++i)
		{
			x += d;
			if (!OutOfBounds(static_cast<float>(yy + y), static_cast<float>(xx + x)))
			{
				t = &m_Field[yy + y][xx + x];
				if (!IsUnpathableTerrain(t) && !IsOccupiedByNonEvasiveUnitExclude(t, _entity))
				{
					temp = x*x + y*y;
					if ( !found || temp < costSqrd )
					{
						closestTile = t;
						costSqrd = temp;
						found = true;
					}
				}
			}
		}

		for (int i = 0; i < L; ++i)
		{
			y += d;
			if (!OutOfBounds(static_cast<float>(yy + y), static_cast<float>(xx + x)))
			{
				t = &m_Field[yy + y][xx + x];
				if (!IsUnpathableTerrain(t) && !IsOccupiedByNonEvasiveUnitExclude(t, _entity))
				{
					temp = x*x + y*y;
					if ( !found || temp < costSqrd )
					{
						closestTile = t;
						costSqrd = temp;
						found = true;
					}
				}
			}
		}

		char cost = static_cast<char>( sqrt(costSqrd) );
		if ( found && x >= cost && y >= cost )
		{
			return closestTile;
		}

		d = -d;
		L++;
	}
}

bool Terrain::IsOccupiedByNonEvasiveUnit(Tile* _t)
{
	Entity e = WhoIsOnTile(_t);

	if (e == ENTITY_INVALID)
		return false;

	if (HasComponent<DoodadComponent>(e))
	{
		if (GetDenseComponent<DoodadComponent>(e)->BlockPath)
			return true;
		else
			return false;
	}
	else if (HasComponent<AgentComponent>(e))
	{
		Agent* a = GetAgentPointer(e);
		if (a != nullptr)
			return !a->IsEvasiveUnit();
	}
	
	return false;
}

bool Terrain::IsOccupiedByNonEvasiveUnitExclude(Tile* _t, Entity _entity)
{
	Entity e = WhoIsOnTile(_t);

	if (e == ENTITY_INVALID)
		return false;

	if (e == _entity)
		return false;

	if (HasComponent<DoodadComponent>(e))
	{
		return true;
	}
	else
	{
		Agent* otherAgent = GetAgentPointer(e);

		if (otherAgent != nullptr)
		{
			Agent* self = GetAgentPointer(_entity);

			if(self != nullptr)
			{
				if (otherAgent->GetTeam() != self->GetTeam())
					return true;
			}

			return !otherAgent->IsEvasiveUnit();
		}
	}

	return false;
}

bool Terrain::IsOccupiedByUnitExclude(Tile* _t, Entity _entity)
{
	Entity e = WhoIsOnTile(_t);

	if (e != ENTITY_INVALID)
		if (e != _entity)
			return true;

	return false;
}

int Terrain::ManhattanDistance(Tile* _t1, Tile* _t2)
{
	return abs(_t1->X - _t2->X) + abs(_t1->Y - _t2->Y);
}

bool Terrain::IsOccupiedByUnit(Tile* _t)
{
	Entity e = WhoIsOnTile(_t);

	if (e != ENTITY_INVALID)
		return true;

	return false;
}

bool Terrain::IsUnpathableTerrain(Tile* t)
{
	return (t->Terrain >= 4);
}

int FIND_MOVABLE_TILE_ITERATIONS = 5;

//Too slow
//Free from non evasive units, may still be blocked by units that can evade
Tile* Terrain::GetMovableTile(float _x, float _y, Entity _entity, Tile* _fromTile)
{
 #if AI_DEBUG == 1
	PROFILE(AutoProfiler AIGetMovableTile("Terrain_GetMovableTile", Profiler::PROFILER_CATEGORY_STANDARD, true));
 #endif
	Tile* t = GetTile(_x, _y);
	Tile* start = t;
	rVector<Tile*> movableTiles;
	int shortest = FIND_MOVABLE_TILE_ITERATIONS+10; // any high value
	int shortestTileIndex = 0;
	if (IsUnpathableTerrain(t) || IsOccupiedByNonEvasiveUnitExclude(t, _entity))
	{
		if (IsUnpathableTerrain(t))
		{
			Tile* freeTile = FreePathTile(start, _fromTile, _entity);
			if (freeTile != nullptr)
				return freeTile;
		}

		int counter = 0;
		int x = 0;
		int y = 0;

		int radius = FIND_MOVABLE_TILE_ITERATIONS;

		for (int i = 0; i < radius * 2 + 1; i++)
		{
			for (int j = 0; j < radius * 2 + 1; j++)
			{
				if (j + i >= (radius) && j <= (radius + i) && i <= (radius + j) && j + i <= (radius * 3))
				{
					x = start->X - radius + j;
					y = start->Y - radius + i;

					if (x < 0 || y < 0 || x > m_GridWidth - 1 || y > m_GridHeight - 1)
						continue;

					t = GetTile(x, y);

					if (!IsUnpathableTerrain(t) && !IsOccupiedByNonEvasiveUnitExclude(t, _entity))
					{
						 #if AI_DEBUG == 1
						 	PROFILE(AIGetMovableTile.Stop());
						 #endif
						int dist = GetTileDistance(t, start);

						//keep looking as long as we find better distances, return if distance start going up again
						if (dist > 2)
						{
							shortest = dist;
							shortestTileIndex = static_cast<int>(movableTiles.size());
						}
						else
							return t;

						movableTiles.push_back(t);
					}
				}
			}
		}
	}
	else
		return t;

 #if AI_DEBUG == 1
 	PROFILE(AIGetMovableTile.Stop());
 #endif

	if (movableTiles.size() > 0)
		return movableTiles[shortestTileIndex];
	else
	{
		return start;
	}
}

//Looks for a totally free tile around the agent
Tile* Terrain::GetSurroundingFreeTile(float _x, float _y, Entity _entity)
{
	Tile* t = GetTile(_x, _y);
	Tile* start = t;

	int counter = 0;
	_x = (float)t->X;
	_y = (float)t->Y;
	int radiusL = -1;
	int radiusH = 2;

	for (int i = radiusL; i < radiusH; i++)
	{
		for (int j = radiusL; j < radiusH; j++)
		{
			if (i == 0 && j == 0)
				continue;

			t = Terrain::GetInstance()->GetTile(_x + i, _y + j);

			if (!IsUnpathableTerrain(t) && !IsOccupiedByUnitExclude(t, _entity))
			{
				return t;
			}
		}
	}

	return start;
}

//TODO: Slow as fuck, has to be optimized.
Terrain::EnemyZone Terrain::GetClosestEnemy( Tile* _tile, int _radius, int _team, unsigned int _updateID, bool checkWithFog )
{
// 	int transY = static_cast<int>(((float)AREA_GRID_SIZE / m_GridHeight)*_tile->Y);
// 	int transX = static_cast<int>(((float)AREA_GRID_SIZE / m_GridWidth)*_tile->X);
#if AI_DEBUG == 1
	PROFILE(AutoProfiler AIGetClosestEnemy("Terrain_GetClosestEnemy", Profiler::PROFILER_CATEGORY_STANDARD, true));
#endif

	if (_updateID == m_UpdateID)
	{
		EnemyZone zone;
		zone.Amount = 0;
		zone.Closest = ENTITY_INVALID;
		zone.UpdateID = m_UpdateID;
#if AI_DEBUG == 1
		PROFILE(AIGetClosestEnemy.Stop());
#endif

		return zone;
	}

	rVector<Agent*> agents;

	for (int i = 0; i < g_SSAI.m_Teams.size(); i++)
	{
		if ( i == _team || g_Alliances.IsAllied( i, _team ) )
		{
			continue;
		}
		for (int j = 0; j < g_SSAI.m_Teams[i].Squads.size(); j++)
		{
			agents.insert(agents.end(), g_SSAI.m_Teams[i].Squads[j]->GetAgents().begin(), g_SSAI.m_Teams[i].Squads[j]->GetAgents().end());
		}
	}

	int enemies = 0;
	int enclosingRadius = _radius;
	Entity closest = ENTITY_INVALID;
	for (int i = 0; i < agents.size(); i++)
	{
		if ( checkWithFog && !g_SSFogOfWar.CalcPlayerVisibilityForPosition( _team, agents[i]->GetWorldPos() ) )
		{
			continue;
		}

		int dist = GetTileDistance(_tile, agents[i]->GetTile());
		if (dist <= enclosingRadius)
		{
			closest = agents[i]->GetEntityID();
			enclosingRadius = dist;
		}
		if (dist <= _radius)
			enemies++;
	}

	EnemyZone zone;
	zone.Closest = closest;
	zone.Amount = enemies;
	zone.UpdateID = m_UpdateID;

	//m_UpdatedAreas[transY][transX] = false;
#if AI_DEBUG == 1
	PROFILE(AIGetClosestEnemy.Stop());
#endif
	return zone;
}

int Terrain::GetTileDistance(Tile* _a, Tile* _b)
{
	return static_cast<int>(sqrt(pow(_a->X - _b->X, 2)+ pow(_a->Y - _b->Y, 2)));
}

Entity Terrain::GetClosestResource(Tile* _tile, int _radius)
{
	Entity closest = ENTITY_INVALID;
	for (int i = 0; i < m_Resources.size(); i++)
	{
		int dist = GetTileDistance(_tile, m_Resources[i]->TilePointer);
		if (dist < _radius)
		{
			closest = m_Resources[i]->EntityID;
			_radius = dist;
		}
	}

	return closest;
}

rVector<Entity> Terrain::GetResourcesInRadius(Tile* _tile, int _radius)
{
	rVector<Entity> closest;
	for (int i = 0; i < m_Resources.size(); i++)
	{
		int dist = GetTileDistance(_tile, m_Resources[i]->TilePointer);
		if (dist < _radius)
		{
			closest.push_back(m_Resources[i]->EntityID);
		}
	}

	return closest;
}

rVector<Entity> Terrain::GetEnemiesInRadius(Tile* _tile, int _radius, int _team)
{
#if AI_DEBUG == 1
	PROFILE(AutoProfiler AIGetEnemiesInRadius("Terrain_GetEnemiesInRadius", Profiler::PROFILER_CATEGORY_STANDARD, true));
#endif

	std::priority_queue<EnemyDistance, rVector<EnemyDistance>, lessDist> EnemyQ;
	rVector<Agent*> agents;

	for (int i = 0; i < g_SSAI.m_Teams.size(); i++)
	{
		if (i == _team || g_Alliances.IsAllied(i, _team))
		{
			continue;
		}
		for (int j = 0; j < g_SSAI.m_Teams[i].Squads.size(); j++)
		{
			agents.insert(agents.end(), g_SSAI.m_Teams[i].Squads[j]->GetAgents().begin(), g_SSAI.m_Teams[i].Squads[j]->GetAgents().end());
		}
	}

	rVector<Entity> returnVector;
	int enclosingRadius = _radius;
	for (int i = 0; i < agents.size(); i++)
	{
		if (!g_SSFogOfWar.CalcPlayerVisibilityForPosition(_team, agents[i]->GetWorldPos()))
		{
			continue;
		}

		int dist = GetTileDistance(_tile, agents[i]->GetTile());
		if (dist < enclosingRadius)
		{
			EnemyDistance ed;
			ed.Distance = dist;
			ed.Enemy = agents[i]->GetEntityID();

			EnemyQ.push(ed);
		}
	}

	while (!EnemyQ.empty())
	{
		returnVector.push_back(EnemyQ.top().Enemy);
		EnemyQ.pop();
	}

#if AI_DEBUG == 1
	PROFILE(AIGetEnemiesInRadius.Stop());
#endif

	return returnVector;
}

void Terrain::LoadAIMap()
{
	const rString aiMapName = g_GameData.GetCurrentMapName() + ".maprob";
	const rString pathName = "../../../asset/maps/" + g_GameData.GetCurrentMapName( ) + "/" + aiMapName;

	std::fstream xInfoFile;
	xInfoFile.open( pathName.c_str() , std::fstream::in );

	if ( xInfoFile.is_open() )
	{
		// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
		printf( "Loading map AI\n" );
		for ( int yy = 0; yy < m_GridHeight; yy++ )
		{
			for ( int xx = 0; xx < m_GridWidth; xx++ )
			{
				xInfoFile >> m_Field[ yy ][ xx ].Terrain;

				if ( m_Field[ yy ][ xx ].Terrain == '_' )
					m_Field[ yy ][ xx ].Terrain = 0;

				m_Field[ yy ][ xx ].X = xx;
				m_Field[ yy ][ xx ].Y = yy;

				m_Entities[yy][xx] = ENTITY_INVALID;
			}
		}

		xInfoFile.close();
	}
	else
	{
		printf( "Failed to load map AI\n" );
	}

	InitValueOfTile();

}

void Terrain::InitValueOfTile()
{
#if	AI_GENERATE_MAP != 1
	LoadCostsFromFile();
#endif

#if	AI_GENERATE_MAP == 1
	for (int yy = 0; yy < m_GridHeight; yy++)
	{
		for (int xx = 0; xx < m_GridWidth; xx++)
		{
			if (IsUnpathableTerrain(&m_Field[yy][xx]))
			{
				m_Field[yy][xx].Cost = 0;
				continue;
			}

			int x = 0;
			int y = 0;
			int d = 1;
			int L = 1;
			bool found = false;
			float temp;
			int costSqrd = 65000;

			while (true)
			{
				for (int i = 0; i < L; ++i)
				{
					x += d;
					if (!OutOfBounds(static_cast<float>(yy + y), static_cast<float>(xx + x)))
					{
						if (IsUnpathableTerrain(&m_Field[yy + y][xx + x]))
						{
							temp = x*x + y*y;
							if ( !found || temp < costSqrd )
							{
								costSqrd = temp;
								found = true;
							}
						}
					}
				}

				for (int i = 0; i < L; ++i)
				{
					y += d;
					if (!OutOfBounds(static_cast<float>(yy + y), static_cast<float>(xx + x)))
					{
						if (IsUnpathableTerrain(&m_Field[yy + y][xx + x]))
						{
							temp = x*x + y*y;
							if ( !found || temp < costSqrd )
							{
								costSqrd = temp;
								found = true;
							}
						}
					}
				}

				char cost = static_cast<char>( sqrt(costSqrd) );
				if ( found && x >= cost && y >= cost )
				{
					m_Field[yy][xx].Cost = cost;
					break;
				}

				d = -d;
				L++;
			}
		}
	}

	WriteCostsToFile();
#endif
}

void Terrain::LoadCostsFromFile()
{
	rString pathName = "../../../asset/maps/" + g_GameData.GetCurrentMapName() + "/CostMap.txt";

	std::fstream xInfoFile;
	xInfoFile.open(pathName.c_str(), std::fstream::in );

	for (int i = 0; i < m_GridHeight; i++)
	{
		for (int j = 0; j < m_GridWidth; j++)
		{
			int temp = 0;
			xInfoFile >> temp;
			m_Field[i][j].Cost = static_cast<unsigned char>(temp);
		}
	}

	xInfoFile.close();
}

#if	AI_GENERATE_MAP == 1
void Terrain::WriteCostsToFile()
{
	rString pathName = "../../../asset/maps/" + g_GameData.GetCurrentMapName() + "/CostMap.txt";

	std::fstream xInfoFile;
// 	xInfoFile.open(pathName.c_str(), std::fstream::in);
// 
// 	//write
// 	xInfoFile.open(pathName.c_str(), std::fstream::in | std::fstream::out);
// 
// 	if (!xInfoFile || !xInfoFile.good())
// 	{
// 		xInfoFile.close();
		xInfoFile.open(pathName.c_str(), std::fstream::in | std::fstream::out | std::fstream::trunc);
//}

	//print map to file
	for (int i = 0; i < m_GridHeight; i++)
	{
		for (int j = 0; j < m_GridWidth; j++)
		{
			xInfoFile << static_cast<int>(m_Field[i][j].Cost) << "	";
		}
		xInfoFile << "\n";
	}

	xInfoFile.close();
}
#endif

//generates the ai grid from tga
#if GENERATE_AI_MAP == 1
void Terrain::WriteAIMapToFile()
{
	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Create map for AI begin:\n");
	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Get heights\n");
	for (int yy = 0; yy < m_GridHeight; yy++)
	{
		for (int xx = 0; xx < m_GridWidth; xx++)
		{
			// 			if (yy == 0 || xx == 0 || xx == m_GridWidth - 1 || yy == m_GridHeight - 1)
			// 				m_Field[yy][xx].Terrain = '#';
			// 			else
			m_Field[yy][xx].Terrain = 0;
			m_Heights[yy][xx] = gfx::g_GFXTerrain.GetHeightAtWorldCoord(xx, yy);

			m_Field[yy][xx].X = xx;
			m_Field[yy][xx].Y = yy;
		}
	}

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Set walkable on stepsize\n");
	for (int yy = 0; yy < m_GridHeight; yy++)
	{
		for (int xx = 0; xx < m_GridWidth; xx++)
		{
			float thisHeight = m_Heights[yy][xx];
			float unWalkValue = 0;
			for (int i = -1; i < 2; i++)
			{
				for (int j = -1; j < 2; j++)
				{
					int x = xx + j;
					int y = yy + i;

					if (x < 0 || y < 0 || x > m_GridWidth - 1 || y > m_GridHeight - 1)
					{
						continue;
					}

					const float stepSize = 1.0f;
					if (std::abs(m_Heights[y][x] - thisHeight) > stepSize)
					{
						unWalkValue++;
					}
				}
			}

			m_Field[yy][xx].Terrain = unWalkValue;
		}
	}

	rVector<Tile*> toEmboss;

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Emboss areas\n");
	int passes = 15;
	for (int j = 0; j < passes; j++)
	{
		for (int yy = 0; yy < m_GridHeight; yy++)
		{
			for (int xx = 0; xx < m_GridWidth; xx++)
			{
				if (!IsUnpathableTerrain(GetTile(xx, yy)))
				{
					int neighbours = AdjacentWallsInt(xx, yy);
					if (neighbours > 3)
						toEmboss.push_back(&m_Field[yy][xx]);
				}
			}
		}
		for (int i = 0; i < toEmboss.size(); i++)
		{
			int x = 0;
			int y = 0;
			x = toEmboss[i]->X;
			y = toEmboss[i]->Y;
			m_Field[y][x].Terrain = '#';
		}
	}

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Smoothing\n");
	//smoothing
	passes = 5;
	for (int j = 0; j < passes; j++)
	{
		for (int yy = 0; yy < m_GridHeight; yy++)
		{
			for (int xx = 0; xx < m_GridWidth; xx++)
			{
				if (IsUnpathableTerrain(GetTile(xx, yy)))
				{
					int neighbours = AdjacentWallsInt(xx, yy);

					if (neighbours < 4)
						m_Field[yy][xx].Terrain = 0;
				}
			}
		}
	}

	rVector<Pathfinder::DirtyTile> badTiles;
	Tile* startTile = GetTile(m_GridWidth*0.5f, m_GridHeight*0.5f);

	int percentage = 0;

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Test path finding to all tiles\n");
#pragma omp parallel
	{
		Pathfinder pathFinderTest;
		int tid = omp_get_thread_num();
	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
		printf("Running on thread %d\n", tid);
#pragma omp parallel for
		for (int i = 0; i < m_GridHeight; i++)
		{
			if (tid == 0)
			{
				if (i % (m_GridHeight / 100) == 0)
				{
					percentage++;
					std::cout << percentage << '%';
					std::cout.flush();
					std::cout << '\r';
				}
			}
			for (int j = 0; j < m_GridWidth; j++)
			{
				pathFinderTest.SetStart(startTile);
				pathFinderTest.SetGoal(GetTile(j, i));
				pathFinderTest.AStar(startTile);
			}
		}

#pragma omp critical
		badTiles.insert(badTiles.end(), pathFinderTest.GetBadTiles().begin(), pathFinderTest.GetBadTiles().end());
	}

	int x = 0;
	int y = 0;

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Adding bad tiles\n");
	//adding bad tiles to un-pathable
	for (int i = 0; i < badTiles.size(); i++)
	{
		x = badTiles[i].x;
		y = badTiles[i].y;
		//xInfoFile << x << " " << y << "\n";
		m_Field[y][x].Terrain = '#';
	}

	rString mapName = "WinterWar";
	rString pathName = "../../../asset/maps/" + rString(mapName) + "/XTerrainInfo.txt";

	std::fstream xInfoFile;
	xInfoFile.open(pathName.c_str(), std::fstream::in);

	//write
	xInfoFile.open(pathName.c_str(), std::fstream::in | std::fstream::out);

	if (!xInfoFile || !xInfoFile.good())
	{
		xInfoFile.close();
		xInfoFile.open(pathName.c_str(), std::fstream::in | std::fstream::out | std::fstream::trunc);
	}

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Printing map\n");
	//print map to file
	for (int i = 0; i < m_GridHeight; i++)
	{
		for (int j = 0; j < m_GridWidth; j++)
		{
			if (m_Field[i][j].Terrain >= 4)
			{
				xInfoFile << '#';
			}
			else
				xInfoFile << "_";
		}
		xInfoFile << "\n";
	}

	// TODOVK: Use logger, make ifdef macro if this needs to be used when logger is not accessible
	printf("Engine export complete\n");

	xInfoFile.close();
}
#endif
