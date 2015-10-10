CreateListCategory( "ControlPoints", "<insert description>" )

CreateListFunction( "SetAllControlPointsNeutral", 	"Makes all control points neutral. I.e. they will no longer belong to any player." )
function SetAllControlPointsNeutral()
	GE_SetAllNeutral()
end

CreateListFunction( "SetAllControlPointsTo", 	"Make all control points belong to the specified player." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID from unit selection.", true )
function SetAllControlPointsTo( playerID )
	playerID 	= playerID 	or GE_GetSelectedPlayer()
	GE_SetAllBelongTo( playerID )
end

CreateListFunction( "SetAllControlPointsNeutralThatBelongTo", 	"Make all of a specified players control points neutral. I.e. they will no longer belong to any player." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID from unit selection.", true )
function SetAllControlPointsNeutralThatBelongTo( playerID )
	playerID 	= playerID 	or GE_GetSelectedPlayer()
	GE_SetAllNeutralThatBelongTo( playerID )
end

CreateListFunction( "SetControlPointNeutral", 	"Make the specified control point neutral. I.e. it will no longer belong to any player." )
CreateListArgument( "entityID",	"int",	"EntityID of control point. Default gets entity ID from unit selection.", true )
function SetControlPointNeutral( entityID )
	entityID 	= entityID	or GE_GetSelectedEntity()
	GE_SetNeutral( entityID )
end

CreateListFunction( "SetControlPointTo", 	"Make the specified control point belong to the specified player." )
CreateListArgument( "playerID",	"int",	"ID of player the control point now should belong to. Default gets player ID of the local player.", true )
CreateListArgument( "entityID",	"int",	"EntityID of control point. Default gets entity ID from unit selection.", true )
function SetControlPointTo( playerID, entityID )
	entityID 	= entityID	or GE_GetSelectedEntity()
	playerID	= playerID	or g_LocalPlayerID
	GE_SetSetBelongTo( playerID, entityID )
end