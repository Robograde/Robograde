/**************************************************
2015 Viktor Kelkkanen & Ola Enberg
***************************************************/

#pragma once
#include "stdafx.h"
#include "Agent.h"
#include "../datadriven/ComponentTypes.h"
#include "../datadriven/ComponentHelper.h"
#include "../component/AgentComponent.h"
#include "../component/ResourceComponent.h"
#include "../component/PlacementComponent.h"

class Terrain
{
public:

	struct EnemyZone
	{
		Entity			Closest = ENTITY_INVALID;
		int				Amount = 0;
		unsigned int	UpdateID = 0;
	};

	struct EnemyDistance
	{
		Entity			Enemy = ENTITY_INVALID;
		int			Distance = 0;
	};

	struct lessDist
	{
		template<class EnemyDistance>
		bool operator()(const EnemyDistance& left, const EnemyDistance& right) const
		{
			return left.Distance > right.Distance;
		}
	};

	Terrain();
	~Terrain();

	void					InitField();

	void LoadAIMap();

#if GENERATE_AI_MAP == 1
	void WriteAIMapToFile();
#endif

	void					UpdateSensor(Entity _agent, Tile* _tile, Tile* _prevTile, int _size, bool _destroy);

	void					SetTerrain(int _x, int _y, char _terrain);
	char					GetTerrain(int _x, int _y);
	Tile*					GetTile(float _x, float _y);
	Tile*					GetTile(int _x, int _y);
	Tile*					GetMovableTile(float _x, float _y, Entity _entity, Tile* _fromTile);
	static Terrain*			GetInstance();
	Tile*					GetSurroundingFreeTile(float _x, float _y, Entity _a);

	
	bool					AdjacentWallsBool(int _x, int _y, bool _avoidEntities);
	Tile*					FreePathTile(Tile* _start, Tile* _goal, Entity _entity);
	bool					FreePathBool(Tile* _start, Tile* _goal, bool _avoidUnits, Entity _entity, char _allowedCost, const glm::vec2& startPosition);
	bool					OutOfBounds(float _x, float _y);


	Entity					WhoIsOnTile(Tile* _tile);

	bool					IsOccupiedByNonEvasiveUnit(Tile* t);
	bool					IsUnpathableTerrain(Tile* t);
	bool					IsOccupiedByUnit(Tile* t);
	bool					IsOccupiedByNonEvasiveUnitExclude(Tile* _t, Entity _a);
	bool					IsOccupiedByUnitExclude(Tile* _t, Entity _a);

	int						ManhattanDistance(Tile* _t1, Tile* t2);

	rVector<Entity>			GetEnemiesInRadius(Tile* _tile, int _radius, int _team);
	Terrain::EnemyZone		GetClosestEnemy(Tile* _tile, int _radius, int _team, unsigned int _updateID, bool checkWithFog = true );
	Entity					GetClosestResource(Tile* _tile, int _radius);
	int						GetTileDistance(Tile* _a, Tile* _b);
	unsigned int			GetUpdateID() const { return m_UpdateID; }

	int						AdjacentWallsInt(int _x, int _y);
	bool					AdjacentCostBool(int _x, int _y, int cost);

	rVector<ResourceComponent*>& GetResourceList() {return m_Resources;}
	rVector<Entity> GetResourcesInRadius(Tile* _tile, int _radius);
	
	int							m_GridWidth;
	int							m_GridHeight;
private:

	unsigned int				m_UpdateID;

	rVector<rVector<Tile>>		m_Field;
	rVector<rVector<Entity>>	m_Entities;
	rVector<ResourceComponent*> m_Resources;

	void InitValueOfTile();
	void LoadCostsFromFile();

#if AI_GENERATE_MAP == 1
	void WriteCostsToFile();
	
	rVector<rVector<float>>		m_Heights;
#endif
};