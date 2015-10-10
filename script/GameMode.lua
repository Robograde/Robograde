CreateListCategory( "GameMode", "<insert description>" )


CreateListFunction( "SwitchGameMode", 			"Switch to the specified Gamemode" )
CreateListArgument( "gameModeName",	"string",	"Name of the gamemode" )
function SwitchGameMode( gameModeName )
	GE_SwitchGameMode( gameModeName )
end