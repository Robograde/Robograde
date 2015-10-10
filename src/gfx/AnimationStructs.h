/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include "AnimationTypes.h"

namespace gfx
{
	typedef int AnimationHandle;
	// typedef rVector<Animation> AnimationCollection;
	const int MAX_SKELETON_JOINTS = 27;

	struct Joint
	{
		Joint(){}
		Joint( int parentId, glm::vec3 position, glm::vec4 rotation )
		{
			ParentId = parentId;
			Position = position;
			Rotation = rotation;
		}

		int ParentId;
		glm::vec3 Position;
		glm::vec4 Rotation;
	};

	struct SkeletonFrame
	{
		SkeletonFrame(){}
		SkeletonFrame( rVector<Joint> skeleton, int frameNumber )
		{
			Skeleton = skeleton;
			FrameNumber = frameNumber;
		}

		int				FrameNumber;
		rVector<Joint>	Skeleton;
	};

	struct AnimationInstance
	{
		AnimationHandle Handle;
		float AnimationTime;
		bool Playing;

	};

	// typedef rVector<rVector<Joint>> SkeletonList;
}