print = function( ... )
	local printVal = ""
	for i,v in ipairs({...}) do
		printVal = printVal .. tostring(v) .. "    "
	end
	printVal = printVal .. "\n"
	Log( printVal )
end

print( "Hello World! ~ Says Lua." )

SRC_DIR = "../../../script/"

dofile( SRC_DIR .. "List.lua" )
dofile( SRC_DIR .. "DebugRendering.lua" )
dofile( SRC_DIR .. "GameMode.lua" )
dofile( SRC_DIR .. "Research.lua" )
dofile( SRC_DIR .. "FogOfWar.lua" )
dofile( SRC_DIR .. "Entities.lua" )
dofile( SRC_DIR .. "Camera.lua" )
dofile( SRC_DIR .. "ControlPoints.lua" )
dofile( SRC_DIR .. "Utility.lua" )

print( "All includes loaded succssfully!" )