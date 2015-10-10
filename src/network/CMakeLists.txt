file(GLOB_RECURSE NetworkSources
	"*.h"
	"*.cpp"
)
add_definitions(-DNETWORK_DLL_EXPORT)
# Create library for utilities
add_library(Network SHARED ${NetworkSources})
target_link_libraries(Network ${SDL2Library} Utility Memory Messaging)

install(
	TARGETS Network DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
