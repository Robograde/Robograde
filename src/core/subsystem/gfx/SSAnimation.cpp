/**************************************************
Copyright 2015 Mattias Wilelmsen
***************************************************/

#include "SSAnimation.h"
#include <gfx/ObjectImporter.h>
#include <utility/Logger.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/AnimationComponent.h"
#include "../../component/PlacementComponent.h"
#include "core/datadriven/ComponentHelper.h"
#include <gfx/AnimationTypes.h>
#include <profiler/AutoProfiler.h>
SSAnimation& SSAnimation::GetInstance()
{
	static SSAnimation instance;
	return instance;
}

SSAnimation::SSAnimation()
	: Subsystem("Animation") { }

void SSAnimation::Startup()
{
	Subsystem::Startup();
}

void SSAnimation::UpdateSimLayer( const float timeStep )
{
	Subsystem::UpdateSimLayer( timeStep );
	EntityMask animationFlag = DenseComponentCollection<AnimationComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		if ( (entityMask & animationFlag) )
		{
			AnimationComponent* animation = GetDenseComponent<AnimationComponent>( entityID );
			if ( animation->Playing )
			{
				PROFILE(AutoProfiler pUpdateAnimation( "updating animations" , Profiler::PROFILER_CATEGORY_STANDARD,true, true));
				UpdateAnimation( animation, timeStep );
				PROFILE( pUpdateAnimation.Stop() );
			}
		}
		entityID++;
	}
}

void SSAnimation::UpdateUserLayer( const float deltaTime )
{
	Subsystem::UpdateUserLayer( deltaTime );
}

void SSAnimation::UpdateAnimation( AnimationComponent* animationComponent, const float deltaTime )
{
	// If no animation is set, stop the animation and return
	if ( animationComponent->CurrentAnimation == gfx::AnimationType::Undefined )
	{
		animationComponent->Stop();
		return;
	}

	animationComponent->CurrentTime += deltaTime;

	if(animationComponent->HandleMap.find(animationComponent->CurrentAnimation) != animationComponent->HandleMap.end())
	{
		gfx::Animation* animation = &(gfx::g_AnimationBank.FetchAnimationWithHandle( animationComponent->HandleMap[animationComponent->CurrentAnimation] ));
		animation->GetMatricesAtTime( animationComponent->MatrixList, animationComponent->CurrentTime, animationComponent->FramesPerSec );
		animationComponent->AnimationRef = animation;

		float animationTime = animation->GetEditableSkeletonList().back().FrameNumber / animationComponent->FramesPerSec;
		if(animationComponent->CurrentTime >= animationTime)
			animationComponent->CurrentTime -= animationTime;
	}
	//for ( auto& animationMap : animationComponent->HandleMap )
	//{
	//	if ( animationMap.first == animationComponent->CurrentAnimation ) // Find the right animation type
	//	{
	//		gfx::Animation* animation = &(gfx::g_AnimationBank.FetchAnimationWithHandle( animationMap.second ));
	//		animation->GetMatricesAtTime( animationComponent->MatrixList, animationComponent->CurrentTime, animationComponent->FramesPerSec );
	//		animationComponent->AnimationRef = animation;
	//		break;
	//	}
	//}
}

void SSAnimation::DebugDrawJoints( gfx::DebugRenderer& debugRenderer )
{
	int totalAnimationsDrawn = 0;
	EntityMask animationFlag = DenseComponentCollection<AnimationComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		if ( (entityMask & animationFlag) )
		{
			AnimationComponent* animation = GetDenseComponent<AnimationComponent>( entityID );
			if ( animation->Playing )
			{
				totalAnimationsDrawn++;
				PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>( entityID );

				for ( auto& matrix : animation->MatrixList )
				{
					glm::mat4 tmp = placementComponent->World * matrix;
				    debugRenderer.RenderSphere( tmp, glm::vec3( 0.5f ) );
					
				}
			}
		}
		entityID++;
	}
}