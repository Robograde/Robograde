file(GLOB_RECURSE MessagingSources
	"*.h"
	"*.cpp"
)
add_definitions(-DMESSAGING_DLL_EXPORT)
# Create library for utilities
add_library(Messaging SHARED ${MessagingSources})
target_link_libraries(Messaging Utility Memory)

install(
	TARGETS Messaging DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
