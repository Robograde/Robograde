/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#include "SSParticle.h"
#include <utility/Logger.h>
#include "../gamelogic/SSCamera.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../component/AgentComponent.h"
#include "../../component/ColourComponent.h"
#include "../../datadriven/EntityManager.h"
#include "SSFogOfWar.h"
#include "../gamelogic/SSCamera.h"
#include <gfx/GraphicsEngine.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/scalar_relational.hpp>
#include "../gamelogic/SSCamera.h"
#include "../../../utility/Randomizer.h"

using namespace gfx;

SSParticle& SSParticle::GetInstance( )
{
	static SSParticle instance;
	return instance;
}

void SSParticle::Startup( )
{
	m_ParticleEngine.Initialize();
	CallbackMessageResultData messageResult = g_CallbackPump.RegisterForCallbackMessage( CALLBACK_MESSAGE_ENTITY_KILLED, std::bind( &SSParticle::EntityKilled, this, std::placeholders::_1 ) );
	// TODOJM: Implement error handling in messagepump
	if ( messageResult.CallbackMessageResult != CallbackMessageResult::OK )
	{
		Logger::Log( "Failed to register for entity killed callback message in " + this->GetName(), "SSParticle", LogSeverity::WARNING_MSG );
	}	
	m_CallbackMessageEntityKilledID = messageResult.CallbackMessageRegisterID;

	m_Textures.push_back( pNew(Texture) );
	m_Textures.back()->Init( "../../../asset/texture/Particle.png", gfx::TextureType::TEXTURE_2D );
	m_Textures.push_back( pNew( Texture ) );
	m_Textures.back( )->Init( "../../../asset/texture/SmokeCloud.png", gfx::TextureType::TEXTURE_2D );
	m_Textures.push_back( pNew( Texture ) );
	m_Textures.back( )->Init( "../../../asset/texture/Empty.png", gfx::TextureType::TEXTURE_2D );
	//m_Textures.back()->Init( "../../../asset/texture/debugparticle.jpg", gfx::TextureType::TEXTURE_2D );

	ParticleTypeSettings pts;
	pts.BlockSize = 64;
	pts.MaxNumberOfBlocks = 128;
	pts.Texture = m_Textures.at(1);
	pts.SourceFactorBlending = GL_SRC_ALPHA;
	pts.DestinationFactorBlending = GL_ONE_MINUS_SRC_ALPHA;

	gfx::ParticleSystem defaultParticleSystem;

	// TODOJM: Set bounding box size
	// Explosion
	defaultParticleSystem.ParticlesTimeToLive = 0.8f;
	defaultParticleSystem.ParticlesSpeed = 2.0f;
	defaultParticleSystem.TimeToLive = 0.800001f;
	defaultParticleSystem.Colour = glm::vec4( 0.5f, 0.5f, 0.5f, 0.4f );
	defaultParticleSystem.Size = glm::vec2( 1.2f, 1.2f );
	defaultParticleSystem.TransparencyPolynome = glm::vec2( -1.0, 0.1 );

	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.IdentifyingName = "Explosion";
	pts.InitialShaderPath = "../../../shader/particle/ExplodingInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/Exploding.glsl";
	m_ParticleTypes[PARTICLE_TYPE_EXPLODING] = m_ParticleEngine.CreateParticleType( pts );

	// Smoke cloud
	defaultParticleSystem.ParticlesTimeToLive = 1.0f;
	defaultParticleSystem.ParticlesSpeed = 15.0f;
	defaultParticleSystem.TimeToLive = 12.5f;
	defaultParticleSystem.Colour = glm::vec4( 0.5f, 0.5f, 0.5f, 0.4f );
	defaultParticleSystem.Size = glm::vec2( 2.0f, 2.0f );
	defaultParticleSystem.TransparencyPolynome = glm::vec2( -1, 1);
	defaultParticleSystem.CullBoxSize = glm::vec2(5, 5);
	pts.MaxNumberOfBlocks = 192;
	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.IdentifyingName = "SmokeCloud";
	pts.Texture = m_Textures.at(1);
	pts.InitialShaderPath = "../../../shader/particle/SmokeCloudInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/SmokeCloud.glsl";
	m_ParticleTypes[PARTICLE_TYPE_SMOKE_CLOUD] = m_ParticleEngine.CreateParticleType( pts );

	// Smoke trail
	defaultParticleSystem.ParticlesSpeed		= 1.25f;
	defaultParticleSystem.TimeToLive			= 99999.9f;
	defaultParticleSystem.ParticlesTimeToLive	= 0.5f;
	defaultParticleSystem.Direction				= glm::vec3( 0.0f, -1.0f, 0.0f );
	defaultParticleSystem.Colour				= glm::vec4( 0.5f, 0.5f, 0.5f, 0.25f );
	defaultParticleSystem.Size					= glm::vec2( 0.15f, 0.15f );
	defaultParticleSystem.TransparencyPolynome	= glm::vec2( -1.0f, 1.0f );
	defaultParticleSystem.CullBoxSize 			= glm::vec2(0, 0);
	pts.MaxNumberOfBlocks = 512;
	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.IdentifyingName = "SmokeTrail";
	pts.Texture = m_Textures.at( 2 );
	pts.InitialShaderPath = "../../../shader/particle/SmokeTrailInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/SmokeTrail.glsl";
	m_ParticleTypes[PARTICLE_TYPE_SMOKE_TRAIL] = m_ParticleEngine.CreateParticleType( pts );

	// Fire ball
	defaultParticleSystem.ParticlesSpeed		= 8.0f;
	defaultParticleSystem.TimeToLive			= 0.4f;
	defaultParticleSystem.ParticlesTimeToLive	= 0.4f;
	defaultParticleSystem.Direction				= glm::vec3( 0.0f, 0.0f, 0.0f );
	defaultParticleSystem.Colour				= glm::vec4( 0.6f, 0.3f, 0.2f, 1.0f );
	defaultParticleSystem.Size					= glm::vec2( 2.0f, 2.0f );
	defaultParticleSystem.TransparencyPolynome	= glm::vec2( -0.5f, -0.5f );
	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.IdentifyingName = "FireBall";
	pts.Texture = m_Textures.at( 1 );
	pts.InitialShaderPath = "../../../shader/particle/FireBallInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/FireBall.glsl";
	m_ParticleTypes[PARTICLE_TYPE_FIRE_BALL] = m_ParticleEngine.CreateParticleType( pts );

	// Cone
	defaultParticleSystem.ParticlesSpeed		= 1.0f;
	defaultParticleSystem.TimeToLive			= 0.4f;
	defaultParticleSystem.ParticlesTimeToLive	= 0.8f;
	defaultParticleSystem.Direction				= glm::vec3( 0.0f, 0.0f, 0.0f );
	defaultParticleSystem.Colour				= glm::vec4( 0.3f, 0.9f, 0.6f, 0.8f );
	defaultParticleSystem.Size					= glm::vec2( 1.5f, 1.5f );
	defaultParticleSystem.TransparencyPolynome	= glm::vec2( -0.8f, 0.0f );
	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.IdentifyingName = "Cone";
	pts.Texture = m_Textures.at( 1 );
	pts.InitialShaderPath = "../../../shader/particle/ConeInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/Cone.glsl";
	m_ParticleTypes[PARTICLE_TYPE_CONE] = m_ParticleEngine.CreateParticleType( pts );

	// Laser
	defaultParticleSystem.ParticlesSpeed		= 0.0f;
	defaultParticleSystem.TimeToLive			= 0.4f;
	defaultParticleSystem.ParticlesTimeToLive	= 0.4f;
	defaultParticleSystem.Direction				= glm::vec3( 0.0f, 0.0f, 0.0f );
	defaultParticleSystem.Colour				= glm::vec4( 1.0f, 0.0f, 0.0f, 0.9f );
	defaultParticleSystem.Size					= glm::vec2( 0.2f, 0.2f );
	defaultParticleSystem.TransparencyPolynome	= glm::vec2( -1.0f, 0.0f );
	pts.BlockSize = 256;
	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.SourceFactorBlending = GL_SRC_ALPHA;
	pts.DestinationFactorBlending = GL_ONE_MINUS_SRC_ALPHA;
	pts.IdentifyingName = "Laser";
	pts.InitialShaderPath = "../../../shader/particle/LaserInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/Laser.glsl";
	pts.Texture = m_Textures.at( 0 );
	m_ParticleTypes[PARTICLE_TYPE_LASER] = m_ParticleEngine.CreateParticleType( pts );

	// Bullet
	defaultParticleSystem.ParticlesSpeed = 0.0f;
	defaultParticleSystem.TimeToLive = 0.0f;
	defaultParticleSystem.ParticlesTimeToLive = 0.0f;
	defaultParticleSystem.Direction = glm::vec3(0.0f, 0.0f, 0.0f);
	defaultParticleSystem.Colour = glm::vec4(1.0f, 0.3f, 0.0f, 1.0f);
	defaultParticleSystem.Size = glm::vec2(0.05f, 0.05f);
	defaultParticleSystem.TransparencyPolynome = glm::vec2( -1.0f, 0.0f );
	pts.BlockSize = 64;
	pts.DefaultParticleSystem = defaultParticleSystem;
	pts.SourceFactorBlending = GL_SRC_ALPHA;
	pts.DestinationFactorBlending = GL_ONE_MINUS_SRC_ALPHA;
	pts.IdentifyingName = "Bullet";
	pts.InitialShaderPath = "../../../shader/particle/BulletInitial.glsl";
	pts.UpdateShaderPath = "../../../shader/particle/Bullet.glsl";
	pts.Texture = m_Textures.at(0);
	m_ParticleTypes[PARTICLE_TYPE_BULLET] = m_ParticleEngine.CreateParticleType(pts);
}

