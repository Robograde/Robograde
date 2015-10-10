/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "DecalManager.h"
#include "BufferManager.h"
#include <utility/Logger.h>


gfx::DecalManager::DecalManager( )
{
	m_Buffer = "DecalBuffer";
}

gfx::DecalManager::~DecalManager()
{
	this->Shutdown( );
}

gfx::DecalManager& gfx::DecalManager::GetInstance()
{
	static DecalManager m_Instance;
	return m_Instance;
}

void gfx::DecalManager::Initialize()
{
	//m_Decals = HandleAllocator<DecalHandle, Decal, MAX_DECALS>( m_InvalidDecal );
	g_BufferManager.CreateBuffer( m_Buffer, GL_SHADER_STORAGE_BUFFER, DECAL_BUFFER_SIZE, GL_DYNAMIC_DRAW );

	m_TextureAtlas.LoadFile( "../../../asset/texture/decal.json" );
	const int framesOffset = MAX_DECALS * sizeof(Decal);

	g_BufferManager.UpdateBuffer( m_Buffer, framesOffset, m_TextureAtlas.GetGPUTextures().data(), (int)m_TextureAtlas.GetGPUTextures().size() * sizeof( GPUTexture ) );
}

void gfx::DecalManager::Update()
{
	for ( auto& decal : m_Decals )
	{
		decal.InvWorld = glm::inverse( decal.World );
	}
	g_BufferManager.UpdateBuffer( m_Buffer, 0, m_Decals.data(), GetDecalCount() * sizeof( Decal ) );
}

void gfx::DecalManager::Shutdown()
{
}

void gfx::DecalManager::AddToQueue( const Decal& decal )
{
	assert( m_Decals.size() + 1 < MAX_DECALS );
	m_Decals.push_back( decal );
}

void gfx::DecalManager::Clear( )
{
	m_Decals.clear( );
}

int gfx::DecalManager::GetDecalCount( ) const
{
	return static_cast<int>( m_Decals.size( ) );
}
