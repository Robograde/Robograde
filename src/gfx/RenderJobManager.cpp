/**************************************************
Zlib Copyright 2015 Henrik Johansson & Mattias Wilelmsen
***************************************************/

#include "RenderJobManager.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include "Shader/ShaderBank.h"
#include "BufferManager.h"
#include<utility/GameTimer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <profiler/AutoProfiler.h>


gfx::RenderJobManager::RenderJobManager()
{
	m_RenderBuffer = "ShaderInputs";
	m_CommandBuffer = 0;
	m_InstanceCountBuffer = "InstanceInput";
	m_SceneBuffer = "SceneInputs";
	m_SkeletonBuffer = "SkeletonInput"; 
	m_BindposeBuffer = "BindposeInput";
	m_InstanceCounter = 0;
	m_NumJobs = 0;
	m_NumSkeletons = 0;
	m_NumShaderInputs = 0;
	m_NumJointInputs = 0;
	m_Jobs = nullptr;
}

gfx::RenderJobManager::~RenderJobManager()
{
	pDeleteArray( m_Jobs );
	pDeleteArray( m_ShaderInputList );
	pDeleteArray( m_SkeletonInputList );
	pDeleteArray( m_BindposeInputList );
}

void gfx::RenderJobManager::Initialize()
{
	m_LineBuffer.reserve(50);
	m_Jobs = pNewArray( RenderJob, MAX_RENDERJOBS );
	m_InstanceCountList.reserve( MAX_RENDERJOBS );
	m_ShaderInputList = pNewArray( ShaderInput, MAX_RENDERJOBS );
	m_SkeletonInputList = pNewArray( glm::mat4, MAX_ANIM_JOBS  * MAX_SKELETON_JOINTS );
	m_BindposeInputList = pNewArray(glm::mat4, 1000 * MAX_SKELETON_JOINTS);
	m_Cmds.reserve( MAX_RENDERJOBS );
	g_BufferManager.CreateBuffer( m_RenderBuffer, GL_SHADER_STORAGE_BUFFER, SHADERINPUT_SIZE, GL_DYNAMIC_DRAW );

	g_BufferManager.CreateBuffer( m_InstanceCountBuffer, GL_SHADER_STORAGE_BUFFER, INSTANCECOUNT_SIZE, GL_DYNAMIC_DRAW );

	g_BufferManager.CreateBuffer( m_SceneBuffer, GL_UNIFORM_BUFFER, sizeof( SceneInput ), GL_DYNAMIC_DRAW );

	g_BufferManager.CreateBuffer( m_SkeletonBuffer, GL_SHADER_STORAGE_BUFFER, SKELETONINPUT_SIZE, GL_DYNAMIC_DRAW );
	
	g_BufferManager.CreateBuffer( m_BindposeBuffer, GL_SHADER_STORAGE_BUFFER, sizeof( glm::mat4 ) * 1000 * MAX_SKELETON_JOINTS, GL_DYNAMIC_DRAW );

	glGenBuffers( 1, &m_CommandBuffer );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_CommandBuffer );
	glBufferData( GL_DRAW_INDIRECT_BUFFER, sizeof( IndirectDrawCall ) * MAX_RENDERJOBS, nullptr, GL_DYNAMIC_DRAW );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );

	m_SkeletonOffsets.push_back(0);
}

