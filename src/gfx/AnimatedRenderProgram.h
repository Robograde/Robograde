/**************************************************
Zlib Copyright 2015 Mattias Wilelmsen
***************************************************/

#include "RenderProgram.h"
#include "Shader/ShaderBank.h"
#include "Texture.h"
#include "RenderJobManager.h"
namespace gfx
{
	class AnimatedRenderProgram : public RenderProgram
	{
	public:
		AnimatedRenderProgram();
		~AnimatedRenderProgram();
		static void ResetAnimationCounter();
		virtual void Init(RenderJobManager* jobManager);
		virtual void Draw(DrawData* data);
		virtual void Shutdown();
		//struct for all the data that is needed for the shader
		struct InputParameters
		{
			unsigned int	BatchCount;
			unsigned int	BatchOffset; //number of draw calls before this one
			GLuint			ShadowMap;
			glm::mat4*		LightMat;
			Texture*		SkyTex;
			Texture*		IrradianceTex;
			int				SkeletonOffset;
		};
	private:
		ShaderProgramHandle m_ProgramHandle;
		bool m_HasDrawID;
		RenderJobManager* m_RenderJobManager;
		static unsigned int ANIMATION_COUNTER;
		
	};
}