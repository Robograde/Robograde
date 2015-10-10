g_List = {}

function CreateListCategory( name, description )
	table.insert( g_List, { name, description, {} } )
end

function CreateListFunction( name, description )
	table.insert( GetLatestListCategory()[3], { name, description, {} } )
end

function CreateListArgument( name, argumentType, description, optional )
	optional = optional or false
	table.insert( GetLatestListFunction()[3], { name, argumentType, description, optional } )
end

function GetLatestListCategory()
	local cat = #g_List
	return g_List[cat]
end

function GetLatestListFunction()
	local latestCategory = GetLatestListCategory()
	local func = #latestCategory[3]
	return latestCategory[3][func]
end

function GetLatestListArgument()
	local latestFunction = GetLatestListFunction()
	local arg = #latestFunction[3]
	return latestFunction[3][arg]
end

CreateListCategory( "List", "<insert description>" )

CreateListFunction( "List", "List all functions. Including descriptions." )
function List()
	for i = 1, #g_List do
		print( "[C=PURPLE]" .. g_List[i][1] .. "[C=GREY] - " .. g_List[i][2] )
		
		for j = 1, #g_List[i][3] do
			print( " [C=GREEN]" .. g_List[i][3][j][1] .. "[C=WHITE] - " .. g_List[i][3][j][2] )
		end
	end
end

CreateListFunction( "ListFull", "List all functions. Including detailed arguments and descriptions." )
function ListFull()
	for i = 1, #g_List do
		print( "[C=PURPLE]" .. g_List[i][1] .. "[C=GREY] - " .. g_List[i][2] )
		
		for j = 1, #g_List[i][3] do
			print( " [C=GREEN]" .. g_List[i][3][j][1] .. "[C=WHITE] - " .. g_List[i][3][j][2] )
			
			for k = 1, #g_List[i][3][j][3] do
				local optional = ""
				if g_List[i][3][j][3][k][4] then
					optional = "[C=RED]Optional! [C=GREY]"
				end
				print( "    [C=YELLOW]" .. g_List[i][3][j][3][k][1] .. "[C=GREY]([C=BLUE]" .. g_List[i][3][j][3][k][2] .. "[C=GREY]) - " .. optional .. g_List[i][3][j][3][k][3] )
			end
		end
	end
end

CreateListFunction( "ListCategories", "List all function categories." )
function ListCategories()
	for i = 1, #g_List do
		print( "[C=PURPLE]" .. g_List[i][1] .. "[C=GREY] - " .. g_List[i][2] )
	end
end

CreateListFunction( "ListCategory", 			"List all functions in the specified category." )
CreateListArgument( "categoryName",	"string",	"Name of the category." )
function ListCategory( categoryName )
	for i = 1, #g_List do
		if g_List[i][1] == categoryName then
			for j = 1, #g_List[i][3] do
				print( "[C=GREEN]" .. g_List[i][3][j][1] .. "[C=WHITE] - " .. g_List[i][3][j][2] )
			end
		end
	end
end

CreateListFunction( "ListCategoryFull", 		"List all functions in the specified category. Including detailed arguments and descriptions." )
CreateListArgument( "categoryName",	"string",	"Name of the category." )
function ListCategoryFull( categoryName )
	for i = 1, #g_List do
		if g_List[i][1] == categoryName then
			for j = 1, #g_List[i][3] do
				print( "[C=GREEN]" .. g_List[i][3][j][1] .. "[C=WHITE] - " .. g_List[i][3][j][2] )
				
				for k = 1, #g_List[i][3][j][3] do
					local optional = ""
					if g_List[i][3][j][3][k][4] then
						optional = "[C=RED]Optional! [C=GREY]"
					end
					print( "   [C=YELLOW]" .. g_List[i][3][j][3][k][1] .. "[C=GREY]([C=BLUE]" .. g_List[i][3][j][3][k][2] .. "[C=GREY]) - " .. optional .. g_List[i][3][j][3][k][3] )
				end
			end
		end
	end
end

CreateListFunction( "ListFunction", 			"List all information about the function." )
CreateListArgument( "functionName",	"string",	"Name of the function." )
function ListFunction( functionName )
	for i = 1, #g_List do		
		for j = 1, #g_List[i][3] do
			if g_List[i][3][j][1] == functionName then
				print( "[C=PURPLE]Description:")
				print( "  " .. g_List[i][3][j][2] )
				
				print( "" )
				print( "[C=PURPLE]Use:" )
				
				local lastNonOptional = #g_List[i][3][j][3]
				for k = 1, #g_List[i][3][j][3] do
					if g_List[i][3][j][3][k][4] then
						lastNonOptional = k - 1
						break
					end
				end
				
				for derp = lastNonOptional, #g_List[i][3][j][3] do
					local funcWithParams = "  " .. g_List[i][3][j][1] .. "( "
					for k = 1, derp do
						funcWithParams = funcWithParams .. "[C=BLUE]" .. g_List[i][3][j][3][k][2] .. "[C=WHITE] " .. g_List[i][3][j][3][k][1]
						if k < derp then
							funcWithParams = funcWithParams .. ", "
						else
							funcWithParams = funcWithParams .. " "
						end
					end
					funcWithParams = funcWithParams .. ")"
					print( funcWithParams )
				end
				
				print( "" )
				print( "[C=PURPLE]Parameters:" )
				
				for k = 1, #g_List[i][3][j][3] do
					local optional = ""
					if g_List[i][3][j][3][k][4] then
						optional = "[C=RED]Optional! [C=WHITE]"
					end
					print( "  " .. g_List[i][3][j][3][k][1] .. "[C=WHITE]([C=BLUE]" .. g_List[i][3][j][3][k][2] .. "[C=WHITE]) - " .. optional .. g_List[i][3][j][3][k][3] )
				end
				
				if #g_List[i][3][j][3] == 0 then
					print( "  <none>" )
				end
			end
		end
	end
end