void gfx::RenderJobManager::BuildBuffers()
{
	
	bool diff = false;
	if(m_NumJobs == 0)
		return;
	//m_DebugRenderer.SetViewProj( m_View, m_Proj );
	m_SkeletonOffsets.clear();
	m_SkeletonOffsets.push_back(0);
	PROFILE(AutoProfiler pRenderJobSort( "RenderJobSort" , Profiler::PROFILER_CATEGORY_STANDARD,true, true));
	//Sort the renderjobs based on which shader it uses
	rVector<RenderJob*> jobList;
	for(int i = 0; i < m_NumJobs; i++)
	{
		jobList.push_back(&m_Jobs[i]);
	}

	SortRenderJobs(jobList);
	PROFILE( pRenderJobSort.Stop() );
	//loop through every job
	DrawCallData dcd;
	dcd.count = 0;
	dcd.shader = jobList[0]->ShaderFlags;
	int animatedJobs = 0;
	
	for ( int i = 0; i < m_NumJobs; ++i )
	{
		if ( dcd.shader != jobList[i]->ShaderFlags )
		{
			m_DrawCallData.push_back( dcd );
			dcd.count = 0;
			dcd.shader = jobList[i]->ShaderFlags;
		}
		dcd.count++;
		//set the shader inputs
		memcpy( &m_ShaderInputList[m_NumShaderInputs], jobList[i]->shaderInputs.data(), jobList[i]->shaderInputs.size() * sizeof( ShaderInput ) );
		m_NumShaderInputs += static_cast<int>(jobList[i]->shaderInputs.size());
		if ( jobList[i]->bindPose.size() > 0 )
		{
			int numModels = (int)jobList[i]->shaderInputs.size();
			int jointsPerModel = (int)jobList[i]->jointList.size() / numModels;
			assert(jointsPerModel <= MAX_SKELETON_JOINTS);
			for (int j = 0; j < numModels; j++)
			{
				for (int k = 0; k < jointsPerModel; k++)
				{
					int indexSkel = m_NumJointInputs + k;
					int indexJoints = j * jointsPerModel + k;
					int maxSkel = MAX_ANIM_JOBS * MAX_SKELETON_JOINTS;
					int maxJoints = (int)jobList[i]->jointList.size();

					m_SkeletonInputList[m_NumJointInputs + k] = jobList[i]->jointList[j * jointsPerModel + k];
				}
				m_NumJointInputs += MAX_SKELETON_JOINTS;
			}	
			memcpy( &m_BindposeInputList[animatedJobs * MAX_SKELETON_JOINTS], jobList[i]->bindPose.data(), jobList[i]->bindPose.size() * sizeof( glm::mat4 ) );
			animatedJobs++;
			m_SkeletonOffsets.push_back(m_NumJointInputs);
		}	
		else if (jobList[i]->ShaderFlags & gfx::ShaderBitFlags::VERTEX_ANIMATED)
		{
			int numModels = (int)jobList[i]->shaderInputs.size();
			m_BindposeInputList[animatedJobs * MAX_SKELETON_JOINTS] = glm::mat4(1);
			for (int i = 0; i < MAX_SKELETON_JOINTS; i++)
			{
				m_SkeletonInputList[m_NumJointInputs] = glm::mat4(1);
				m_NumJointInputs++;
			}
			animatedJobs++;
		}

		//Indirect Buffer
		m_Cmds.push_back( jobList[i]->Command );
		//set the instance counter
		m_InstanceCountList.push_back( m_InstanceCounter );
		m_InstanceCounter += jobList[i]->Command.InstanceCount - 1;
		jobList[i]->shaderInputs.clear();
		jobList[i]->jointList.clear();
		jobList[i]->bindPose.clear();
	}
	
	m_DrawCallData.push_back( dcd );
	g_BufferManager.UpdateBuffer( m_RenderBuffer, 0, m_ShaderInputList, sizeof( ShaderInput ) * m_NumShaderInputs );
	g_BufferManager.UpdateBuffer( m_SkeletonBuffer, 0, m_SkeletonInputList, sizeof( glm::mat4 ) * m_NumJointInputs );
	g_BufferManager.UpdateBuffer( m_InstanceCountBuffer, 0, m_InstanceCountList.data(), static_cast<int>(sizeof( unsigned int ) * m_InstanceCountList.size()) );
	g_BufferManager.UpdateBuffer( m_BindposeBuffer, 0, m_BindposeInputList, sizeof( glm::mat4 ) * MAX_SKELETON_JOINTS * animatedJobs );
	
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_CommandBuffer );
	glBufferSubData( GL_DRAW_INDIRECT_BUFFER, 0, m_Cmds.size() * sizeof( IndirectDrawCall ), m_Cmds.data() );

	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );

	m_NumJobs = 0;
	m_NumShaderInputs = 0;
	m_NumJointInputs = 0;
	m_NumSkeletons = 0;
	m_InstanceCountList.clear();
	m_Cmds.clear();
}

