CreateListCategory( "Entities", "<insert description>" )

CreateListFunction( "CreateUnit", 						"Create a unit (squad member)" )
CreateListArgument( "pos", 			"array<number>",	"Spawn position for unit. {x,y}, or {xy} allowed." )
CreateListArgument( "squadID",		"int",		 		"ID of squad that the unit will be added to. Default gets squad ID from your unit selection.", true )
CreateListArgument( "playerID",		"int",		 		"ID of player that the unit will be added to. Default gets player ID from your unit selection.", true )
function CreateUnit( pos, squadID, playerID )
	playerID 	= playerID 	or GE_GetSelectedPlayer()
	squadID		= squadID	or GE_GetSelectedSquad()
	if #pos == 1 then
		pos[2] = pos[1]
	end
	GE_CreateSquadUnit( pos[1], pos[2], squadID, playerID )
end

CreateListFunction( "CreateTerrain", "Create the terrain." )
function CreateTerrain()
	GE_CreateTerrain()
end

CreateListFunction( "CreateResource", 					"Create a resource that can be consumed by squad units." )
CreateListArgument( "pos", 			"array<number>",	"Spawn position for resource. {x,y}, or {xy} allowed." )
CreateListArgument( "scale", 		"array<number>",	"Scale of the spawned resource. Only {x,y,z} allowed." )
CreateListArgument( "orientation",	"array<number>",	"Quaternion orientation of the spawned resource. Only {w,x,y,z} allowed. Use {1,0,0,0} for no orientation." )
CreateListArgument( "modelPath", 	"string",			"Path to file containing the visual model of the resource." )
function CreateResource( pos, scale, orientation, modelPath )
	if #pos == 1 then
		pos[2] = pos[1]
	end
	GE_CreateResource( pos[1], pos[2], scale[1], scale[2], scale[3], orientation[1], orientation[2], orientation[3], orientation[4], modelPath )
end

CreateListFunction( "CreateProp", 						"Create an object that can block the path of the squad units" )
CreateListArgument( "pos", 			"array<number>",	"Spawn position for prop. {x,y}, or {xy} allowed." )
CreateListArgument( "scale", 		"array<number>",	"Scale of the spawned prop. Only {x,y,z} allowed." )
CreateListArgument( "orientation",	"array<number>",	"Quaternion orientation of the spawned prop. Only {w,x,y,z} allowed. Use {1,0,0,0} for no orientation." )
CreateListArgument( "blockPath", 	"bool",				"If the prop should block the AI's pathfinding maybe???" )
CreateListArgument( "modelPath", 	"string",			"Path to file containing the visual model of the prop." )
CreateListArgument( "radius", 		"int",				"How large of a radius should be blocked for the pathfinding maybe???." )
function CreateProp( pos, scale, orientation, blockPath, modelPath, radius )
	if #pos == 1 then
		pos[2] = pos[1]
	end
	GE_CreateProp( pos[1], pos[2], scale[1], scale[2], scale[3], orientation[1], orientation[2], orientation[3], orientation[4], blockPath, modelPath, radius )
end

CreateListFunction( "CreateControlPoint", 				"Create a control point that can be taken over by squad units." )
CreateListArgument( "pos", 			"array<number>",	"Spawn position for control point. {x,y}, or {xy} allowed." )
CreateListArgument( "scale", 		"array<number>",	"Scale of the spawned control point. Only {x,y,z} allowed." )
CreateListArgument( "orientation",	"array<number>",	"Quaternion orientation of the spawned control point. Only {w,x,y,z} allowed. Use {1,0,0,0} for no orientation." )
CreateListArgument( "playerID", 	"int",				"ID of player that the control point will belong to when spawned. Use -1 for spawning it neutral." )
CreateListArgument( "modelPath", 	"string",			"Path to file containing the visual model of the control point." )
function CreateControlPoint( pos, scale, orientation, playerID, modelPath )
	if #pos == 1 then
		pos[2] = pos[1]
	end
	GE_CreateControlPoint( pos[1], pos[2], scale[1], scale[2], scale[3], orientation[1], orientation[2], orientation[3], orientation[4], playerID, modelPath )
end

