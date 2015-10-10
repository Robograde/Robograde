/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "RenderProgram.h"
#include "Shader/ShaderBank.h"
#include "Texture.h"
#include "RenderJobManager.h"
namespace gfx
{
	class DeferredGeometryRenderProgram : public RenderProgram
	{
	public:
		DeferredGeometryRenderProgram();
		~DeferredGeometryRenderProgram();

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
		};
	private:
		ShaderProgramHandle m_ProgramHandle;
		bool m_HasDrawID;
		RenderJobManager* m_RenderJobManager;
		
	};
}