void gfx::RenderJobManager::BindBuffers( ShaderProgram* program )
{
	program->Apply();
	g_BufferManager.BindBufferToProgram( m_SceneBuffer, program, 0 );

	g_BufferManager.BindBufferWithRange( m_RenderBuffer, program, 1, SHADERINPUT_SIZE );

	g_BufferManager.BindBufferWithRange( m_InstanceCountBuffer, program, 2, INSTANCECOUNT_SIZE );

	g_BufferManager.BindBufferToProgram( "MaterialBuffer", program, 4 );

	g_BufferManager.BindBufferToProgram( "TextureBuffer", program, 6 );

	g_BufferManager.BindBufferWithRange( m_SkeletonBuffer, program, 10, SKELETONINPUT_SIZE );

	g_BufferManager.BindBufferWithRange( m_BindposeBuffer, program, 14, sizeof(glm::mat4) * 1000);

}

void gfx::RenderJobManager::Render( ModelHandle modelhandle, const glm::mat4& world, const glm::vec4& color )
{
	Model model = g_ModelBank.FetchModel( modelhandle );
	for ( auto& meshit : model.Meshes )
	{
		m_Jobs[m_NumJobs].Command = IndirectDrawCall( meshit.Indices, 1, meshit.IndexBufferOffset, 0, 0 );
		int matid = g_MaterialBank.GetMaterialID( meshit.Material );
		if ( matid != -1 )
			m_Jobs[m_NumJobs].ShaderFlags = g_MaterialBank.GetMaterial( matid )->GetBitFlags();
		else
			m_Jobs[m_NumJobs].ShaderFlags = gfx::ShaderBitFlags::NONE;

		m_Input.MatIDPadd3.x = matid;
		m_Input.Color = color;
		m_Input.World = world;
		m_Jobs[m_NumJobs].shaderInputs.push_back( m_Input );
		m_NumJobs++;
	}
}

void gfx::RenderJobManager::RenderInstanced( ModelHandle modelhandle, const rVector<glm::mat4>& worlds, const rVector<glm::vec4>& colors )
{
	const Model* model = &(g_ModelBank.FetchModel( modelhandle ));
	for ( auto& meshit : model->Meshes )
	{
		m_Jobs[m_NumJobs].Command = IndirectDrawCall( meshit.Indices, static_cast<int>(worlds.size()), meshit.IndexBufferOffset, 0, 0 );
		int matid = g_MaterialBank.GetMaterialID( meshit.Material );

		if ( matid != -1 )
			m_Jobs[m_NumJobs].ShaderFlags = g_MaterialBank.GetMaterial( matid )->GetBitFlags();
		else
			m_Jobs[m_NumJobs].ShaderFlags = gfx::ShaderBitFlags::NONE;

		if ( model->Type == POS_NORMAL_TEX_TANGENT_JOINTS_WEIGHTS )
		{
			m_Jobs[m_NumJobs].ShaderFlags = m_Jobs[m_NumJobs].ShaderFlags | gfx::ShaderBitFlags::VERTEX_ANIMATED;
		}

		m_Input.MatIDPadd3.x = matid;
		for ( int i = 0; i < static_cast<int>(worlds.size()); ++i )
		{
			m_Input.World = worlds[i];
			m_Input.Color = colors[i];
			m_Jobs[m_NumJobs].shaderInputs.push_back( m_Input );
		}
		m_NumJobs++;
	}
}

