/**************************************************
2015 Ola Enberg
***************************************************/

#include "SSFogOfWar.h"
#include "../../CompileFlags.h"

#include <gfx/GraphicsEngine.h>
#include <gfx/BufferManager.h>
#include <profiler/AutoGPUProfiler.h>
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/VisionComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/OwnerComponent.h"
#include "../../picking/PickingType.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/Alliances.h"
#include "../gamelogic/SSCollision.h"
#include "core/component/TerrainFollowComponent.h"


SSFogOfWar& SSFogOfWar::GetInstance()
{
	static SSFogOfWar instance;
	return instance;
}

void SSFogOfWar::Startup()
{
	m_FogActive			= true;
	m_UsedVisionIndex	= FOG_OF_WAR_VISUAL_TEXTURE_INDEX;
	m_CPUTileSize		= glm::vec2( 1.0f );
	m_GPUTileSize		= glm::vec2( 1.0f );
	m_CurrentStartRow	= 0;
	m_RowsPerUpdate		= FOG_OF_WAR_CPU_TEXTURE_HEIGHT;
	m_TextureHandle		= -1;
	m_GenTextureHandle	= -1;
	m_GenerationShader	= 0;
	m_FilterShader		= 0;

	DEV(
		g_Script.Register( "GE_SetFogActive",				std::bind( &SSFogOfWar::ScriptSetFogActive,				this, std::placeholders::_1 ) );
		g_Script.Register( "GE_ClearVisionGivingPlayers",	std::bind( &SSFogOfWar::ScriptClearVisionGivingPlayers,	this, std::placeholders::_1 ) );
		g_Script.Register( "GE_AddVisionGivingPlayer",		std::bind( &SSFogOfWar::ScriptAddVisionGivingPlayer,	this, std::placeholders::_1 ) );
		g_Script.Register( "GE_RemoveVisionGivingPlayer",	std::bind( &SSFogOfWar::ScriptRemoveVisionGivingPlayer,	this, std::placeholders::_1 ) );
		g_Script.Register( "GE_SetUsedPlayerVision",		std::bind( &SSFogOfWar::ScriptSetUsedPlayerVision,		this, std::placeholders::_1 ) );
	);

	for ( int i = 0; i < g_PlayerData.GetPlayerCount(); ++i )
	{
		if ( g_Alliances.IsAllied( i, g_PlayerData.GetPlayerID() ) )
		{
			this->AddVisionGivingPlayer( i );
		}
	}

	const GameModeType& gameModeType = g_GameModeSelector.GetCurrentGameMode().Type;
	if ( gameModeType == GameModeType::Replay || gameModeType == GameModeType::AIOnly ) 
	{
		this->SetFogActive( false );
	}

	rVector<int> groupsAffectedByFog;
	groupsAffectedByFog.push_back( PICKING_TYPE_FRIEND );
	groupsAffectedByFog.push_back( PICKING_TYPE_ENEMY );
	groupsAffectedByFog.push_back( PICKING_TYPE_RESOURCE );
	groupsAffectedByFog.push_back( PICKING_TYPE_UNIT_PROP );
	g_SSCollision.SetGroupsAffectedByFog( groupsAffectedByFog );
	
	for ( int i = 0; i < FOG_OF_WAR_TEXTURE_COUNT; ++i )
	{
		m_Texture[i].resize( FOG_OF_WAR_CPU_TEXTURE_HEIGHT );
		for ( int row = 0; row < m_Texture[i].size(); ++row )
		{
			m_Texture[i][row].resize( FOG_OF_WAR_CPU_TEXTURE_WIDTH );
			for ( int column = 0; column < m_Texture[i][row].size(); ++column )
			{
				m_Texture[i][row][column] = ((i == FOG_OF_WAR_VISUAL_TEXTURE_INDEX) && !m_FogActive);
			}
		}
	}

	glGenTextures( 1, &m_TextureHandle );
	glBindTexture( GL_TEXTURE_2D, m_TextureHandle );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, FOG_OF_WAR_GPU_TEXTURE_WIDTH, FOG_OF_WAR_GPU_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	gfx::g_GFXEngine.SetFogOfWarInfo( m_TextureHandle, glm::vec2( g_GameData.GetFieldWidth(), g_GameData.GetFieldHeight() ) );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glGenTextures( 1, &m_GenTextureHandle );
	glBindTexture( GL_TEXTURE_2D, m_GenTextureHandle );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, FOG_OF_WAR_GPU_TEXTURE_WIDTH, FOG_OF_WAR_GPU_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	m_VisionBufferName = "VisionBuffer";
	gfx::g_BufferManager.CreateBuffer( m_VisionBufferName, GL_SHADER_STORAGE_BUFFER, 16 * 9216, GL_DYNAMIC_DRAW );

	m_GenerationShader = gfx::g_ShaderBank.LoadShaderProgram( "../../../shader/FogGen.glsl" );
	gfx::g_BufferManager.BindBufferToProgram( m_VisionBufferName, gfx::g_ShaderBank.GetProgramFromHandle(m_GenerationShader), 11 );

	m_FilterShader = gfx::g_ShaderBank.LoadShaderProgram( "../../../shader/FogFilter.glsl" );

	g_SSCollision.SetFogOfWarInfo( m_Texture[m_UsedVisionIndex], glm::ivec2( FOG_OF_WAR_CPU_TEXTURE_WIDTH, FOG_OF_WAR_CPU_TEXTURE_HEIGHT ), m_CPUTileSize );

	if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Editor )
		SetFogActive( false );

	Subsystem::Startup();
}