void SSParticle::UpdateUserLayer( const float deltaTime )
{
	m_ParticleEngine.Update( deltaTime );
	// Cull particle systems
	for ( auto& particleType : m_ParticleTypes )
	{
		gfx::ParticleBucket::BucketUpdateInfo bucketUpdateInfo = m_ParticleEngine.GetParticleBucketUpdateInfo( particleType );
		for ( unsigned int i = bucketUpdateInfo.StartBlock; i < bucketUpdateInfo.EndBlock; ++i )
		{
			ParticleSystem* ps = &bucketUpdateInfo.Buffer->at( i );
			if ( ps->TimeToLive > 0.0f )
			{
				if ( ps->DoDraw != 2.0f )
				{
					ps->DoDraw = DoCullParticleSystem( *ps ) ? 0.0f : 1.0f;
				}
			}
		}
	}

	const CameraLens* lens = &g_SSCamera.GetActiveCamera()->GetLens();
	m_ParticleEngine.Render(
			g_SSCamera.GetActiveCamera()->GetViewProjection(),
			g_SSCamera.GetActiveCamera()->GetPosition(),
			gfx::g_GFXEngine.GetDepthBuffer(),
			lens->Near, lens->Far, (float) lens->WindowWidth, (float) lens->WindowHeight);
}

