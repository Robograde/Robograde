/**************************************************
Copyright 2015 Robograde Development Team
***************************************************/

#pragma once
#include <gfx/RenderJobManager.h>
#include <gfx/Lights.h>
#include <network/NetworkEngine.h>
#include <utility/Config.h>
#include <gfx/DebugRenderer.h>
#include <gui/GUIEngine.h>

class Engine
{
public:
	bool Initialize();
	void Run();
	void Cleanup();

private:
	void Update( const GameTimer::DeltaTimeInfo& deltaTimeInfo );
	void Render();

	bool InitializeGFX();
	bool InitializeSubsystems();
	bool InitializeAudio();
	bool InitializeScene();

	Config 					m_ConfigGraphics;

	gfx::DebugRenderer		m_debugRenderer;
};