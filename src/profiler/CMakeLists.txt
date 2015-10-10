file(GLOB_RECURSE ProfilerSources
	"*.h"
	"*.cpp"
)
add_definitions(-DPROFILER_DLL_EXPORT)
# Create library for utilities
add_library(Profiler SHARED ${ProfilerSources})
target_link_libraries(Profiler Utility Memory ${standardLibraries} ${SDL2Library} ${openglLibraries})

install(
	TARGETS Profiler DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
