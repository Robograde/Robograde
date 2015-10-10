/**************************************************
2015 David Pejtersen
***************************************************/
#include "SSEditorCameraPaths.h"
#include <script/ScriptEngine.h>
#include <input/Input.h>
#include "../gfx/SSRender.h"
#include "../../input/GameMessages.h"

void SSEditorCameraPaths::Startup( )
{
	m_SimInterests = MessageTypes::EDITOR_CAMERA_PATHS;
	g_SSMail.RegisterSubscriber( this );

	// Init gui
	int x = 16, y = 16;

	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	
	// Create GUI elements
	m_SplineList	= g_GUI.AddComboBox	( "", GUI::Rectangle( x,							y,			128,		32 ), m_WindowName );
	m_SplineNodes	= g_GUI.AddComboBox	( "", GUI::Rectangle( x + 128 + 64 + 32,			y,			128,		32 ), m_WindowName );
	m_AddSpline		= g_GUI.AddButton	( "", GUI::Rectangle( x + 128,						y,			32,			32 ), m_WindowName );
	m_AddNode		= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 64 + 32,			y,			32,			32 ), m_WindowName );
	m_RemoveSpline	= g_GUI.AddButton	( "", GUI::Rectangle( x + 128 + 32,					y,			32,			32 ), m_WindowName );
	m_RemoveNode	= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 64 + 32 + 32,		y,			32,			32 ), m_WindowName );
	m_PlayButton	= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 128 + 64,			y,			128,		32 ), m_WindowName );
	m_StopButton	= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 128 + 64,			y + 32,		128,		32 ), m_WindowName );
	m_UseRTSCamera	= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 128 + 64 + 128,		y,			64,			32 ), m_WindowName );
	m_UseFPSCamera	= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 128 + 128 + 128,	y,			64,			32 ), m_WindowName );
	m_GoToNode		= g_GUI.AddButton	( "", GUI::Rectangle( x + 256 + 128 + 64 + 128,		y + 32,		128,		32 ), m_WindowName );
	m_TimeBox		= g_GUI.AddTextBox	( "", GUI::Rectangle( x + 128 + 64  + 32,			y + 32,		128 + 64,	32 ), m_WindowName );
	m_NameBox		= g_GUI.AddTextBox	( "", GUI::Rectangle( x,							y + 32,		128 + 64,	32 ), m_WindowName );

	// Set background images
	m_SplineList  ->SetBackgroundImage( "Button_128x32.png" );
	m_SplineNodes ->SetBackgroundImage( "Button_128x32.png" );
	m_AddSpline   ->SetBackgroundImage( "Button_32.png" );
	m_AddNode     ->SetBackgroundImage( "Button_32.png" );
	m_RemoveSpline->SetBackgroundImage( "Button_32.png" );
	m_RemoveNode  ->SetBackgroundImage( "Button_32.png" );
	m_PlayButton  ->SetBackgroundImage( "Button_128x32.png" );
	m_StopButton  ->SetBackgroundImage( "Button_128x32.png" );
	m_UseRTSCamera->SetBackgroundImage( "Button_64x32.png" );
	m_UseFPSCamera->SetBackgroundImage( "Button_64x32.png" );
	m_GoToNode    ->SetBackgroundImage( "Button_128x32.png" );

	m_SplineList ->SetItemBackgroundImage( "Button_128x32.png" );
	m_SplineNodes->SetItemBackgroundImage( "Button_128x32.png" );

	// Set open directions
	m_SplineList ->SetDirectionUp( );
	m_SplineNodes->SetDirectionUp( );

	// Set colours
	m_AddSpline   ->SetColour( glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
	m_AddNode     ->SetColour( glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
	m_RemoveSpline->SetColour( glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	m_RemoveNode  ->SetColour( glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

	// Set texts
	m_AddSpline   ->SetText( "+" );
	m_AddNode     ->SetText( "+" );
	m_RemoveSpline->SetText( "-" );
	m_RemoveNode  ->SetText( "-" );
	m_PlayButton  ->SetText( "Play" );
	m_StopButton  ->SetText( "Stop" );
	m_UseRTSCamera->SetText( "RTS Cam" );
	m_UseFPSCamera->SetText( "FPS Cam" );
	m_GoToNode    ->SetText( "Go To Node" );

	// Set click scripts
	m_AddSpline   ->SetClickScript( "EDITOR_AddCameraSpline( );" );
	m_AddNode     ->SetClickScript( "EDITOR_AddCameraNode( );" );
	m_RemoveSpline->SetClickScript( "EDITOR_RemoveCameraSpline( );" );
	m_RemoveNode  ->SetClickScript( "EDITOR_RemoveCameraNode( );" );
	m_PlayButton  ->SetClickScript( "EDITOR_RunCameraSpline( );" );
	m_StopButton  ->SetClickScript( "EDITOR_StopCameraSpline( );" );
	m_UseRTSCamera->SetClickScript( "EDITOR_UseRTSCamera( );" );
	m_UseFPSCamera->SetClickScript( "EDITOR_UseFPSCamera( );" );
	m_GoToNode    ->SetClickScript( "EDITOR_GoToSplineNode( );" );

	// Create button script functions
	g_Script.Register( "EDITOR_AddCameraSpline",	[&] ( IScriptEngine* ) { OnEventAddSpline( );																				return 0; } );
	g_Script.Register( "EDITOR_AddCameraNode",		[&] ( IScriptEngine* ) { OnEventAddNode( );																					return 0; } );
	g_Script.Register( "EDITOR_RemoveCameraSpline",	[&] ( IScriptEngine* ) { OnEventRemoveSpline( );																			return 0; } );
	g_Script.Register( "EDITOR_RemoveCameraNode",	[&] ( IScriptEngine* ) { OnEventRemoveNode( );																				return 0; } );
	g_Script.Register( "EDITOR_StopCameraSpline",	[&] ( IScriptEngine* ) { g_SSCamera.GetSplineCamera( )->Stop( );															return 0; } );
	g_Script.Register( "EDITOR_UseRTSCamera",		[&] ( IScriptEngine* ) { g_SSCamera.UseRTSCamera( );																		return 0; } );
	g_Script.Register( "EDITOR_UseFPSCamera",		[&] ( IScriptEngine* ) { g_SSCamera.UseFPSCamera( );																		return 0; } );
	g_Script.Register( "EDITOR_RunCameraSpline",	[&] ( IScriptEngine* ) {
		float time = m_Timeline->GetValue( m_TimelinePlayer );
		if ( time + 0.1f < g_SSCamera.GetSplineCamera( )->GetMaxTime( m_CurrentSpline ) )
			g_SSCamera.GetSplineCamera( )->Run( m_CurrentSpline, time );
		else
			g_SSCamera.GetSplineCamera( )->Run( m_CurrentSpline );
		return 0;
	} );
	g_Script.Register( "EDITOR_GoToSplineNode",		[&] ( IScriptEngine* ) {
		g_SSCamera.UseFPSCamera( );
		g_SSCamera.GetActiveCamera( )->SetPosition( g_SSCamera.GetSplineCamera()->GetNodePosition( m_CurrentSpline, m_CurrentNode ) );
		g_SSCamera.GetActiveCamera( )->SetOrientation( g_SSCamera.GetSplineCamera( )->GetNodeOrientation( m_CurrentSpline, m_CurrentNode ) );
		return 0;
	} );

	// Create the timeline
	m_Timeline = pNew( MultiSlider, glm::ivec2( x, y + 96 ), glm::ivec2( 704, 16 ), m_WindowName );

	// Update combo box lists
	UpdateSplineList( );
	UpdateNodeList( );
}

void SSEditorCameraPaths::Shutdown( )
{
	pDelete( m_Timeline );
}

void SSEditorCameraPaths::UpdateUserLayer( const float deltaTime )
{
	// Track selected spline
	auto spline = m_ComboIndexToSplineID.find( m_SplineList->GetSelectedIndex( ) );
	if ( spline != m_ComboIndexToSplineID.end( ) )
		m_CurrentSpline = spline->second;
	else
		m_CurrentSpline = NO_SPLINE;

	if ( m_PrevSpline != m_CurrentSpline )
	{
		UpdateNodeList( );
		m_PrevSpline	= m_CurrentSpline;
		m_CurrentNode	= -1;
		
		m_NameBox->SetText( m_SplineList->GetItem( m_SplineList->GetSelectedIndex( ) ).Text.Text );
	}

	// Update name field for spline
	if ( m_CurrentSpline != NO_SPLINE && !m_NameBox->IsInputting( ) && m_SplineList->GetItem( m_SplineList->GetSelectedIndex( ) ).Text.Text != m_NameBox->GetText( ) )
	{
		g_SSCamera.GetSplineCamera( )->SetSplineName( m_CurrentSpline, m_NameBox->GetText( ).c_str( ) );
		UpdateSplineList( );
	}

	// Update time field for node
	float nodeTime = g_SSCamera.GetSplineCamera( )->GetNodeTime( m_CurrentSpline, m_CurrentNode );
	if ( m_CurrentNode != m_SplineNodes->GetSelectedIndex( ) )
	{
		m_CurrentNode	= m_SplineNodes->GetSelectedIndex( );
		nodeTime		= g_SSCamera.GetSplineCamera( )->GetNodeTime( m_CurrentSpline, m_CurrentNode );
		m_TimeBox->SetText( rToString( nodeTime ) );
	}

	float time = strtof( m_TimeBox->GetText( ).c_str( ), nullptr );
	if ( time != nodeTime && !m_TimeBox->IsInputting( ) )
	{
		int node = g_SSCamera.GetSplineCamera( )->SetNodeTime( m_CurrentSpline, m_CurrentNode, time );
		m_SplineNodes->SetSelectedIndex( node );
		UpdateTimeline( );
		
		m_Timeline->SetLastMoved( node );
	}

	// Draw spline
	if ( g_GUI.IsWindowOpen( m_WindowName ) )
	{
		float maxTime		= g_SSCamera.GetSplineCamera( )->GetMaxTime( m_CurrentSpline );
		float sampleRate	= maxTime / 100.0f; // <-- nr of lines
		for ( float f = 0.0f; f < maxTime - sampleRate; f += sampleRate )
		{
			// TODODP: Optimize!! (GetSplinePosition only need to be run once per iteration by reusing the result)
			g_SSRender.RenderLine(
				g_SSCamera.GetSplineCamera( )->GetSplinePosition( m_CurrentSpline, f ),
				g_SSCamera.GetSplineCamera( )->GetSplinePosition( m_CurrentSpline, f + sampleRate ),
				glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ), false, 0.0f );
		}
	}

	// Update timeline input
	m_Timeline->UpdatePositions( g_GUI.GetWindowPos( "BottomBarWindow" ) );

	for ( auto& it : m_NodeIDToTimelineID )
	if ( m_Timeline->GetMoved( ) == it.second )
	{
		m_SplineNodes->SetSelectedIndex( (unsigned int) it.first );
		
		float value = m_Timeline->GetValue( it.second );
		if ( value != g_SSCamera.GetSplineCamera( )->GetNodeTime( m_CurrentSpline, it.first ) )
			m_TimeBox->SetText( rToString( value ) );
	}

	if ( g_SSCamera.GetSplineCamera( )->Running( ) )
		m_Timeline->SetValue( m_TimelinePlayer, g_SSCamera.GetSplineCamera( )->GetRunTime( ) );
	else if ( m_Timeline->GetMoved( ) == m_TimelinePlayer )
	{
		float time = m_Timeline->GetValue( m_TimelinePlayer );

		g_SSCamera.UseFPSCamera( );
		g_SSCamera.GetActiveCamera( )->SetPosition( g_SSCamera.GetSplineCamera()->GetSplinePosition( m_CurrentSpline, time ) );
		g_SSCamera.GetActiveCamera( )->SetOrientation( g_SSCamera.GetSplineCamera( )->GetSplineOrientation( m_CurrentSpline, time ) );
	}
}

void SSEditorCameraPaths::UpdateSimLayer( const float timeStep )
{
	for ( const Message* fromMail : m_SimMailbox )
	{
		const EditorCameraPathsMessage* msg = static_cast<const EditorCameraPathsMessage*>(fromMail);
		switch ( msg->Event )
		{
			case EditorCameraPathsMessage::EventType::NewSpline:
				g_SSCamera.GetSplineCamera( )->NewSpline( msg->SplineName.c_str( ) );
				break;

			case EditorCameraPathsMessage::EventType::DeleteSpline:
				g_SSCamera.GetSplineCamera( )->DeleteSpline( msg->Spline );
				break;

			case EditorCameraPathsMessage::EventType::RenameSpline:
				g_SSCamera.GetSplineCamera( )->SetSplineName( msg->Spline, msg->SplineName.c_str( ) );
				break;

			case EditorCameraPathsMessage::EventType::NewNode:
				g_SSCamera.GetSplineCamera( )->NewNode( msg->Spline, msg->NodeTime, msg->NodePosition, msg->NodeOrientation );
				break;

			case EditorCameraPathsMessage::EventType::DeleteNode:
				g_SSCamera.GetSplineCamera( )->DeleteNode( msg->Spline, msg->Node );
				break;

			case EditorCameraPathsMessage::EventType::ChangeNodeTime:
				g_SSCamera.GetSplineCamera( )->SetNodeTime( msg->Spline, msg->Node, msg->NodeTime );
				break;

			case EditorCameraPathsMessage::EventType::None:
			default:
				break;
		}
	}
}

void SSEditorCameraPaths::UpdateSplineList( )
{
	m_ComboIndexToSplineID.clear( );
	m_SplineList->ClearItems( );

	size_t size = g_SSCamera.GetSplineCamera( )->GetNumSplines( );
	if ( size == 0 )
		m_SplineList->AddItem( "No Spline" );
	else
		for ( size_t i = 0; i < size; ++i )
		{
			const CameraSpline::SplineInfo& info = g_SSCamera.GetSplineCamera( )->GetSplineInfoFromIndex( i );
			m_SplineList->AddItem( info.Name );
			m_ComboIndexToSplineID.emplace( i, info.ID );

			if ( m_CurrentSpline == info.ID )
				m_SplineList->SetSelectedIndex( (unsigned int) i );
		}
}

void SSEditorCameraPaths::UpdateNodeList( )
{
	m_SplineNodes->ClearItems( );

	size_t size = g_SSCamera.GetSplineCamera( )->GetNumNodes( m_CurrentSpline );
	if ( size == 0 )
		m_SplineNodes->AddItem( "No Nodes" );
	else
		for ( size_t i = 0; i < size; ++i )
			m_SplineNodes->AddItem( rString( "Node " ) + rToString( i + 1 ) );

	UpdateTimeline( );
}

void SSEditorCameraPaths::UpdateTimeline( )
{
	m_NodeIDToTimelineID.clear( );

	float time = 0.0f;
	if ( m_TimelinePlayer >= 0 )
		time = m_Timeline->GetValue( m_TimelinePlayer );

	m_Timeline->ClearAll( );
	m_Timeline->SetMin( 0.0f );
	m_Timeline->SetMax( g_SSCamera.GetSplineCamera( )->GetMaxTime( m_CurrentSpline ) * 1.02f );

	size_t size = g_SSCamera.GetSplineCamera( )->GetNumNodes( m_CurrentSpline );
	for ( size_t i = 0; i < size; ++i )
		m_NodeIDToTimelineID.emplace( i, m_Timeline->AddEntry( g_SSCamera.GetSplineCamera( )->GetNodeTime( m_CurrentSpline, i ) ) );

	if ( size >= 2 )
	{
		m_TimelinePlayer = m_Timeline->AddEntry( time );
		m_Timeline->SetColour( m_TimelinePlayer, glm::vec4( 0.4f, 1.0f, 0.4f, 1.0f ) );
	}

	m_Timeline->RefreshPositions( );
}

void SSEditorCameraPaths::OnEventAddSpline( )
{
	m_CurrentSpline = g_SSCamera.GetSplineCamera( )->NewSpline( (rString( "Spline " ) + rToString( g_SSCamera.GetSplineCamera( )->GetNumSplines( ) + 1 )).c_str( ) );
	UpdateSplineList( );
}

void SSEditorCameraPaths::OnEventRemoveSpline( )
{
	g_SSCamera.GetSplineCamera( )->DeleteSpline( m_CurrentSpline );
	UpdateSplineList( );

	m_CurrentSpline = NO_SPLINE;
}

void SSEditorCameraPaths::OnEventAddNode( )
{
	if ( m_CurrentSpline != NO_SPLINE )
	{
		g_SSCamera.GetSplineCamera( )->NewNode(
			m_CurrentSpline,
			g_SSCamera.GetSplineCamera( )->GetMaxTime( m_CurrentSpline ) + 1.0f, // TODODP: Add field for time instead
			g_SSCamera.GetActiveCamera( )->GetPosition( ),
			g_SSCamera.GetActiveCamera( )->GetOrientation( )
			);
		UpdateNodeList( );

		m_SplineNodes->SetSelectedIndex( (unsigned int) g_SSCamera.GetSplineCamera( )->GetNumNodes( m_CurrentSpline ) - 1 );
	}
}

void SSEditorCameraPaths::OnEventRemoveNode( )
{
	int nodeIndex = m_SplineNodes->GetSelectedIndex( );
	g_SSCamera.GetSplineCamera( )->DeleteNode( m_CurrentSpline, nodeIndex );
	UpdateNodeList( );

	if ( nodeIndex != 0 )
		m_SplineNodes->SetSelectedIndex( nodeIndex - 1 );
}

//////////////////////////////////////////////////////////////////////////////////
SSEditorCameraPaths::MultiSlider::MultiSlider( glm::ivec2 position, glm::ivec2 size, const char* window )
{
	m_Position	= position;
	m_Size		= size;
	m_Window	= window;

	m_SliderBackground = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", position.x, position.y + (size.y / 4), size.x, size.y / 2, glm::vec4( 0.9f, 0.9f, 1.0f, 1.0f ) ), m_Window );
	m_SliderBackground->SetTexture( "Button_256x16.png" );
}

int SSEditorCameraPaths::MultiSlider::AddEntry( float value )
{
	SliderObject* entry = nullptr;
	for ( auto& it : m_SliderEntries )
	if ( it.Active == false )
	{
		entry = &it;
		break;
	}

	if ( entry == nullptr )
	{
		m_SliderEntries.push_back( SliderObject( ) );
		entry = &m_SliderEntries.back( );

		entry->Sprite = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", m_Position.x, m_Position.y, m_Size.y, m_Size.y, glm::vec4( 0.8f, 0.8f, 1.0f, 0.95f ) ), m_Window );
		entry->Sprite->SetTexture( "Button_32.png" );
	}

	entry->ID		= ++m_NextID;
	entry->Active	= true;
	entry->Value	= value;

	if ( value < m_MinValue )
		m_MinValue = value;
	if ( value > m_MaxValue )
		m_MaxValue = value;

	return entry->ID;
}

void SSEditorCameraPaths::MultiSlider::ClearAll( )
{
	for ( auto& it : m_SliderEntries )
	{
		it.Active = false;
		it.Sprite->GetSpriteDefinitionRef( ).Colour = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	}
	m_NextID = -1;
}

void SSEditorCameraPaths::MultiSlider::UpdatePositions( glm::ivec2 parentPos )
{
	glm::ivec2	mousePos	= glm::ivec2( g_Input->GetMousePosX( ), g_Input->GetMousePosY( ) );
	glm::ivec2	bgPos		= m_SliderBackground->GetPosition( ) + parentPos;
	bool		mouseDown	= g_Input->MouseDown( MOUSE_BUTTON_LEFT );

	if ( mouseDown )
	{
		for ( auto it = m_SliderEntries.rbegin( ); it != m_SliderEntries.rend( ); ++it )
		{
			if ( it->Active )
			{
				glm::ivec2 pos	= it->Sprite->GetPosition( ) + parentPos;
				glm::ivec2 size	= it->Sprite->GetSize( );

				// Move function
				auto moveNode = [&] ( ) -> void
				{
					int x, y;
					x = mousePos.x - parentPos.x - size.x / 2;
					x = glm::min( glm::max( x, bgPos.x - parentPos.x ), bgPos.x - parentPos.x + m_SliderBackground->GetSize( ).x - size.x );
					y = it->Sprite->GetPosition( ).y;

					it->Sprite->SetPosition( x, y );
					it->Value = m_MinValue + (m_MaxValue - m_MinValue) * (x - size.x) / (float) (m_SliderBackground->GetSize( ).x - size.x);
				};

				// Move conditions
				if ( m_LastMovedID == it->ID )
				{
					moveNode( );
					break;
				}
				if ( !m_MouseWasDown && pos.x <= mousePos.x && mousePos.x <= pos.x + size.x && pos.y <= mousePos.y && mousePos.y <= pos.y + size.y )
				{
					moveNode( );
					m_LastMovedID = it->ID;
					break;
				}
			}
		}

		m_MouseWasDown	= true;
	}
	else
	{
		m_MouseWasDown	= false;
		m_LastMovedID	= -1;
	}
}

void SSEditorCameraPaths::MultiSlider::RefreshPositions( )
{
	for ( auto& it : m_SliderEntries )
	{
		int x = m_Position.x + (int) ((m_Size.x - it.Sprite->GetSize( ).x) * it.Value / m_MaxValue);
		int y = m_Position.y;

		it.Sprite->SetPosition( x, y );

		it.Sprite->SetVisible( false );
		if ( it.Active )
			it.Sprite->SetVisible( true );
	}
}

float SSEditorCameraPaths::MultiSlider::GetValue( int node ) const
{
	for ( auto& it : m_SliderEntries )
		if ( it.ID == node && it.Active )
			return it.Value;
	return m_MinValue;
}

void SSEditorCameraPaths::MultiSlider::SetColour( int node, glm::vec4 colour )
{
	for ( auto& it : m_SliderEntries )
		if ( it.ID == node && it.Active )
			it.Sprite->GetSpriteDefinitionRef( ).Colour = colour;
}

void SSEditorCameraPaths::MultiSlider::SetValue( int node, float value )
{
	for ( auto& it : m_SliderEntries )
	if ( it.ID == node && it.Active )
	{
		it.Value = value;
		RefreshPositions( );
	}
}