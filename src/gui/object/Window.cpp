/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Window.h"
#include <algorithm>
#include <utility/Logger.h>
#include <input/Input.h>
#include "../GUIEngine.h"

namespace GUI
{
	Window::Window()
	{
	}
	
	Window::Window( const rString& name, const rString& parent, Rectangle boundingBox , bool border ) :
		Object( name, parent, boundingBox )
	{
		m_Border = border;
		if( border )
		{
			m_BackgroundColour = glm::vec4( 0.0f, 0.0f, 0.0f, 0.5f );
			m_BorderColour = glm::vec4( 0.0f, 0.0f, 0.0f, 0.7f );

			m_Background = SpriteDefinition( "", 0, 0, boundingBox.Width, boundingBox.Height, m_BackgroundColour );
			m_Background.BorderSize = 1;
			m_Background.BorderColour = m_BorderColour;
		}
		
		m_IsWindow = true;
	}
	
	Window::~Window()
	{
		for( auto& child : m_Children )
		{
			if( child.second )
			{
				if( child.second->IsWindow() )
					g_GUI.EraseWindow( child.second->GetName() );
					
				pDelete( child.second );
				child.second = nullptr;
			}
		}
		g_GUI.EraseWindow( m_Name );
		m_Children.clear();
		m_OrderedChildren.clear();
	}
	
	void Window::Initialize()
	{
	}
	
