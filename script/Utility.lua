CreateListCategory( "Utility", "<insert description>" )

CreateListFunction( "Crash", 		"Crashes the program by calling abort()." )
function Crash()
	GE_CrashToDesktop()
end

CreateListFunction( "PlayAs", 			"Switches which player you play as. Not fully functional." )
CreateListArgument( "playerID",	"int",	"Player ID of the one you want to play as. Default gets value from selected unit.", true )
function PlayAs( playerID )
	playerID = playerID or GE_GetSelectedPlayer()
	
	RemoveVision( g_LocalPlayerID )
	
	GE_PlayAs( playerID )
	
	AddVision( playerID )
end

CreateListFunction( "PrintStarted", 	"Print list of subsystems that have been started." )
function PrintStarted()
	GE_PrintStartedSubsystems()
end

CreateListFunction( "PrintUnstarted", 	"Print list of subsystems that have not yet been started." )
function PrintUnstarted()
	GE_PrintUnstartedSubsystems()
end

CreateListFunction( "PrintGameModesSubsystems", 	"Print list of subsystems that will be run in the specified game mode." )
CreateListArgument( "gameModeName",	"string",		"Name of the game mode." )
function PrintGameModesSubsystems( gameModeName )
	GE_PrintGameModesSubsystems( gameModeName )
end