CreateListFunction( "IncreaseSquadSize",			"Create new units and adds them to the squad." )
CreateListArgument( "nrOfUnitsToCreate",	"int",	"Default is 1.", true  )
CreateListArgument( "squadID", 				"int",	"ID of squad that the units will be added to. Default gets squad ID from your unit selection.", true  )
CreateListArgument( "playerID", 			"int",	"ID of player that the units will be added to. Default gets player ID from your unit selection.", true  )
function IncreaseSquadSize( nrOfUnitsToCreate, squadID, playerID )
	playerID 			= playerID 			or GE_GetSelectedPlayer()
	squadID				= squadID			or GE_GetSelectedSquad()
	nrOfUnitsToCreate	= nrOfUnitsToCreate	or 1
	GE_IncreaseSquadSize( nrOfUnitsToCreate, squadID, playerID )
end

CreateListFunction( "DecreaseSquadSize",			"Remove units from the squad." )
CreateListArgument( "nrOfUnitsToRemove", 	"int",	"Default is 1.", true  )
CreateListArgument( "squadID", 				"int",	"ID of squad that the units will be removed from. Default gets squad ID from your unit selection.", true  )
CreateListArgument( "playerID", 			"int",	"ID of player that the units will be removed from. Default gets player ID from your unit selection.", true  )
function DecreaseSquadSize( nrOfUnitsToRemove, squadID, playerID )
	playerID 			= playerID 			or GE_GetSelectedPlayer()
	squadID				= squadID			or GE_GetSelectedSquad()
	nrOfUnitsToRemove	= nrOfUnitsToRemove	or 1
	GE_DecreaseSquadSize( nrOfUnitsToRemove, squadID, playerID )
end

CreateListFunction( "ChangeSquadSize", 			"Change size of the squad by creating or killing its units." )
CreateListArgument( "newUnitAmount", 	"int",	"Default is doing nothing.", true  )
CreateListArgument( "squadID", 			"int",	"ID of squad that will have its size changed. Default gets squad ID from your unit selection.", true  )
CreateListArgument( "playerID", 		"int",	"ID of player owning the squad. Default gets player ID from your unit selection.", true  )
function ChangeSquadSize( newUnitAmount, squadID, playerID )
	playerID 		= playerID 	or GE_GetSelectedPlayer()
	squadID			= squadID	or GE_GetSelectedSquad()
	if newUnitAmount ~= nil then
		GE_ChangeSquadSize( newUnitAmount, squadID, playerID )
	end
end

CreateListFunction( "SetSS", 			"Set size of the squad by creating or killing its units." )
CreateListArgument( "newUnitAmount", 	"int",	"Default is doing nothing.", true  )
CreateListArgument( "squadID", 			"int",	"ID of squad that will have its size changed. Default gets squad ID from your unit selection.", true  )
CreateListArgument( "playerID", 		"int",	"ID of player owning the squad. Default gets player ID from your unit selection.", true  )
function SetSS( newUnitAmount, squadID, playerID )
	playerID 		= playerID 	or GE_GetSelectedPlayer()
	squadID			= squadID	or GE_GetSelectedSquad()
	if newUnitAmount ~= nil then
		GE_ChangeSquadSize( newUnitAmount, squadID, playerID )
	end
end

CreateListFunction( "CreateSquad", 			"WARNING!!! NOT IMPLEMENTED YET!!!" )
CreateListArgument( "playerID", 	"int",	"ID of player the squad will be added to. Default gets player ID from your unit selection.", true  )
function CreateSquad( playerID )
	playerID 	= playerID 	or GE_GetSelectedPlayer()
	print("This function is not yet implemented")
end

CreateListFunction( "KillPlayer", 			"Kill all of the players units." )
CreateListArgument( "playerID", 	"int",	"ID of player to be killed. Default gets player ID from your unit selection.", true  )
function KillPlayer( playerID )
	playerID 	= playerID 	or GE_GetSelectedPlayer()
	GE_KillPlayer( playerID )
end

CreateListFunction( "KillSquad", 			"Kills all units in the squad" )
CreateListArgument( "squadID", 		"int",	"ID of squad to be killed. Default gets squad ID from your unit selection.", true  )
CreateListArgument( "playerID", 	"int",	"ID of player owning the squad. Default gets player ID from your unit selection.", true  )
function KillSquad( squadID, playerID )
	playerID 		= playerID 	or GE_GetSelectedPlayer()
	squadID			= squadID	or GE_GetSelectedSquad()
	ChangeSquadSize( 0, squadID, playerID );
end