void gfx::RenderJobManager::RenderInstancedAnimated( ModelHandle modelhandle, const rVector<glm::mat4>& worlds, const rVector<glm::vec4>& colors, const rVector<glm::mat4>& skeletons, const rVector<glm::mat4>& bindPose )
{
	const Model* model = &(g_ModelBank.FetchModel( modelhandle ));
	
	for ( auto& meshit : model->Meshes )
	{
		m_Jobs[m_NumJobs].Command = IndirectDrawCall( meshit.Indices, static_cast<int>(worlds.size()), meshit.IndexBufferOffset, 0, 0 );
		int matid = g_MaterialBank.GetMaterialID( meshit.Material );

		if ( matid != -1 )
			m_Jobs[m_NumJobs].ShaderFlags = g_MaterialBank.GetMaterial( matid )->GetBitFlags();
		else
			m_Jobs[m_NumJobs].ShaderFlags = gfx::ShaderBitFlags::NONE;

		// Add an animation flag
		m_Jobs[m_NumJobs].ShaderFlags = m_Jobs[m_NumJobs].ShaderFlags | gfx::ShaderBitFlags::VERTEX_ANIMATED;

		if (skeletons.size() < 1)
		{
			int breakHere = 5;
		}

		m_Input.MatIDPadd3.x = matid;
		for ( int i = 0; i < static_cast<int>(worlds.size()); ++i )
		{
			m_Input.World = worlds[i];
			m_Input.Color = colors[i];
			m_Jobs[m_NumJobs].shaderInputs.push_back(m_Input);
			
		}
		m_Jobs[m_NumJobs].jointList = skeletons;
		m_Jobs[m_NumJobs].bindPose = bindPose;
		m_NumJobs++;
	}
}

GLuint gfx::RenderJobManager::GetCommandBuffer()
{
	return m_CommandBuffer;
}

rVector<gfx::DrawCallData>& gfx::RenderJobManager::GetDrawCallData()
{
	return m_DrawCallData;
}

void gfx::RenderJobManager::SortRenderJobs(rVector<RenderJob*>& renderJobLists)
{
	std::sort( renderJobLists.begin(), renderJobLists.end(),
		[]( const RenderJob* job1, const RenderJob* job2 ){
		return static_cast<int>(job1->ShaderFlags) < static_cast<int>(job2->ShaderFlags); }
	);

	//std::sort( m_Jobs, m_Jobs + m_NumJobs, gfx::RenderJobManager::CmpRenderJob);
	//int i = 0;
}

void gfx::RenderJobManager::SetSceneInputs(const glm::mat4& view, const glm::mat4& proj, const glm::vec4& campos)
{
	m_Proj = proj;
	m_View = view;
	m_Campos = campos;
}

rVector<int> gfx::RenderJobManager::GetSkeletonOffsets() const
{
	return m_SkeletonOffsets;
}

void gfx::RenderJobManager::UpdateSceneBuffer()
{
	SceneInput si;
	si.viewProj = m_Proj * m_View;
	si.campos = m_Campos;
	g_BufferManager.UpdateBuffer( m_SceneBuffer, 0, &si, static_cast<int>(sizeof( SceneInput )));
}

void gfx::RenderJobManager::RenderLine(const glm::vec3& origin, const glm::vec3& destination, const glm::vec4& color, bool dashed, float animationProgres)
{
	Line l;
	l.Origin = origin;
	l.Destination = destination;
	l.Color = color;
	l.Dashed = dashed;
	l.AnimationProgres = animationProgres;
	m_LineBuffer.push_back(l);
}

rVector<gfx::Line>& gfx::RenderJobManager::GetLines()
{
	return m_LineBuffer;
}

void gfx::RenderJobManager::ClearLists()
{
	m_DrawCallData.clear();
	m_InstanceCounter = 0;
	m_NumJobs = 0;
	m_LineBuffer.clear();
}

bool gfx::RenderJobManager::CmpRenderJob(const RenderJob& job1, const RenderJob& job2)
{
	return int(job1.ShaderFlags) < int(job2.ShaderFlags);
}