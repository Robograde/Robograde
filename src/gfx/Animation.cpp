/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#include "Animation.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> 

gfx::Animation::Animation(){}
gfx::Animation::~Animation(){}

rVector<gfx::SkeletonFrame>& gfx::Animation::GetEditableSkeletonList()
{
	return m_SkeletonFrames;
}

const rVector<glm::mat4>& gfx::Animation::GetBindPoseRef() const
{
	return m_BindPoseMatrices;
}

void gfx::Animation::CreateBindPoseMatrices()
{
	GetMatricesAtTime( m_BindPoseMatrices, 0 );
}

rString gfx::Animation::GetName() const
{
	return m_Name;
}

void gfx::Animation::SetName(rString name)
{
	m_Name = name;
}

void gfx::Animation::SetDefaultFPS( const float fps )
{
	m_FramesPerSec = fps;
}

void gfx::Animation::GetMatricesAtTime( rVector<glm::mat4>& matVec, float time, float framesPerSec)
{
	if ( framesPerSec == 0.0f )
	{
		framesPerSec = std::max( m_FramesPerSec, 0.00001f );
	}
	float secPerFrame = 1.0f / framesPerSec;

	int numKeyframes = (int)m_SkeletonFrames.size();
	int numFrames = m_SkeletonFrames.back().FrameNumber;

	float currFrame = time / secPerFrame; 
	int lowFrame = (int)currFrame;
	int highFrame = lowFrame + 1;

	while ( lowFrame >= numFrames )
	{
		time -= secPerFrame * numFrames;
		currFrame = time / secPerFrame;
		lowFrame = (int)currFrame;
		highFrame = lowFrame + 1;
	}

	if ( highFrame >= numFrames )
		highFrame -= numFrames;

	matVec.clear();

	int lowKeyframe, highKeyframe; // TODOMW: Initialize these values.
	for ( int i = 0; i < numKeyframes; i++ )
	{
		if ( lowFrame < m_SkeletonFrames[i].FrameNumber )
		{
			lowKeyframe = i - 1;
			break;
		}
	}

	highKeyframe = lowKeyframe + 1;
	if ( highKeyframe >= (int)m_SkeletonFrames.size() )
	{
		highKeyframe = 0;
	}
	
	float weight = currFrame - m_SkeletonFrames[lowKeyframe].FrameNumber;
	float normalizedWeight = weight / (m_SkeletonFrames[highKeyframe].FrameNumber - m_SkeletonFrames[lowKeyframe].FrameNumber);

	int size = (int)m_SkeletonFrames[lowKeyframe].Skeleton.size();

	glm::quat lowRotQuat;
	glm::quat highRotQuat;
	glm::quat rotQuat;
	glm::vec3 position;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 world;

	for ( int i = 0; i < size; i++ )
	{
		const gfx::Joint& lowJoint = m_SkeletonFrames[lowKeyframe].Skeleton[i];
		const gfx::Joint& highJoint = m_SkeletonFrames[highKeyframe].Skeleton[i];
		lowRotQuat = glm::quat( lowJoint.Rotation.w, lowJoint.Rotation.x, lowJoint.Rotation.y, lowJoint.Rotation.z );
		highRotQuat = glm::quat( highJoint.Rotation.w, highJoint.Rotation.x, highJoint.Rotation.y, highJoint.Rotation.z );
		rotQuat = glm::slerp( lowRotQuat, highRotQuat, normalizedWeight );

		position = lowJoint.Position * (1.0f - normalizedWeight) + highJoint.Position * normalizedWeight;

		// Create world matrix
		translation = glm::translate(
			glm::mat4( 1.0f ),
			position );

		rotation = glm::mat4_cast( rotQuat );
		world = translation * rotation;
		matVec.push_back( world );
	}
}

void gfx::Animation::SetBindpose( SkeletonFrame& frame )
{
	m_BindPoseMatrices.clear();
	for ( gfx::Joint& joint : frame.Skeleton )
	{
		glm::quat rotation = glm::quat( joint.Rotation.w, joint.Rotation.x, joint.Rotation.y, joint.Rotation.z );
		glm::mat4 translationMatrix = glm::translate( glm::mat4( 1.0f ), joint.Position );
		glm::mat4 rotationMatrix = glm::mat4_cast( rotation );
		glm::mat4 world = translationMatrix * rotationMatrix;
		m_BindPoseMatrices.push_back( world );
	}
}
