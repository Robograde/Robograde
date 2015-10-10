file(GLOB_RECURSE GUISources
	"*.h"
	"*.cpp"
	"graphics2D/*.h"
	"graphics2D/*.cpp"
	"graphics2D/tempGfxUtility/*.h"
	"graphics2D/tempGfxUtility/*.cpp"
)

# Generate source groups depending on folders (creates Visual Studio filters)
set(RELPATH ${ProjectRoot}/src/)
get_filename_component(RELPATH ${RELPATH} ABSOLUTE)
foreach(f ${GUISources})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${RELPATH} ${f})
    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

add_definitions(-DGUI_DLL_EXPORT)
# Create library for gui
add_library(GUI SHARED ${GUISources})
#target_link_libraries(GUI Utility ${FreeTypeLibrary})
target_link_libraries(GUI Utility Memory gfx Input Script ${SOILLibrary} ${openglLibraries} ${FREETYPE_LIBRARIES} ${standardLibraries})

install(
	TARGETS GUI DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
