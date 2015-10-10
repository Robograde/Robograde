/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include "stdafx.h"

class Node
{
	public:
	Tile* TilePointer;
	int Fcost;
	Tile* Parent;
	bool Closed;
	Node() :TilePointer(nullptr), Fcost(0), Parent(nullptr){}
	Node(Tile* _tile, const int _fCost,Tile* _parent)
	{
		TilePointer = _tile;
		Fcost = _fCost;
		Parent = _parent;
		Closed = false;
	}

	bool operator<(const Node& rhs) const { return Fcost < rhs.Fcost; }

};