void SSFogOfWar::Shutdown()
{
	this->ClearVisionGivingPlayers();
	glDeleteTextures( 1, &m_TextureHandle );

	for ( int i = 0; i < FOG_OF_WAR_TEXTURE_COUNT; ++i )
	{
		for ( auto& row : m_Texture[i] )
		{
			row.clear();
		}
		m_Texture[i].clear();
	}
	m_VisionBuffer.clear();

	Subsystem::Shutdown();
}

void SSFogOfWar::UpdateSimLayer( const float deltaTime )
{
	int width = g_GameData.GetFieldWidth();
	int height = g_GameData.GetFieldHeight();

	m_CPUTileSize.x = static_cast<float>(width) / FOG_OF_WAR_CPU_TEXTURE_WIDTH;
	m_CPUTileSize.y = static_cast<float>(height) / FOG_OF_WAR_CPU_TEXTURE_HEIGHT;
	m_GPUTileSize.x = static_cast<float>(width) / FOG_OF_WAR_GPU_TEXTURE_WIDTH;
	m_GPUTileSize.y = static_cast<float>(height) / FOG_OF_WAR_GPU_TEXTURE_HEIGHT;

	EntityMask visionFlag		= DenseComponentCollection<VisionComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask placementFlag	= DenseComponentCollection<PlacementComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask ownerFlag		= DenseComponentCollection<OwnerComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask combinedFlag		= visionFlag | placementFlag;

	for ( short playerID = 0; playerID < FOG_OF_WAR_TEXTURE_COUNT; ++playerID )
	{
		m_VisionBuffer.clear();

		if ( (playerID != FOG_OF_WAR_VISUAL_TEXTURE_INDEX) || m_FogActive )
		{
			int entityID = 0;
			for ( auto& entityMask : g_EntityManager.GetEntityMasks() )
			{
 				if ( ( entityMask & combinedFlag ) != combinedFlag )
 				{
					++entityID;
 					continue;
 				}
		
 				VisionComponent*	visionComponent		= GetDenseComponent<VisionComponent>( entityID );
				PlacementComponent*	placementComponent	= GetDenseComponent<PlacementComponent>( entityID );

				if ( !(entityMask & ownerFlag) || CalcPlayerGivesVision( GetDenseComponent<OwnerComponent>( entityID )->OwnerID, playerID ) )
				{
					//m_VisionBuffer.push_back( VisionGiver { placementComponent->Position, visionComponent->VisionRange } );

					//no uphill vision
					float range = visionComponent->VisionRange * 0.66667f;
					for (int i = -1; i < 2; i++)
					{
						for (int j = -1; j < 2; j++)
						{
							TerrainFollowComponent* tfc = GetDenseComponent<TerrainFollowComponent>(entityID);

							float x = i*range + placementComponent->Position.x;
							float z = j*range + placementComponent->Position.z;
							float y = gfx::g_GFXTerrain.GetHeightAtWorldCoord(x, z);
							glm::vec3 pos = placementComponent->Position + glm::vec3(i*range, y, j*range);

 							if (y <= placementComponent->Position.y +2)
 							{
								m_VisionBuffer.push_back(VisionGiver{ pos, visionComponent->VisionRange });
							}
						}
					}

				}
				++entityID;
			}
		}
		else
		{
			m_VisionBuffer.push_back( VisionGiver { glm::vec3( 0.0f ), 12345678.9f } );
		}

		for ( int row = m_CurrentStartRow; row < m_CurrentStartRow + m_RowsPerUpdate; ++row )
		{
			for ( int column = 0; column < m_Texture[playerID][row].size(); ++column )
			{
				m_Texture[playerID][row][column] = ((playerID == FOG_OF_WAR_VISUAL_TEXTURE_INDEX) && !m_FogActive);
			}
		}

		for ( auto& visionGiver : m_VisionBuffer )
		{
			CalculateFog( playerID, visionGiver.Position, visionGiver.Range );
		}

		if ( playerID == m_UsedVisionIndex )
		{
			gfx::ShaderProgram* generateShaderProgram = gfx::g_ShaderBank.GetProgramFromHandle( m_GenerationShader );
			generateShaderProgram->Apply();
			glBindImageTexture( 0, m_GenTextureHandle, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8 );
			gfx::g_BufferManager.UpdateBuffer( "VisionBuffer", 0, m_VisionBuffer.data(), sizeof(VisionGiver) * static_cast<int>(m_VisionBuffer.size()) );
			generateShaderProgram->SetUniformVec2( "g_TileSize", m_GPUTileSize );
			generateShaderProgram->SetUniformUInt( "g_VisionGiverCount", static_cast<int>(m_VisionBuffer.size()) );
			GPU_PROFILE( AutoGPUProfiler gpuGenProfile( "FogOfWarGeneration", true ); );
			glDispatchCompute( FOG_OF_WAR_GPU_TEXTURE_WIDTH / 32, FOG_OF_WAR_GPU_TEXTURE_HEIGHT / 32, 1 );
			GPU_PROFILE( gpuGenProfile.Stop() );

			gfx::ShaderProgram* filterShaderProgram = gfx::g_ShaderBank.GetProgramFromHandle( m_FilterShader );
			filterShaderProgram->Apply();
			//bind output texture
			glBindImageTexture( 0, m_TextureHandle, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8 );
			//bind input texture
			filterShaderProgram->SetUniformTextureHandle("g_GenFogTex",m_GenTextureHandle, 1);
			//GPU_PROFILE( AutoGPUProfiler gpuFilterProfile( "FogOfWarFiltering", true ); );
			glDispatchCompute( FOG_OF_WAR_GPU_TEXTURE_WIDTH / 32, FOG_OF_WAR_GPU_TEXTURE_HEIGHT / 32, 1 );
			//GPU_PROFILE( gpuFilterProfile.Stop() );
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			//glBindImageTexture( 0, m_GenTextureHandle, 0, false, 0, GL_READ_WRITE, GL_RGBA8 );
			//filterShaderProgram->SetUniformTextureHandle("g_GenFogTex",m_TextureHandle, 1);
			//glDispatchCompute( FOG_OF_WAR_GPU_TEXTURE_WIDTH / 32, FOG_OF_WAR_GPU_TEXTURE_HEIGHT / 32, 1 );
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			//glBindImageTexture( 0, m_TextureHandle, 0, false, 0, GL_READ_WRITE, GL_RGBA8 );
			//filterShaderProgram->SetUniformTextureHandle("g_GenFogTex",m_GenTextureHandle, 1);
			//glDispatchCompute( FOG_OF_WAR_GPU_TEXTURE_WIDTH / 32, FOG_OF_WAR_GPU_TEXTURE_HEIGHT / 32, 1 );

			gfx::g_GFXEngine.SetFogOfWarInfo(m_TextureHandle, glm::vec2(width, height));
			g_SSCollision.SetFogOfWarInfo( m_Texture[m_UsedVisionIndex], glm::ivec2( FOG_OF_WAR_CPU_TEXTURE_WIDTH, FOG_OF_WAR_CPU_TEXTURE_HEIGHT ), m_CPUTileSize );
		}
	}

	m_CurrentStartRow += m_RowsPerUpdate;
	if ( m_CurrentStartRow >= FOG_OF_WAR_CPU_TEXTURE_HEIGHT )
	{
		m_CurrentStartRow = 0;
	}

	if ( m_RowsPerUpdate == FOG_OF_WAR_CPU_TEXTURE_HEIGHT )
		m_RowsPerUpdate = FOG_OF_WAR_ROWS_PER_UPDATE;
}

