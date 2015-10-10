/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <script/ScriptEngine.h>
#include <GL/glew.h>
#include <gfx/Shader/ShaderBank.h>
#include "../../utility/GlobalDefinitions.h"

#define FOG_OF_WAR_CPU_TEXTURE_WIDTH	64
#define FOG_OF_WAR_CPU_TEXTURE_HEIGHT	64
#define FOG_OF_WAR_GPU_TEXTURE_WIDTH	128
#define FOG_OF_WAR_GPU_TEXTURE_HEIGHT	128
#define FOG_OF_WAR_ROWS_PER_UPDATE		2
#define FOG_OF_WAR_TEXTURE_COUNT		(MAX_PLAYERS + 1)
#define FOG_OF_WAR_VISUAL_TEXTURE_INDEX	MAX_PLAYERS

#define g_SSFogOfWar SSFogOfWar::GetInstance()	// Conveniency access function

struct VisionGiver
{
	glm::vec3	Position;
	float		Range;
};

class SSFogOfWar : public Subsystem
{
public:
	static SSFogOfWar&				GetInstance						();

	void							Startup							() override;
	void							Shutdown						() override;
	void							UpdateSimLayer					( const float deltaTime ) override;

									// 0 is fog, 255 is visible
	GLuint							GetTextureHandle				() const;
	bool							CalcVisibilityForPosition		( const glm::vec3& position ) const;
	bool							CalcPlayerVisibilityForPosition	( short playerID, const glm::vec3& position ) const;

	void							SetFogActive					( bool newFogActive );
	void							ClearVisionGivingPlayers		();
	void							AddVisionGivingPlayer			( short playerID );
	void							RemoveVisionGivingPlayer		( short playerID );

private:
									// No external instancing allowed
									SSFogOfWar						() : Subsystem( "FogOfWar" ) {}
									SSFogOfWar						( const SSFogOfWar & rhs );
									~SSFogOfWar						() {};
	SSFogOfWar&						operator=						( const SSFogOfWar & rhs );

	bool							CalcPlayerGivesVision			( short playerID, short fogID );
	void							CalculateFog					( short playerID, const glm::vec3& position, const float radius );

	int								ScriptSetFogActive				( IScriptEngine* scriptEngine );
	int								ScriptClearVisionGivingPlayers	( IScriptEngine* scriptEngine );
	int								ScriptAddVisionGivingPlayer		( IScriptEngine* scriptEngine );
	int								ScriptRemoveVisionGivingPlayer	( IScriptEngine* scriptEngine );
	int								ScriptSetUsedPlayerVision		( IScriptEngine* scriptEngine );
	
	bool							m_FogActive;
	short							m_UsedVisionIndex;
	glm::vec2						m_CPUTileSize;
	glm::vec2						m_GPUTileSize;
	rVector<short>					m_VisionGivingPlayers;

	rVector<rVector<bool>>			m_Texture[FOG_OF_WAR_TEXTURE_COUNT];		// Last Texture is used for visual so that "Lua FogOff()" doesn't mess up game sync"
	int								m_CurrentStartRow;
	int								m_RowsPerUpdate;

	GLuint							m_GenTextureHandle;
	GLuint							m_TextureHandle;
	rVector<VisionGiver>			m_VisionBuffer;
	rString							m_VisionBufferName;
	gfx::ShaderProgramHandle		m_GenerationShader;
	gfx::ShaderProgramHandle		m_FilterShader;
};