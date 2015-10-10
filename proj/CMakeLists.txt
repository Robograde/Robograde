cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
#set(CMAKE_BUILD_TYPE Debug CACHE STRING "What build type to create (Debug/Release)." FORCE)

option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." ON)
option(CREATE_TESTS "Create tests." ON)
option(DEV_BUILD "Wheter to build developer functionality." ON)
option(CPU_PROFILE "CPU Profiling." ON)
option(GPU_PROFILE "GPU Profiling." OFF)

if (UNIX)
	set(CMAKE_CXX_FLAGS_DEBUG 	"${CMAKE_CXX_FLAGS_DEBUG} -g -Wno-ignored-attributes -Wno-deprecated -Wno-shift-op-parentheses -fexceptions")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")
	set(CMAKE_LINKER 			"/usr/bin/llvm-ld")
	set(CMAKE_CXX_COMPILER 		"/usr/bin/clang++")
	set(CMAKE_C_COMPILER 		"/usr/bin/clang")
	# Verbose output linking
	#set(CMAKE_EXE_LINKER_FLAGS 	"-v")

	add_compile_options(-stdlib=libc++)
	add_compile_options(-std=c++1y)
	add_compile_options(-m64)
endif(UNIX)
if (NOT UNIX)
	# Parallel compilation in Visual Studio
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D \"_CRT_SECURE_NO_WARNINGS\"")

	# Ignore the "PDB not found" linker error and fix warning LNK4098: defaultlib 'msvcrt.lib' conflicts with use of other libs
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /ignore:4099 /NODEFAULTLIB:\"MSVCRT\"")
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /ignore:4099 /NODEFAULTLIB:\"MSVCRTD\"")
	SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:\"MSVCRT\"")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:\"MSVCRTD\"")
endif(NOT UNIX)
add_definitions(-D__extern_always_inline=inline)
add_definitions(-DGLM_FORCE_RADIANS)
# Developer build flag
if(${DEV_BUILD})
	message("Building devoloper version")
	add_definitions(-DROBOGRADE_DEV)
	add_definitions(-DDEBUG_MEMORY)
	set(BUILD_PREFIX_NAME dev)
else()
	message("Building non-dev version")
	set(BUILD_PREFIX_NAME nondev)
endif(${DEV_BUILD})
# CPU Profiling flag
if(${GPU_PROFILE})
	message("Building with GPU profiling")
	add_definitions(-DROBOGRADE_GPU_PROFILING)
endif(${GPU_PROFILE})
# GPU Profiling flag
if(${CPU_PROFILE})
	message("Building with CPU profiling")
	add_definitions(-DROBOGRADE_CPU_PROFILING)
endif(${CPU_PROFILE})

# I name thee
project(Robograde)

# enable_testing()

# Define a variable for the root of the project (Not the project file)
set(ProjectRoot ${Robograde_SOURCE_DIR}/../)
get_filename_component(ProjectRootAbsolute ${ProjectRoot} ABSOLUTE)

# Include directory
file(RELATIVE_PATH IncludeDirectoryRelative ${Robograde_SOURCE_DIR} ${ProjectRootAbsolute}/include)
get_filename_component(IncludeDirectoryAbsolute ${IncludeDirectoryRelative} ABSOLUTE)

# Library directory
file(RELATIVE_PATH LibraryDirectoryRelative ${Robograde_SOURCE_DIR} ${ProjectRootAbsolute}/lib/64-bit)
get_filename_component(LibraryDirectoryAbsolute ${LibraryDirectoryRelative} ABSOLUTE)

# Project settings
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()
string(TOLOWER ${CMAKE_SYSTEM_NAME} OperatingSystemNameLowerCase)
if(MSVC)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ProjectRoot}/bin/${OperatingSystemNameLowerCase}/)
endif(MSVC)
if(UNIX)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ProjectRoot}/bin/${OperatingSystemNameLowerCase}/${CMAKE_BUILD_TYPE})
endif(UNIX)

set(CMAKE_INSTALL_PREFIX "${ProjectRoot}Robograde")
set(TargetArchitecture "x86_64")

# Find SDL2
#INCLUDE(FindPkgConfig)
#PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
find_path(SDL2Include SDL2/SDL.h
	PATHS ${IncludeDirectoryAbsolute})
set(SDL2Includes ${SDL2Include})
find_library(SDL2Library
	NAMES SDL2 SDL2main
	PATHS ${LibraryDirectoryAbsolute})
set(SDL2Libraries ${SDL2Library})
if (MSVC)
	find_library(SDL2mainLibrary
		NAMES SDL2main
		PATHS ${LibraryDirectoryAbsolute})
	list(APPEND SDL2Libraries ${SDL2mainLibrary})
