/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once

#define PI 3.1415926f
/*	
	To use balance test, set AI_BALANCE_TEST 1, and change what weapons to start with in SSAI.h:
		80 int m_CurrentTestUpg1 = 4;
		81 int m_CurrentTestUpg2 = 3;
*/

#define AI_BALANCE_TEST 0
#define AI_DEBUG 0
#define AI_RENDER_DEBUG 0 //AI_DEBUG needs to be 1
#define AI_RENDER_DEBUG_TERRAIN 0
#define AI_GENERATE_MAP 0
#define AI_RENDER_GOALS 0 
#define AI_REMOVE_MESHES 0

#define AI_DRAW_NEURAL_NETS 0
static const bool PRINT_AI_PLAYER = 0;

static bool AI_TRAIN_NEURAL_NETS = 1;
struct Tile
{
	int		X;
	int		Y;
	char	Terrain;
	unsigned char Cost;
#if AI_DEBUG == 1
	char	DebugInfo;
#endif

	Tile() :X(0), Y(0), Terrain(0), Cost(0)
#if AI_DEBUG == 1
		, DebugInfo(0)
#endif	
	{}
};

struct Goal
{
	Tile*	Tile;
	int		Tag		= -1;

	Goal( struct Tile* tile )			{ this->Tile = tile; }
	Goal( struct Tile* tile, int tag )	{ this->Tile = tile; this->Tag = tag; }
};