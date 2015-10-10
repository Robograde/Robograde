/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
namespace Colors
{
#ifdef __unix__
	// Reset
	static const char* Reset =		"\e[0m";

	// Regular
	static const char* Black =		"\e[0;30m";
	static const char* Red =		"\e[0;31m";
	static const char* Green =		"\e[0;32m";
	static const char* Yellow =		"\e[0;33m";
	static const char* Blue =		"\e[0;34m";
	static const char* Purple =		"\e[0;35m";
	static const char* Cyan =		"\e[0;36m";
	static const char* White =		"\e[0;37m";

	// Bold
	static const char* BBlack =		"\e[1;30m";
	static const char* BRed =		"\e[1;31m";
	static const char* BGreen =		"\e[1;32m";
	static const char* BYellow =	"\e[1;33m";
	static const char* BBlue =		"\e[1;34m";
	static const char* BPurple =	"\e[1;35m";
	static const char* BCyan =		"\e[1;36m";
	static const char* BWhite =		"\e[1;37m";

	// Underline
	static const char* UBlack =		"\e[4;30m";
	static const char* URed =		"\e[4;31m";
	static const char* UGreen =		"\e[4;32m";
	static const char* UYellow =	"\e[4;33m";
	static const char* UBlue =		"\e[4;34m";
	static const char* UPurple =	"\e[4;35m";
	static const char* UCyan =		"\e[4;36m";
	static const char* UWhite =		"\e[4;37m";

	// High Intensity
	static const char* IBlack =		"\e[0;90m";
	static const char* IRed =		"\e[0;91m";
	static const char* IGreen =		"\e[0;92m";
	static const char* IYellow =	"\e[0;93m";
	static const char* IBlue =		"\e[0;94m";
	static const char* IPurple =	"\e[0;95m";
	static const char* ICyan =		"\e[0;96m";
	static const char* IWhite =		"\e[0;97m";

	// Bold High Intensity
	static const char* BIBlack =	"\e[1;90m";
	static const char* BIRed =		"\e[1;91m";
	static const char* BIGreen =	"\e[1;92m";
	static const char* BIYellow =	"\e[1;93m";
	static const char* BIBlue =		"\e[1;94m";
	static const char* BIPurple =	"\e[1;95m";
	static const char* BICyan =		"\e[1;96m";
	static const char* BIWhite =	"\e[1;97m";

	// Background
	static const char* BGBlack =	"\e[40m";
	static const char* BGRed =		"\e[41m";
	static const char* BGGreen =	"\e[42m";
	static const char* BGYellow =	"\e[43m";
	static const char* BGBlue =		"\e[44m";
	static const char* BGPurple =	"\e[45m";
	static const char* BGCyan =		"\e[46m";
	static const char* BGWhite =	"\e[47m";

	// High Intensity Backgrounds
	static const char* IBGBlack =	"\e[0;100m";
	static const char* IBGRed =		"\e[0;101m";
	static const char* IBGGreen =	"\e[0;102m";
	static const char* IBGYellow =	"\e[0;103m";
	static const char* IBGBlue =	"\e[0;104m";
	static const char* IBGPurple =	"\e[0;105m";
	static const char* IBGCyan =	"\e[0;106m";
	static const char* IBGWhite =	"\e[0;107m";

#elif defined(_WIN32) || defined(WIN32)
	// Reset
	static const char* Reset =		"";

	// Regular
	static const char* Black =		"";
	static const char* Red =		"";
	static const char* Green =		"";
	static const char* Yellow =		"";
	static const char* Blue =		"";
	static const char* Purple =		"";
	static const char* Cyan =		"";
	static const char* White =		"";

	// Bold
	static const char* BBlack =		"";
	static const char* BRed =		"";
	static const char* BGreen =		"";
	static const char* BYellow =	"";
	static const char* BBlue =		"";
	static const char* BPurple =	"";
	static const char* BCyan =		"";
	static const char* BWhite =		"";

	// Underline
	static const char* UBlack =		"";
	static const char* URed =		"";
	static const char* UGreen =		"";
	static const char* UYellow =	"";
	static const char* UBlue =		"";
	static const char* UPurple =	"";
	static const char* UCyan =		"";
	static const char* UWhite =		"";

	// High Intensity
	static const char* IBlack =		"";
	static const char* IRed =		"";
	static const char* IGreen =		"";
	static const char* IYellow =	"";
	static const char* IBlue =		"";
	static const char* IPurple =	"";
	static const char* ICyan =		"";
	static const char* IWhite =		"";

	// Bold High Intensity
	static const char* BIBlack =	"";
	static const char* BIRed =		"";
	static const char* BIGreen =	"";
	static const char* BIYellow =	"";
	static const char* BIBlue =		"";
	static const char* BIPurple =	"";
	static const char* BICyan =		"";
	static const char* BIWhite =	"";

	// Background
	static const char* BGBlack =	"";
	static const char* BGRed =		"";
	static const char* BGGreen =	"";
	static const char* BGYellow =	"";
	static const char* BGBlue =		"";
	static const char* BGPurple =	"";
	static const char* BGCyan =		"";
	static const char* BGWhite =	"";

	// High Intensity Backgrounds
	static const char* IBGBlack =	"";
	static const char* IBGRed =		"";
	static const char* IBGGreen =	"";
	static const char* IBGYellow =	"";
	static const char* IBGBlue =	"";
	static const char* IBGPurple =	"";
	static const char* IBGCyan =	"";
	static const char* IBGWhite =	"";

#endif
}