void SSParticle::Shutdown( )
{	
	g_CallbackPump.UnregisterForCallbackMessage( CALLBACK_MESSAGE_ENTITY_KILLED, m_CallbackMessageEntityKilledID );
	m_ParticleEngine.Shutdown();
	for ( auto& texture : m_Textures )
	{
		pDelete( texture );
	}
	m_Textures.clear();
}

gfx::ParticleSystem* SSParticle::SpawnParticleSystem( PARTICLE_TYPE particleType, gfx::ParticleSystem& particleSystem, bool fogOfWarCull )
{
	assert( particleType < PARTICLE_TYPE_SIZE );
	particleSystem.DoDraw = fogOfWarCull ? (DoCullParticleSystem( particleSystem ) ? 0.0f : 1.0f) : 2.0f;
	return m_ParticleEngine.Spawn( m_ParticleTypes[particleType], particleSystem );
}

const gfx::ParticleSystem& SSParticle::GetDefaultParticleSystem( PARTICLE_TYPE particleType ) const 
{
	assert( particleType < PARTICLE_TYPE_SIZE );
	return m_ParticleEngine.GetDefaultParticleSystem( m_ParticleTypes[particleType] );
}

void SSParticle::EntityKilled( CallbackMessage* message )
{
	DataCallbackMessage<Entity>* dataMessage = static_cast<DataCallbackMessage<Entity>*>(message);

	Entity entity = dataMessage->m_Data;
	EntityMask mask = g_EntityManager.GetEntityMask( entity );
	if (  	   mask & GetDenseComponentFlag<PlacementComponent>()
			&& mask & GetDenseComponentFlag<AgentComponent>()
			&& mask & GetDenseComponentFlag<ColourComponent>() )
	{
		PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>( entity );

		if ( g_SSFogOfWar.CalcVisibilityForPosition( placementComponent->Position ) )
		{
			ColourComponent* colourComponent = GetDenseComponent<ColourComponent>( entity );
			ParticleSystem particleSystem = GetDefaultParticleSystem( PARTICLE_TYPE_EXPLODING );
			particleSystem.EmitterPosition = placementComponent->Position + glm::vec3(0, PARTICLE_EXPLOSION_HEIGHT_OFFSET, 0);
			m_ParticleEngine.Spawn( m_ParticleTypes[PARTICLE_TYPE_EXPLODING], particleSystem );
		}
	}
}

