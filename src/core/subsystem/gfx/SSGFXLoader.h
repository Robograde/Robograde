/**************************************************
2015 Robograde Development Team
***************************************************/

#pragma once
#include "../Subsystem.h"
#include "../../CompileFlags.h"

#ifdef ROBOGRADE_DEV
	//#define LOW_RES_TEXTURES
#endif

// Conveniency access function
#define g_SSGFXLoader SSGFXLoader::GetInstance()

class SSGFXLoader : public Subsystem
{
public:
	static SSGFXLoader& GetInstance();

	void Startup() override;
	void Shutdown() override;

private:
	// No external instancing allowed
	SSGFXLoader() : Subsystem( "GFXLoader", "Graphics Loader" ) {}
	SSGFXLoader( const SSGFXLoader& rhs );
	~SSGFXLoader() {};
	SSGFXLoader& operator=( const SSGFXLoader& rhs );
};
