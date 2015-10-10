CreateListCategory( "Fog", "<insert description>" )

CreateListFunction( "Fog", 						"Toggle fog of war on or off." )
CreateListArgument( "activation",	"bool",		"true turns fog on, false turns it off." )
function Fog( activation )
	GE_SetFogActive( activation )
end

CreateListFunction( "FogOff", "Toggle fog of war off." )
function FogOff()
	Fog( false )
end

CreateListFunction( "FogOn", "Toggle fog of war on." )
function FogOn()
	Fog( true )
end

CreateListFunction( "ClearVision", "No longer show any players vision." )
function ClearVision()
	GE_ClearVisionGivingPlayers()
end

CreateListFunction( "AddVision", 					"Add specified players vision to yours." )
CreateListArgument( "playerID",		"variadic",		"List ID:s of all players you want vision from. Seperate each playerID by a comma(,)." )
function AddVision(...)
	local arg={...}
	for i=1, #arg do
		GE_AddVisionGivingPlayer( arg[i] )
	end
end

CreateListFunction( "RemoveVision", 				"Remove specified players vision from yours." )
CreateListArgument( "playerID",		"variadic",		"List ID:s of all players you no longer want vision from. Seperate each playerID by a comma(,)." )
function RemoveVision(...)
	local arg={...}
	for i=1, #arg do
		GE_RemoveVisionGivingPlayer( arg[i] )
	end
end

CreateListFunction( "UseVision", 					"See how vision that the AI uses looks. Leave paramaters empty to go back to normal vision" )
CreateListArgument( "playerID",		"int",			"ID of player you want to see vision of. Default sets vision back to one that is affected by FogOff, AddVision, e.t.c.", true )
function UseVision( playerID )
	playerID = playerID or 4
	GE_SetUsedPlayerVision( playerID )
end