GLuint SSFogOfWar::GetTextureHandle() const
{
	return m_TextureHandle;
}

bool SSFogOfWar::CalcVisibilityForPosition( const glm::vec3& position ) const
{
	return this->CalcPlayerVisibilityForPosition( m_UsedVisionIndex, position );
}

bool SSFogOfWar::CalcPlayerVisibilityForPosition( short playerID, const glm::vec3& position ) const
{
	if ( (playerID < 0) || (playerID >= FOG_OF_WAR_TEXTURE_COUNT ) )
	{
		return true;
	}

	const int x = static_cast<int>( position.x / m_CPUTileSize.x);
	const int z = static_cast<int>( position.z / m_CPUTileSize.y);

	if ( x < 0 || z < 0 || x >= FOG_OF_WAR_CPU_TEXTURE_WIDTH || z >= FOG_OF_WAR_CPU_TEXTURE_HEIGHT )
	{
		return false;
	}

	return m_Texture[playerID][z][x];
}

void SSFogOfWar::SetFogActive( bool newFogActive )
{
	m_FogActive = newFogActive;
}

void SSFogOfWar::ClearVisionGivingPlayers()
{
	m_VisionGivingPlayers.clear();
}

void SSFogOfWar::AddVisionGivingPlayer( short playerID )
{
	for ( auto& visionGivingPlayer : m_VisionGivingPlayers )
	{
		if ( playerID == visionGivingPlayer )
		{
			return;
		}
	}
	m_VisionGivingPlayers.push_back( playerID );
}

