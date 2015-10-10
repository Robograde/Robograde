/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "RenderProgram.h"
#include "Shader/ShaderBank.h"
#include "Texture.h"
#include "RenderJobManager.h"
#include "GBuffer.h"
#include <glm/glm.hpp>
namespace gfx
{
	class DeferredLightProgram : public RenderProgram
	{
	public:
		DeferredLightProgram();
		~DeferredLightProgram();
		virtual void Init(RenderJobManager* jobManager);
		virtual void Init(RenderJobManager* jobManager, bool useFogOfWar);
		virtual void Draw(DrawData* data);
		virtual void Shutdown();
		//struct for all the data that is needed for the shader
		struct InputParameters
		{
			GBuffer*		gbuffers;
			GLuint			FogOfWarTex;
			glm::vec2		WorldSize;
			glm::vec2		ScreenSize;
			glm::mat4		ShadowMat;
			GLuint			ShadowMap;
			Texture*		SkyTex;
			Texture*		IrradianceTex;
		};
	private:
		ShaderProgramHandle m_ProgramHandle;
		RenderJobManager*	m_RenderJobManager;
		GLuint				m_WorkGroupSizeX;
		GLuint				m_WorkGroupSizeY;
		const GLuint		m_WORK_GROUP_SIZE = 16;
	};
}