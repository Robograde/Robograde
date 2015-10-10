file(GLOB_RECURSE UtilitySources
	"Logger.h"
	"Logger.cpp"
	"Config.h"
	"Config.cpp"
	"CallbackConfig.h"
	"CallbackConfig.cpp"
	"ConfigManager.h"
	"ConfigManager.cpp"
	"FrameCounter.h"
	"FrameCounter.cpp"
	"Colors.h"
	"UtilityLibraryDefine.h"
	"ThreadingUtility.h"
	"PlatformDefinitions.h"
	"LocklessQueue.h"
	"GameTimer.h"
	"GameTimer.cpp"
	"RetardedWindowsIncludes.h"
	"SerializationUtility.h"
	"SerializationUtility.cpp"
	"Randomizer.h"
	"Colours.h"
	"FileUtility.h"
	"FileUtility.cpp"
	"StringUtility.h"
	"CallbackMessage.h"
	"CallbackMessage.cpp"
	"CallbackPump.h"
	"CallbackPump.cpp"
	"CallbackCollection.h"
	"CallbackCollection.cpp"
	"NeatFunctions.h"
	"DataSizes.h"
	"Handle.h"
	"HandleAllocator.h"
)

add_definitions(-DUTILITY_DLL_EXPORT)
# Create library for utilities
add_library(Utility SHARED ${UtilitySources})
target_link_libraries(Utility Memory ${standardLibraries} ${SDL2Library})

# Utility tests
add_executable(Utility_test test/TestConfig.cpp)
target_link_libraries(Utility_test Utility Memory ${standardLibraries} ${testLibraries})

install(
	TARGETS Utility DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
