/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "RenderProgram.h"
#include "Shader/ShaderBank.h"
#include "Texture.h"
#include "RenderJobManager.h"
namespace gfx
{
	class BasicRenderProgram : public RenderProgram
	{
	public:
		BasicRenderProgram();
		~BasicRenderProgram();

		virtual void Init(RenderJobManager* jobManager);
		virtual void Draw(DrawData* data);
		virtual void Shutdown();
		//struct for all the data that is needed for the shader
		struct BasicData
		{
			unsigned int	BatchCount;
			unsigned int	BatchOffset; //number of draw calls before this one
			glm::vec2		WorldSize;
			Texture*		SkyTex;
			Texture*		IrradianceTex;
			GLuint			FogTex;
			int				PointLightCount;
			int				DirLightCount;
			glm::mat4		ShadowMat;
			GLuint			ShadowTex;
		};
	private:
		ShaderProgramHandle m_ProgramHandle;
		bool m_HasDrawID;
		RenderJobManager* m_RenderJobManager;
		
	};
}