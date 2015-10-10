file(GLOB_RECURSE CollisionDetectionSources
	"ICollisionDetection.h"
	"ICollisionDetection.cpp"
	"engine/*.h"
	"engine/*.cpp"
	"entity/*.h"
	"entity/*.cpp"
	"detection/*.h"
	"detection/*.cpp"
	"volume/*.h"
	"volume/*.cpp"
)

# Generate source groups depending on folders (creates Visual Studio filters)
set(RELPATH ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(RELPATH ${RELPATH} ABSOLUTE)
foreach(f ${CollisionDetectionSources})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${RELPATH} ${f})
    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
	if( ${SRCGR} MATCHES "[*.cpp|*.h|*.hpp]" )
		source_group("src" FILES ${f})
	else()
		source_group("src\\${SRCGR}" FILES ${f})
	endif()
endforeach()

add_definitions(-DCOLLISION_DETECTION_DLL_EXPORT)
# Create library for Collision Detection
add_library(CollisionDetection SHARED ${CollisionDetectionSources})
target_link_libraries( CollisionDetection Memory Utility ${SDL2Library})

# Collision Detection tests
file(GLOB_RECURSE CollisionDetectionTestSources
	"test/CollisionLibraryInterfaceTest.cpp"
	"test/CollisionVolumesTest.cpp"
	"test/RayVsPlaneTest.cpp"
	"test/RayVsSphereTest.cpp"
)
add_executable(CollisionDetection_test ${CollisionDetectionTestSources} ${CollisionDetectionSources} )
target_link_libraries(CollisionDetection_test Utility Memory ${standardLibraries} ${testLibraries})

install(
	TARGETS CollisionDetection DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
