CreateListCategory( "DebugRendering", "<insert description>" )

CreateListFunction( "ShowTerrainPicking", 				"Renders a box where picking ray intersects the terrain." )
function ShowTerrainPicking()
	GE_SetShowTerrainPicking( true )
end

CreateListFunction( "HideTerrainPicking", 				"Hides rendering of a box where picking ray intersects the terrain." )
function HideTerrainPicking()
	GE_SetShowTerrainPicking( false )
end

CreateListFunction( "ShowSquadPath", 					"Renders lines that shows the selected squad(s) path." )
function ShowSquadPath()
	GE_SetShowSquadPath( true )
end

CreateListFunction( "HideSquadPath", 					"Hides rendering of lines that shows the selected squad(s) path." )
function HideSquadPath()
	GE_SetShowSquadPath( false )
end

CreateListFunction( "ShowUnitPath", 					"Renders lines that shows the selected unit(s) path." )
function ShowUnitPath()
	GE_SetShowUnitPath( true )
end

CreateListFunction( "HideUnitPath", 					"Hides rendering of lines that shows the selected unit(s) path." )
function HideUnitPath()
	GE_SetShowUnitPath( false )
end