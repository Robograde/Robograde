file(GLOB_RECURSE MemorySources
	"Allocator.h"
	"Allocator.cpp"
	"STLAllocator.h"
	"Alloc.h"
	"Alloc.cpp"
	"rString.h"
	"rString.cpp"
	"AllocationPatterns.h"
	"AllocationPatterns.cpp"
	"FrameAllocator.h"
	"FrameAllocator.cpp"
	"MemoryLibraryDefine.h"
)

add_definitions(-DMEMORY_DLL_EXPORT)
# Create library for utilities
add_library(Memory SHARED ${MemorySources})
target_link_libraries(Memory ${standardLibraries})

install(
	TARGETS Memory DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