void SSFogOfWar::RemoveVisionGivingPlayer( short playerID )
{
	for ( int i = static_cast<int>(m_VisionGivingPlayers.size()) - 1; i >= 0; --i )
	{
		if ( m_VisionGivingPlayers[i] == playerID )
		{
			m_VisionGivingPlayers.erase( m_VisionGivingPlayers.begin() + i );
		}
	}
}

bool SSFogOfWar::CalcPlayerGivesVision( short playerID, short fogID )
{
	if ( fogID == FOG_OF_WAR_VISUAL_TEXTURE_INDEX )
	{
		for ( auto& visionGivingPlayer : m_VisionGivingPlayers )
		{
			if ( playerID == visionGivingPlayer )
			{
				return true;
			}
		}
	}
	else
	{
		return g_Alliances.IsAllied( playerID, fogID );
	}
	return false;
}

void SSFogOfWar::CalculateFog( short playerID, const glm::vec3& position, const float radius )
{
	const int minX = glm::max( static_cast<int>((position.x - radius) / m_CPUTileSize.x), 0 );
	const int minZ = glm::max( static_cast<int>((position.z - radius) / m_CPUTileSize.y), m_CurrentStartRow );
	const int maxX = glm::min( static_cast<int>((position.x + radius) / m_CPUTileSize.x), FOG_OF_WAR_CPU_TEXTURE_WIDTH - 1 );
	const int maxZ = glm::min( static_cast<int>((position.z + radius) / m_CPUTileSize.y), m_CurrentStartRow + m_RowsPerUpdate - 1 );

	for ( int z = minZ; z <= maxZ; ++z )
	{
		for ( int x = minX; x <= maxX; ++x )
		{
			glm::vec2 tilePos( (static_cast<float>(x) + 0.5f) * m_CPUTileSize.x, (static_cast<float>(z) + 0.5f) * m_CPUTileSize.y );
			glm::vec2 dist = tilePos - glm::vec2( position.x, position.z );

			if ( glm::dot( dist, dist ) <= radius * radius )
			{
				m_Texture[playerID][z][x] = true;
			}
		}
	}
}

int SSFogOfWar::ScriptSetFogActive( IScriptEngine* scriptEngine )
{
	bool fogActive	= scriptEngine->PopBool();

	this->SetFogActive( fogActive );

	return 0;	// Number of return values.
}

int SSFogOfWar::ScriptClearVisionGivingPlayers( IScriptEngine* scriptEngine )
{
	this->ClearVisionGivingPlayers();

	return 0;	// Number of return values.
}

int SSFogOfWar::ScriptAddVisionGivingPlayer( IScriptEngine* scriptEngine )
{
	int playerID = scriptEngine->PopInt();

	this->AddVisionGivingPlayer( playerID );

	return 0;	// Number of return values.
}

int SSFogOfWar::ScriptRemoveVisionGivingPlayer( IScriptEngine* scriptEngine )
{
	int playerID = scriptEngine->PopInt();

	this->RemoveVisionGivingPlayer( playerID );

	return 0;	// Number of return values.
}

int SSFogOfWar::ScriptSetUsedPlayerVision( IScriptEngine* scriptEngine )
{
	int playerID = scriptEngine->PopInt();

	m_UsedVisionIndex = playerID;

	return 0;
}