	void Window::Update( float deltaTime, glm::ivec2 parentPos )
	{
		m_BoundingBox.Origin = parentPos;
		if( m_Open )
		{
			//The following code controls the "bring to front" behaviour of windows
			int i = 0;
			for( std::list<Object*>::iterator childIt = m_OrderedChildren.begin(); childIt != m_OrderedChildren.end(); childIt++ )
			{
				if( ( *childIt )->GetBringToFront() )
				{
					if( ( *childIt )->IsWindow() )
					{
						if( i != m_OrderedChildren.size() - 1 )
							m_OrderedChildren.splice( m_OrderedChildren.end(), m_OrderedChildren, childIt );
					}
					else
						m_BringToFront = true;
					
				}
				i++;
			}
			
			//This updates all children
			for( std::list<Object*>::reverse_iterator childIt = m_OrderedChildren.rbegin(); childIt != m_OrderedChildren.rend(); ++childIt )
			{
				Object* child = ( *childIt );
				//Very basic Culling, TODOIA: Make better later
				child->SetOrigin( parentPos.x + m_BoundingBox.X, parentPos.y + m_BoundingBox.Y + m_ScrollOffset );
				if( !m_CullContent || m_BoundingBox.Contains( child->GetBoundingBoxRef() ) )
				{
					child->SetCulled( false );
					
					//Update if not culled
					if( child->IsEnabled() )
						child->Update( deltaTime, glm::ivec2( parentPos.x + m_BoundingBox.X, parentPos.y + m_BoundingBox.Y + m_ScrollOffset ) );
				}
				else
					child->SetCulled( true );
			}
				
			//The following code controls the "moveable" behaviour of windows
			if( g_GUI.IsInputEnabled() )
			{
				if( m_BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
				{
					if( m_Scrollable )
					{
						int oldScrollOffset = m_ScrollOffset;
						m_ScrollOffset += g_Input->GetScrollY() * 16;
						if( m_ScrollOffset > 0 )
							m_ScrollOffset = 0;
						
						if( m_ScrollOffset != oldScrollOffset )
							g_Input->ConsumeMouseButtons();
					}
					
					if( m_Border && !m_ClickThrough )
					{
						if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
						{
							if( m_Moveable )
							{
								m_Moving = true;
								m_ClickPos = glm::ivec2( g_Input->GetMousePosX() - m_BoundingBox.X , g_Input->GetMousePosY() - m_BoundingBox.Y );
							}
							m_BringToFront = true;
							
							g_Input->ConsumeMouseButtons();
						}
						if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT ) )
							g_Input->ConsumeMouseButtons();
					}
				}
				
				if( g_Input->MouseUp( MOUSE_BUTTON_LEFT )  )
					m_Moving = false;
					
				if( m_Moving )
				{
					m_BoundingBox.X = g_Input->GetMousePosX() - m_ClickPos.x;
					m_BoundingBox.Y = g_Input->GetMousePosY() - m_ClickPos.y;
					
					glm::ivec2 parentSize = g_GUI.GetWindowSize( m_Parent );
					
					if( m_BoundingBox.X < 0 )
						m_BoundingBox.X = 0;
					else if( m_BoundingBox.X + m_BoundingBox.Width > parentSize.x )
						m_BoundingBox.X = parentSize.x - m_BoundingBox.Width;
					if( m_BoundingBox.Y < 0 )
						m_BoundingBox.Y = 0;
					else if( m_BoundingBox.Y + m_BoundingBox.Height > parentSize.y )
						m_BoundingBox.Y = parentSize.y - m_BoundingBox.Height;
					
					
				}
			}
		}
	}
	
	void Window::Render( glm::ivec2 parentPos )
	{
		if( m_Open )
		{
			if( m_Border )
			{
				m_Background.Colour = m_BackgroundColour * m_Opacity;
				m_Background.BorderColour = m_BorderColour * m_Opacity;

				m_Background.Origin = parentPos + glm::ivec2( m_BoundingBox.X, m_BoundingBox.Y );
				g_GUI.EnqueueSprite( &m_Background );
			}
			//TODOIA: MIght want to use this later, but probably not. Depends on if I'll use depth buffer
//			if( m_CullContent )
//				m_ScissorDefinition = ScissorDefinition( m_BoundingBox.GetLeft(), m_BoundingBox.GetTop(), m_BoundingBox.Width, m_BoundingBox.Height );
//			else
//				m_ScissorDefinition = ScissorDefinition( 0, 0, 4000, 4000 );
//			g_Graphics2D.EnqueueScissor( &m_ScissorDefinition );
			
			for( Object* child : m_OrderedChildren )
			{
				if( !child->GetCulled() && child->IsVisible() )
					child->Render( glm::ivec2( parentPos.x + m_BoundingBox.X, parentPos.y + m_BoundingBox.Y + m_ScrollOffset ) );
			}
		}
	}
	
	bool Window::AddChild( const rString& name, Object* child )
	{
		auto success = m_Children.emplace( name, child );
		if( success.second )
			m_OrderedChildren.push_back( child );
		return success.second;
	}
	
	Object* Window::GetChild( const rString& name )
	{
		for( auto child : m_Children )
		{
			if( child.second->GetName() == name )
				return child.second;
		}
		Logger::Log( "Couldn't get child of name: " + name + " From parent: " + m_Name, "GUIEngine", LogSeverity::ERROR_MSG );
		return nullptr;
	}
	
	void Window::DeleteChild( const rString& name )
	{
		for( auto itChild = m_OrderedChildren.begin(); itChild != m_OrderedChildren.end(); itChild++ )
		{
			if( ( *itChild )->GetName() == name )
			{
				m_OrderedChildren.erase( itChild );
				break;
			}
		}
		
		for( auto& child : m_Children )
		{
			if( child.second->GetName() == name )
			{
				pDelete( child.second );
				child.second = nullptr;
				m_Children.erase( name );
				return;
			}
			else if( child.second->IsWindow() )
			{
				Window* window = static_cast<Window*>( child.second );
				window->DeleteChild( name );
			}
		}
	}
	
	glm::ivec2 Window::GetSize()
	{
		return glm::ivec2( m_BoundingBox.Width, m_BoundingBox.Height );
	}
	
	void Window::SetMoveable( bool moveable )
	{
		m_Moveable = moveable;
	}
	
	void Window::SetScrollable( bool scrollable )
	{
		m_Scrollable = scrollable;
		rString name = m_Name;
	}
	
	void Window::SetCulling( bool culling )
	{
		m_CullContent = culling;
	}
	
	void Window::SetClickThrough( bool clickThrough )
	{
		m_ClickThrough = clickThrough;
	}
	
	void Window::Open()
	{
		for( auto child : m_Children )
			child.second->OnWindowOpen();
			
		m_Open = true;
	}
	
	void Window::Close()
	{
		for( auto child : m_Children )
			child.second->OnWindowClose();
			
		m_Open = false;
	}
	
	void Window::ToggleOpen()
	{
		if( m_Open )
			Close();
		else
			Open();
	}
	
	void Window::SetToggleGroup( const rString& groupName )
	{
		m_ToggleGroup = groupName;
		m_InGroup = true;
		g_GUI.AddWindowToGroup( this, groupName );
	}

	void Window::SetBackgroundColour( glm::vec4 colour )
	{
		m_BackgroundColour = colour;
	}
}