endif(MSVC)

# Find glew
find_path(GLEWInclude GL/glew.h
	PATHS ${IncludeDirectoryAbsolute})
find_library(GLEWLibrary
	NAMES glew32 glew GLEW
	PATHS ${LibraryDirectoryAbsolute})
list(APPEND openglLibraries ${GLEWLibrary})

# Find lua
find_path(LUAInclude lua.hpp
    PATHS ${IncludeDirectoryAbsolute}/lua/)
find_library(LUALibrary
    NAMES lua lua52
    PATHS ${LibraryDirectoryAbsolute})

# Find ogg
find_path(OggInclude ogg.h
    PATHS ${IncludeDirectoryAbsolute}/ogg/)
find_library(OggLibrary
    NAMES libogg ogg
    PATHS ${LibraryDirectoryAbsolute})

# Find vorbis
find_path(VorbisInclude vorbisfile.h
    PATHS ${IncludeDirectoryAbsolute}/vorbis/)
find_library(VorbisLibrary
    NAMES libvorbis vorbis
    PATHS ${LibraryDirectoryAbsolute})
find_library(VorbisFileLibrary
    NAMES libvorbisfile vorbisfile
    PATHS ${LibraryDirectoryAbsolute})
list(APPEND VorbisLibrary ${VorbisFileLibrary})

# Find soil
find_path(SOIL2Include soil2/SOIL2.h
	PATHS ${IncludeDirectoryAbsolute})
find_library(SOIL2Library
	NAMES soil2 SOIL2 libsoil2 soil2-debug
	PATHS ${LibraryDirectoryAbsolute})

# Find Freetype
INCLUDE(FindFreetype)
IF(NOT FREETYPE_FOUND)
	if (UNIX)
		find_path(FreetypeInclude freetype2/ft2build.h
			PATHS ${IncludeDirectoryAbsolute})
	endif (UNIX)
	if (MSVC)
		find_path(FREETYPE_INCLUDE_DIRS ft2build.h
			PATHS ${IncludeDirectoryAbsolute}/FreeType)
	endif (MSVC)
	find_library(FREETYPE_LIBRARIES
		NAMES freetype2 freetype252
		PATHS ${LibraryDirectoryAbsolute})
ENDIF(NOT FREETYPE_FOUND)

# Google test
find_path(gtestInclude gtest/gtest.h
	PATHS ${IncludeDirectoryAbsolute})
set(gtestIncludes ${gtestInclude})

link_directories(
	${LibraryDirectoryRelative}
)

if (UNIX)
	list(APPEND standardLibraries "c++")
	list(APPEND standardLibraries "c++abi")
	list(APPEND openglLibraries "GL")
endif(UNIX)
if (MSVC)
	list(APPEND openglLibraries "opengl32")
endif(MSVC)

if(${CREATE_TESTS})
	set(testIncludes ${gtestIncludes})
	set(testLibraries gtest gtest_main)
endif()

include_directories(
	${testIncludes}
	${SDL2Includes}
	${GLEWInclude}
	${FREETYPE_INCLUDE_DIRS}
	${LUAInclude}
	${IncludeDirectoryRelative}
	${ProjectRoot}/src/
)

set(BUILD_PREFIX_PATH ../build/${BUILD_PREFIX_NAME}\ ${CMAKE_BUILD_TYPE})

# Add each separate library/executable
add_subdirectory(../src/core 				${BUILD_PREFIX_PATH}/core)
add_subdirectory(../src/utility 			${BUILD_PREFIX_PATH}/utility)
add_subdirectory(../src/gui 				${BUILD_PREFIX_PATH}/gui)
add_subdirectory(../src/profiler 			${BUILD_PREFIX_PATH}/profiler)
add_subdirectory(../src/network 			${BUILD_PREFIX_PATH}/network)
add_subdirectory(../src/input 				${BUILD_PREFIX_PATH}/input)
add_subdirectory(../src/collisionDetection 	${BUILD_PREFIX_PATH}/collisionDetection)
add_subdirectory(../src/script 				${BUILD_PREFIX_PATH}/script)
add_subdirectory(../src/audio 				${BUILD_PREFIX_PATH}/audio)
add_subdirectory(../src/gfx 				${BUILD_PREFIX_PATH}/gfx)
add_subdirectory(../src/messaging 			${BUILD_PREFIX_PATH}/messaging)
add_subdirectory(../src/memory 				${BUILD_PREFIX_PATH}/memory)

add_subdirectory(${ProjectRootAbsolute}/lib/gtest ${ProjectRootAbsolute}/lib/gtest)
