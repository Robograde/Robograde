/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "ComboBox.h"
#include "../GUIEngine.h"
#include <script/ScriptEngine.h>
#include <input/Input.h>

namespace GUI
{
	ComboBox::ComboBox()
	{
	
	}
	
	ComboBox::ComboBox( const rString& name, const rString& parent, Rectangle boundingBox ) :
		Object( name, parent, boundingBox )
	{
		m_BoundingBox		= boundingBox;
		m_ItemBoxBounds		= boundingBox;
		
		//This is the background sprite for the combobox button
		m_Background		= SpriteDefinition( "", boundingBox.X, boundingBox.Y, boundingBox.Width, boundingBox.Height, glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		m_Background.BorderSize = 1;

		m_Colour = m_Background.Colour;
		m_HighLightColour = glm::vec4( 0.5f, 0.8f, 1.0f, 1.0f );

		m_ArrowIcon = SpriteDefinition( "ComboBoxArrow.png", boundingBox.X + boundingBox.Width - 32, boundingBox.Y, 32, 32, glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		
		//This is the textdefinition for the combobox button
		m_TextDefinition.Alignment = ALIGNMENT_MIDDLE_CENTER;
		m_TextDefinition.BoundsSize = glm::ivec2( boundingBox.Width - 2 - 32, boundingBox.Height - 2 );
		
		//A rectangle that cover the cmbobox items
		m_ItemBoxBounds.Y = boundingBox.Y + boundingBox.Height;
		
		//The max amount of items that are shown at a time
		m_MaxShownItems = 6;
		m_ScrollOffset = 0;
		
		m_ScrollBarBackground = SpriteDefinition( "", boundingBox.X + boundingBox.Width - 16, boundingBox.Y + boundingBox.Height, 16, boundingBox.Height * m_MaxShownItems );
		m_ScrollBarBackground.BorderSize = 2;
		m_ScrollBar =  SpriteDefinition( "", m_ScrollBarBackground.Position.x, m_ScrollBarBackground.Position.y, 16, boundingBox.Height * m_MaxShownItems, glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f ) );
		m_ScrollBar.BorderSize = 2;
		
		
		m_Items.reserve( 30 );
		m_SelectedIndex = 0;
		m_IsOpen = false;
		
		m_FontID = g_GUI.GetCurrentFont();
		m_TextDefinition.FontID = m_FontID;

		m_OverrideDirection = false;
	}
	
	ComboBox::~ComboBox()
	{
	
	}
	
	void ComboBox::Initialize()
	{
	
	}
	
	void ComboBox::Update( float deltaTime , glm::ivec2 parentPos )
	{
		if( m_BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) && m_Enabled )
		{
			m_Hover = true;
			if( g_Input->MouseDown( MOUSE_BUTTON_LEFT ) )
			{
				m_Down = true;
			}
			else
				m_Down = false;
			
			if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT )  )
			{
				m_Down = true;
				g_Input->ConsumeMouseButtons();
				Click();
			}
		}
		else
		{
			m_Down = false;
			m_Hover = false;
		}


		int numItems = static_cast<int>( m_Items.size() );
	
		if( m_IsOpen )
		{
			m_Down = true;
			
			if( numItems < m_MaxShownItems )
				m_ItemBoxBounds.Height = m_BoundingBox.Height * numItems;
			else
				m_ItemBoxBounds.Height = m_BoundingBox.Height * m_MaxShownItems;
			
			
			if( m_OverrideDirection )
			{
				m_ItemBoxBounds.Y = m_BoundingBox.GetPosition().y - m_ItemBoxBounds.Height;
			}


			if( !m_ItemBoxBounds.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
			{
				if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT )  )
				{
					m_IsOpen = false;
				}
			}
			else
			{
				m_ScrollOffset -= g_Input->GetScrollY( true );
				
				if( m_ScrollOffset + m_MaxShownItems + 1 > numItems )
					m_ScrollOffset = numItems - m_MaxShownItems;
				if( m_ScrollOffset < 0 )
					m_ScrollOffset = 0;
			}
			
			int i = 0;
			for( ComboBoxItem& item : m_Items )
			{
				if( i == m_SelectedIndex )
				{
					item.Background.Colour = m_Colour * ( m_HighLightColour + 0.7f );
					item.Background.BorderSize = 2;
				}
				else
				{
					item.Background.Colour = m_Colour;
					item.Background.BorderSize = 0;
				}
				
				if( item.BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
				{
					item.Background.Colour = m_Colour * ( m_HighLightColour + 0.1f );
					
					if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT )  )
					{
						m_IsOpen = false;
						m_SelectedIndex = i;
						m_TextDefinition.Text = item.Text.Text;
						g_Input->ConsumeMouseButtons();
						if( m_Script != "" )
							g_Script.Perform( m_Script.c_str() );
					}
				}
				i++;
			}
		}
	}
	
	void ComboBox::Click()
	{
		m_IsOpen = !m_IsOpen;
		m_BringToFront = true;
	}
	
	void ComboBox::Render( glm::ivec2 parentPos )
	{
		int numItems = static_cast<int>( m_Items.size() );
		m_BoundingBox.Origin = parentPos;
		m_ItemBoxBounds.Origin = parentPos;
		m_ScrollBarBackground.Origin = parentPos;
		m_ScrollBar.Origin = parentPos;
		
		m_Background.SetBounds( m_BoundingBox );
		
		m_Background.Origin = parentPos;
		m_ArrowIcon.Origin = parentPos;
		
		if( !m_Enabled )
			m_Background.Colour = m_Colour - glm::vec4( 0.1f, 0.1f, 0.1f, 0.0f ); //Just to avoid button being transparent
		else if( m_Down )
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.2f );
		else if( m_Hover )
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.1f );
		else
			m_Background.Colour = m_Colour;
			
		g_GUI.EnqueueSprite( &m_Background );
		g_GUI.EnqueueSprite( &m_ArrowIcon );
		
		
		if( numItems > 0 )
		{
			m_TextDefinition.Text = m_Items[m_SelectedIndex].Text.Text;
			m_TextDefinition.Colour = m_Items[m_SelectedIndex].Text.Colour;
		}
		
		if( m_TextDefinition.Text != "" )
		{
			m_TextDefinition.Origin = parentPos;
			
			m_TextDefinition.Position = glm::ivec2( m_BoundingBox.X, m_BoundingBox.Y );
			
			g_GUI.EnqueueText( &m_TextDefinition );
		}
		
		if( m_IsOpen )
		{
			
			for( ComboBoxItem& item : m_Items )
			{
				//Hide items
				item.Visible = false;
				item.Background.Texture = m_ItemTexture;
			}
			
			if( m_MaxShownItems < numItems )
			{
				
				m_ScrollBarBackground.Height = m_ItemBoxBounds.Height;
				m_ScrollBarBackground.Position.y = m_ItemBoxBounds.GetPosition().y;
				m_ScrollBar.Height = m_ScrollBarBackground.Height  / (numItems - m_MaxShownItems + 1 ) ;
				m_ScrollBar.Position.y = m_ScrollBarBackground.Position.y + m_ScrollOffset * m_ScrollBar.Height;
				
				g_GUI.EnqueueSprite( &m_ScrollBarBackground );
				g_GUI.EnqueueSprite( &m_ScrollBar );
			}
			
			
			for( int i = 0; i < m_MaxShownItems ; i++ )
			{
				if( m_ScrollOffset + i < numItems )
				{
					ComboBoxItem& item = m_Items[m_ScrollOffset + i];
					if( m_MaxShownItems < numItems  )
					{
						item.Background.Width = m_BoundingBox.Width - 16;
						item.BoundingBox.Width = item.Background.Width;
					}
					
					if( m_OverrideDirection )
						item.BoundingBox.Y =  i * m_BoundingBox.Height - m_ItemBoxBounds.Height;
					else
						item.BoundingBox.Y =  i * m_BoundingBox.Height + m_BoundingBox.Height;

					item.Background.Position.y = item.BoundingBox.Y;
					
							
					item.Background.Origin =	m_Background.Origin + m_Background.Position;
					item.BoundingBox.Origin =	m_Background.Origin + m_Background.Position;
					item.Visible = true;
					g_GUI.EnqueueSprite( &item.Background );
				}
			}
			
			//Because of the way the rendering works it's more efficient to separate the sprite and text rendering
			for( int i = 0; i < m_MaxShownItems ; i++ )
			{
				if( m_ScrollOffset + i < numItems )
				{
					ComboBoxItem& item = m_Items[m_ScrollOffset + i];
					item.Text.Origin = item.BoundingBox.Origin;
					item.Text.Position = item.Background.Position;
					item.Text.BoundsSize = glm::ivec2( item.BoundingBox.Width, item.BoundingBox.Height );
					g_GUI.EnqueueText( &item.Text );
				}
			}
		}
			
	}
	
	TextDefinition&ComboBox::GetTextDefinitionRef()
	{
		return m_TextDefinition;
	}
	
	SpriteDefinition&ComboBox::GetBackgroundRef()
	{
		return m_Background;
	}
	
	unsigned int ComboBox::GetSelectedIndex()
	{
		return m_SelectedIndex;
	}

	int ComboBox::GetNumItems()
	{
		return static_cast<int>(  m_Items.size() );
	}
	
	void ComboBox::SetText( const rString& text )
	{
		int index = 0;
		for( int i = 0; i < m_Items.size(); i++ )
		{
			if( m_Items[i].Text.Text == text )
			{
				m_SelectedIndex = index;
				m_TextDefinition.Text = text;
			}
			index++;
		}
	}
	
	void ComboBox::SetTextAlignment( ALIGNMENT textAlign )
	{
		m_TextDefinition.Alignment = textAlign;
	}
	
	void ComboBox::SetSelectedIndex( unsigned int index )
	{
		m_SelectedIndex = index;
	}
	
	void ComboBox::SetMaxShownItems( int amount )
	{
		m_MaxShownItems = amount;
	}

	void ComboBox::SetIndex( unsigned int index )
	{
		if( index < m_Items.size() )
			m_SelectedIndex = index;
	}
	
	void ComboBox::SetBackgroundImage( const rString& imagePath )
	{
		m_Background.Texture = imagePath;
	}

	void ComboBox::SetItemBackgroundImage( const rString& imagePath )
	{
		m_ItemTexture = imagePath.c_str();
	}
	
	void ComboBox::SetColour( glm::vec4 colour )
	{
		m_Colour = colour;
	}
	
	void ComboBox::SetHighLightColour( glm::vec4 colour )
	{
		m_HighLightColour = colour;
	}
	
	void ComboBox::SetClickScript( const rString& script )
	{
		m_Script = script;
	}
	
	void ComboBox::AddItem( const rString& item )
	{
		int x = 0;
		int y = 0;
		int width =  m_BoundingBox.Width;
		int height = m_BoundingBox.Height;
		
		ComboBoxItem comboBoxItem;
		
		comboBoxItem.Text = TextDefinition( item.c_str(), x, y );
		comboBoxItem.Text.Alignment = ALIGNMENT_MIDDLE_CENTER;
		comboBoxItem.Text.FontID = m_FontID;
		
		comboBoxItem.Background = SpriteDefinition( "", x, y, width, height );
		comboBoxItem.Background.BorderSize = 0;
		
		comboBoxItem.BoundingBox = Rectangle( x, y, m_BoundingBox.Width, m_BoundingBox.Height );
		
		m_Items.push_back( comboBoxItem );
	}

	void ComboBox::ClearItems()
	{
		m_Items.clear();
		m_SelectedIndex = 0;
		m_ScrollOffset = 0;
	}

	ComboBoxItem& ComboBox::GetItem( unsigned int index )
	{
		assert( index < m_Items.size() );

		return m_Items.at( index );
	}

	 void ComboBox::SetDirectionUp( )
	 {
		 m_OverrideDirection = true;
	 }
}
