/**************************************************
2015 David Pejtersen
***************************************************/
#pragma once
#include "../Subsystem.h"
#include <gfx/DebugRenderer.h>
#include <gui/GUIEngine.h>
#include "../gamelogic/SSCamera.h"
#include "../utility/SSMail.h"

#define g_SSEditorCameraPaths SSEditorCameraPaths::GetInstance( )

class SSEditorCameraPaths : public Subsystem, public Subscriber
{
private:
	class MultiSlider; // Declaration at the end of file

public:
	static SSEditorCameraPaths& GetInstance( ) { static SSEditorCameraPaths instance; return instance; }

	void Startup( ) override;
	void Shutdown( ) override;
	void UpdateUserLayer( const float deltaTime ) override;
	void UpdateSimLayer( const float timeStep ) override;

private:
	/* no type */			SSEditorCameraPaths( ) : Subsystem( "EditorCameraPaths" ), Subscriber( "EditorCameraPaths" ) { }
	/* no type */			SSEditorCameraPaths( const SSEditorCameraPaths& );
	/* no type */			~SSEditorCameraPaths( ) { }
	SSEditorCameraPaths&	operator=( const SSEditorCameraPaths& );

	void					UpdateSplineList( );
	void					UpdateNodeList( );
	void					UpdateTimeline( );

	void					OnEventAddSpline( );
	void					OnEventAddNode( );
	void					OnEventRemoveSpline( );
	void					OnEventRemoveNode( );

	const char* const		m_WindowName = "CameraPathsWindow";

	GUI::ComboBox*			m_SplineList;
	GUI::ComboBox*			m_SplineNodes;

	GUI::Button*			m_AddSpline;
	GUI::Button*			m_RemoveSpline;
	GUI::Button*			m_AddNode;
	GUI::Button*			m_RemoveNode;
	
	GUI::Button*			m_PlayButton;
	GUI::Button*			m_StopButton;
	GUI::Button*			m_UseRTSCamera;
	GUI::Button*			m_UseFPSCamera;
	GUI::Button*			m_GoToNode;

	GUI::TextBox*			m_TimeBox;
	GUI::TextBox*			m_NameBox;

	rMap<size_t, SplineID>	m_ComboIndexToSplineID;
	rMap<size_t, int>		m_NodeIDToTimelineID;
	SplineID				m_CurrentSpline = 0;
	SplineID				m_PrevSpline	= 0;
	int						m_CurrentNode	= 0;

	MultiSlider*			m_Timeline;
	int						m_TimelinePlayer = -2;
};

class SSEditorCameraPaths::MultiSlider
{
public:
	MultiSlider( ) { }
	~MultiSlider( ) { }

	MultiSlider( glm::ivec2 position, glm::ivec2 size, const char* window );

	int		AddEntry( float value );
	void	ClearAll( );

	void	UpdatePositions( glm::ivec2 parentPos );
	void	RefreshPositions( );

	void	SetMin( float value ) { m_MinValue = value; }
	void	SetMax( float value ) { m_MaxValue = value; }

	float	GetMin( ) const { return m_MinValue; }
	float	GetMax( ) const { return m_MaxValue; }
	
	float	GetValue( int node ) const;
	int		GetMoved( ) const { return m_LastMovedID; }

	void	SetLastMoved( int node ) { m_LastMovedID = node; }
	void	SetColour( int node, glm::vec4 colour );
	void	SetValue( int node, float value );

private:
	struct SliderObject
	{
		int				ID;
		bool			Active;
		float			Value;
		GUI::Sprite*	Sprite;
	};

	rVector<SliderObject>	m_SliderEntries;

	GUI::Sprite*			m_SliderBackground;

	float					m_MinValue = 0.0f;
	float					m_MaxValue = 1.0f;

	bool					m_MouseWasDown;
	int						m_LastMovedID;
	int						m_NextID = -1;

	glm::ivec2				m_Position;
	glm::ivec2				m_Size;

	const char*				m_Window;
};