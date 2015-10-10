/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once

enum ENTITY_TYPE
{
	PICKING_TYPE_TERRAIN,
	PICKING_TYPE_CONTROLLABLE,
	PICKING_TYPE_FRIEND,
	PICKING_TYPE_ENEMY,
	PICKING_TYPE_RESOURCE,
	PICKING_TYPE_UNIT_PROP,
	PICKING_TYPE_PROP,
	PICKING_TYPE_CONTROL_POINT,
	PICKING_TYPE_GHOST_PLACEMENT_OBJECT,
	PICKING_TYPE_SFX_EMITTER,
	PICKING_TYPE_PARTICLE_EMITTER,
	PICKING_TYPE_BRUSH,
	PICKING_TYPE_SIZE		// KEEP LAST
};