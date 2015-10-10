file(GLOB_RECURSE ScriptSources
	"*.h"
	"*.cpp"
)

add_definitions(-DSCRIPT_DLL_EXPORT)
# Create library for input
add_library(Script SHARED ${ScriptSources})
target_link_libraries(Script Utility Memory ${LUALibrary})

install(
	TARGETS Script DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
