/**************************************************
Zlib Copyright 2015 Henrik Johansson & Mattias Wilelmsen
***************************************************/

#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include "RenderJob.h"
#include "ModelBank.h"
#include "Shader/ShaderBank.h"
#include <gfx/DebugRenderer.h>
#include "LineRenderProgram.h"

#ifdef _WIN32
#ifdef GFX_DLL_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif

namespace gfx
{
    #define MAX_RENDERJOBS 10000
	#define MAX_ANIM_JOBS 10000
	#define INSTANCECOUNT_SIZE sizeof(unsigned int) * MAX_RENDERJOBS
    #define SHADERINPUT_SIZE sizeof(ShaderInput) * MAX_RENDERJOBS
	#define SKELETONINPUT_SIZE sizeof(glm::mat4) * gfx::MAX_SKELETON_JOINTS * MAX_ANIM_JOBS

    struct  DrawCallData
    {
        int count;
        gfx::ShaderBitFlags shader;
    };
	struct SceneInput
	{
		glm::mat4 viewProj;
		glm::vec4 campos;
	};
	class  RenderJobManager
    {
    public:
		// TMP
		glm::mat4 VIEW_MATRIX;
		glm::mat4 PROJ_MATRIX;

        GFX_API RenderJobManager();
        GFX_API ~RenderJobManager();
        GFX_API void Initialize();
		GFX_API void Render( ModelHandle model, const glm::mat4& world, const glm::vec4& color );
		//GFX_API void RenderAnimated( ModelHandle modelhandle, const glm::mat4& world, const glm::vec4& color, const rVector<glm::mat4> joints );
		GFX_API void RenderInstanced( ModelHandle modelhandle, const rVector<glm::mat4>& worlds, const rVector<glm::vec4>& colors );
		GFX_API void RenderInstancedAnimated( ModelHandle modelhandle, const rVector<glm::mat4>& worlds, const rVector<glm::vec4>& colors, const rVector<glm::mat4>& skeletons, const rVector<glm::mat4>& bindPose );
        GFX_API void BuildBuffers();
        GFX_API void BindBuffers(ShaderProgram* program);
        GFX_API GLuint GetCommandBuffer();
        GFX_API rVector<gfx::DrawCallData>& GetDrawCallData();
		GFX_API void ClearLists();
		GFX_API void SetSceneInputs(const glm::mat4& view, const glm::mat4& proj, const glm::vec4& campos );
		GFX_API rVector<int> GetSkeletonOffsets() const;
		void UpdateSceneBuffer();
		glm::mat4& GetView(){ return m_View; };
		glm::mat4& GetProj(){ return m_Proj; };
		glm::vec4& GetCamPos() { return m_Campos; };

		GFX_API void RenderLine(const glm::vec3& origin, const glm::vec3& destination, const glm::vec4& color, bool dashed, float animationProgres);
		rVector<Line>& GetLines();
    private:
        rString m_RenderBuffer;
		rString m_InstanceCountBuffer;
		rString m_SceneBuffer;
		rString m_SkeletonBuffer;
		rString m_BindposeBuffer;
        GLuint m_CommandBuffer;

		RenderJob*						m_Jobs;
		int								m_NumJobs;
		int								m_NumSkeletons;
        rVector<gfx::DrawCallData>		m_DrawCallData;
		rVector<unsigned int>			m_InstanceCountList;
		ShaderInput*					m_ShaderInputList;
		int								m_NumShaderInputs;
		glm::mat4*						m_SkeletonInputList;
		glm::mat4*						m_BindposeInputList;
		rVector<int>					m_SkeletonOffsets;
		int								m_NumJointInputs;
		rVector<IndirectDrawCall>		m_Cmds;
		unsigned int					m_InstanceCounter;
		ShaderInput						m_Input;
		glm::mat4						m_View;
		glm::mat4						m_Proj;
		glm::vec4						m_Campos;
		rVector<Line>					m_LineBuffer;

        void SortRenderJobs(rVector<RenderJob*>& renderJobLists);
		static bool CmpRenderJob(const RenderJob& job1, const RenderJob& job2);
    };
}