bool SSParticle::DoCullParticleSystem( const gfx::ParticleSystem& ps )
{
	return !(g_SSFogOfWar.CalcVisibilityForPosition( glm::vec3( ps.EmitterPosition.x - ps.CullBoxSize.x, 0, ps.EmitterPosition.z - ps.CullBoxSize.y ) ) ||
			g_SSFogOfWar.CalcVisibilityForPosition( glm::vec3( ps.EmitterPosition.x + ps.CullBoxSize.x, 0, ps.EmitterPosition.z + ps.CullBoxSize.y ) ) ||
			g_SSFogOfWar.CalcVisibilityForPosition( glm::vec3( ps.EmitterPosition.x - ps.CullBoxSize.x, 0, ps.EmitterPosition.z + ps.CullBoxSize.y ) ) ||
			g_SSFogOfWar.CalcVisibilityForPosition( glm::vec3( ps.EmitterPosition.x - ps.CullBoxSize.x, 0, ps.EmitterPosition.z + ps.CullBoxSize.y ) ) ||
			g_SSFogOfWar.CalcVisibilityForPosition( ps.EmitterPosition ));
}

void SSParticle::DamageEffect(Entity target)
{
	gfx::ParticleSystem effect = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_SMOKE_TRAIL);
	effect.ParticlesSpeed = 5;
	effect.TimeToLive = 0.4f;
	effect.ParticlesTimeToLive = effect.TimeToLive;
	effect.EmitterPosition = GetDenseComponent<PlacementComponent>(target)->Position;
	effect.Direction = glm::normalize(glm::vec3(static_cast<float>(g_Randomizer.UserRand(0, 1000)), static_cast<float>(g_Randomizer.UserRand(500, 1000)), static_cast<float>(g_Randomizer.UserRand(0, 1000))));
	g_SSParticle.SpawnParticleSystem(PARTICLE_TYPE_SMOKE_TRAIL, effect);
}
