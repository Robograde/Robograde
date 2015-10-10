CreateListCategory( "Camera", "<insert description>" )

CreateListFunction( "SetCameraPosition", 	"Teleports the camera to the specified position." )
CreateListArgument( "posX",		"number",	"x-position in world coordinates." )
CreateListArgument( "posY",		"number",	"y-position in world coordinates." )
CreateListArgument( "posZ",		"number",	"z-position in world coordinates." )
function SetCameraPosition( posX, posY, posZ )
	GE_SetCameraPosition( posX, posY, posZ )
end

CreateListFunction( "LookAt", 				"Switches to RTS mode and makes the camera look at the specified point." )
CreateListArgument( "posX",		"number",	"x-position of point to look at in world coordinates." )
CreateListArgument( "posY",		"number",	"y-position of point to look at in world coordinates. Default gets value from posX.", true )
CreateListArgument( "posZ",		"number",	"z-position of point to look at in world coordinates. Default gets value from posY and then sets posY to 0.", true )
function LookAt( posX, posY, posZ )
	if posY == nil then
		posY = 0
		posZ = posX
	else
		if posZ == nil then
			posZ = posY
			posY = 0
		end
	end
	
	GE_CameraLookAt( posX, posY, posZ )
end

CreateListFunction( "SetCameraMoveSpeed", 		"Changes how fast the active camera moves." )
CreateListArgument( "moveSpeed",	"number",	"Move speed in meters per second." )
function SetCameraMoveSpeed( moveSpeed )
	GE_SetCameraMoveSpeed( moveSpeed )
end

CreateListFunction( "SetCameraAlternativeMoveSpeed", 	"Changes how fast the first person camera moves while holding down the \"run\" key on the keyboard." )
CreateListArgument( "moveSpeed",	"number",			"Move speed in meters per second." )
function SetCameraAlternativeMoveSpeed( moveSpeed )
	GE_SetCameraAlternativeMoveSpeed( moveSpeed )
end

CreateListFunction( "SetCameraFOV", 	"Changes Field of View of the active cameras lens." )
CreateListArgument( "fov",	"number",	"Field of view given in radians." )
function SetCameraFOV( fov )
	GE_SetCameraFOV( fov )
end

CreateListFunction( "CameraBoundsActive", 	"Set bounds that keep the RTS camera inside the map on or off." )
CreateListArgument( "active",	"bool",		"true turns bounds on, false turns them off." )
function CameraBoundsActive( active )
	GE_SetCameraBoundsActive( active )
end

CreateListFunction( "CameraBoundsOn", 		"Set bounds that keep the RTS camera inside the map on." )
function CameraBoundsOn()
	CameraBoundsActive( true )
end

CreateListFunction( "CameraBoundsOff", 		"Set bounds that keep the RTS camera inside the map off." )
function CameraBoundsOff()
	CameraBoundsActive( false )
end

CreateListFunction( "CameraRTS", 			"Switch camera to RTS mode." )
function CameraRTS()
	GE_ActivateCameraRTS()
end

CreateListFunction( "CameraFirstPerson", 	"Switch camera to First person mode." )
function CameraFirstPerson()
	GE_ActivateCameraFP()
end