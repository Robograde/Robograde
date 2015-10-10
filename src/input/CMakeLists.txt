file(GLOB_RECURSE InputSources
	"Input.h"
	"Input.cpp"
	"TextInput.h"
	"TextInput.cpp"
	"KeyBindings.h"
	"KeyBindings.cpp"
	"KeyBindingCollection.h"
	"KeyBindingCollection.cpp"
	"Typedefs.h"
)

add_definitions(-DINPUT_DLL_EXPORT)
# Create library for input
add_library(Input SHARED ${InputSources})
target_link_libraries(Input Memory Utility ${SDL2Library})

## Input tests
#if(${CREATE_TESTS})
#	add_executable(Input_test test/TestKeyBindings.cpp)
#	target_link_libraries(Input_test Input Memory Utility ${standardLibraries} ${testLibraries})
#endif()

install(
	TARGETS Input DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
