/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include "RenderJobManager.h"
namespace gfx
{
	/*
	This struct is used for sending in paramaters for the draw command.
	The shaderflag tell stuff like what shader features it wants.
	The ExtraData is optional but can be used with things like sending in extra paramaters.
	See BasicRenderProgram for use cases
	*/
	struct DrawData
	{
		int ShaderFlags;
		void* ExtraData;
	};
	/*
	this class is used to set up varying functionality in the graphics pipeline
	*/
	class RenderProgram
	{
	public:
		virtual void Init(RenderJobManager* jobManager) = 0; //Load shaders
		virtual void Draw(DrawData* data) = 0; //render whatever
		virtual void Shutdown() = 0; //clear data
	};
}