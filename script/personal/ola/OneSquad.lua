print( "Running OneSquad.lua"  )

print( "  Killing other players." )
for i = 1, 3 do
	KillPlayer( i )
end

--print( "  Killing second squad from player 0." )
--KillSquad( 1, 0 )

print( "  Changing size of remaining squad" )
ChangeSquadSize( 18, 0, 0 )

print( "Finished OneSquad.lua" )