file(GLOB_RECURSE GFXSources
	"*.h"
	"*.cpp"
	"*/*.h"
	"*/*.cpp"
)
add_definitions(-DGFX_DLL_EXPORT)
add_library(gfx SHARED ${GFXSources})
target_link_libraries(gfx Utility Memory Profiler ${SOIL2Library} ${openglLibraries} ${SDL2Library})

install(
	TARGETS gfx DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
