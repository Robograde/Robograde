CreateListCategory( "Research", "<insert description>" )

g_LocalPlayerID = 0

CreateListFunction( "Research", 		"Start a research for the specified player." )
CreateListArgument( "research",	"int",	"ID of research." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function Research( research, playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_SwitchActiveResearch( research, playerID )
end

CreateListFunction( "QueueResearch", 	"Queue a research for the specified player." )
CreateListArgument( "research",	"int",	"ID of research." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function QueueResearch( research, playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_QueueResearch( research, playerID )
end

CreateListFunction( "SetResearchSpeed", "Change how often research should tick." )
CreateListArgument( "researchSpeed",	"number",	"Number of ticks per second." )
function SetResearchSpeed( researchSpeed )
	GE_SetResearchSpeed( researchSpeed )
end

CreateListFunction( "GiveResearch", 	"Set a specified research for a player to finished." )
CreateListArgument( "research",	"int",	"ID of research." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function GiveResearch( research, playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_GiveResearch( research, playerID )
end

CreateListFunction( "ResetResearch", 	"Set progress for a specified research for a player to 0." )
CreateListArgument( "research",	"int",	"ID of research." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function ResetResearch( research, playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_ResetResearch( research, playerID )
end

CreateListFunction( "GiveAllResearch", 	"Set all of a players research to finished." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function GiveAllResearch( playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_GiveAllResearch( playerID )
end

CreateListFunction( "QueueAllResearch", "Queue up all research for a player." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function QueueAllResearch( playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_QueueAllResearch( playerID )
end

CreateListFunction( "ResetAllResearch", "Set progress for all of a players research to 0." )
CreateListArgument( "playerID",	"int",	"ID of player. Default gets player ID of the local player.", true )
function ResetAllResearch( playerID )
	playerID = playerID or g_LocalPlayerID;
	GE_ResetAllResearch( playerID )
end