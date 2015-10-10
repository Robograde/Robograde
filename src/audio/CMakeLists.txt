file(GLOB_RECURSE AudioSources
	"AudioEngine.h"
	"AudioEngine.cpp"
	"Sound.h"
	"Sound.cpp"
	"Channel.h"
	"Channel.cpp"
	"ListenerOrientation.h"
	"AudioDLLExport.h"
	"Wav_data.h"
	"BeatManager.h"
	"BeatManager.cpp"
	"MusicManager.h"
	"MusicManager.cpp"
	"ChannelFader.h"
	"ChannelFader.cpp"
	"sdl/AudioBuffer.h"
	"sdl/AudioEngineSDL.h"
	"sdl/AudioEngineSDL.cpp"
	"sdl/AudioFileReader.h"
	"sdl/ChannelSDL.h"
	"sdl/ChannelSDL.cpp"
	"sdl/OggFileReader.h"
	"sdl/OggFileReader.cpp"
	"sdl/SoundSDL.h"
	"sdl/SoundSDL.cpp"
	"sdl/WavFileReader.h"
	"sdl/WavFileReader.cpp"
)

add_definitions(-DAUDIO_DLL_EXPORT)
# Create library for audio
add_library(Audio SHARED ${AudioSources})
target_link_libraries(Audio Memory Utility ${standardLibraries} ${SDL2Libraries} ${OggLibrary} ${VorbisLibrary})

install(
	TARGETS Audio DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
