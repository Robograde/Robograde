/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "RenderProgram.h"
#include "Shader/ShaderBank.h"
#include "RenderJobManager.h"
#include "GBuffer.h"
#include <glm/glm.hpp>
namespace gfx
{
	class DefferedDecalProgram : public RenderProgram
	{
	public:
		DefferedDecalProgram();
		~DefferedDecalProgram();

		virtual void Init(RenderJobManager* jobManager);
		virtual void Draw(DrawData* data);
		virtual void Shutdown();
		
		struct DecalData
		{
			glm::vec2 ScreenSize;
			GBuffer* gbuffer;
		};
	private:
		ShaderProgramHandle m_ProgramHandle;
		VertexBuffer m_VertexBuffer;
		GLuint m_IndexBuffer;
		RenderJobManager* m_JobManager;
	};
}