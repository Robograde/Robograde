#pragma once

#ifdef ROBOGRADE_DEV
	#define DEV(x) x;
	#define NON_DEV(x)
#else
	#define DEV(x)
	#define NON_DEV(x) x;
#endif