/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Button.h"
#include "../GUIEngine.h"
#include <script/ScriptEngine.h>
#include <input/Input.h>

namespace GUI
{
	Button::Button()
	{
	
	}
	
	Button::Button( const rString& name, const rString& parent, Rectangle boundingBox ) :
		Object( name, parent, boundingBox )
	{
		m_BoundingBox		= boundingBox;
		
		m_Background		= SpriteDefinition( "", boundingBox.X, boundingBox.Y, boundingBox.Width, boundingBox.Height );
		
		m_TextColour = glm::vec4( 1.0f );
		m_TextDefinition.Colour = m_TextColour;

		m_Colour = m_Background.Colour;
		m_HighLightColour = glm::vec4( 0.8f, 1.0f, 1.0f, 1.0f );

		m_ImageColour = glm::vec4( 1.0f );
		m_ImageHighLightColour = glm::vec4( 1.0f );
		
		m_TextDefinition.Alignment = ALIGNMENT_MIDDLE_CENTER;
		m_TextDefinition.BoundsSize = glm::ivec2( boundingBox.Width - 2, boundingBox.Height - 2 );

		m_SecondaryTextDefinition.Alignment = ALIGNMENT_BOTTOM_RIGHT;
		m_SecondaryTextDefinition.BoundsSize = glm::ivec2( boundingBox.Width - 2, boundingBox.Height - 2 );

		m_DisabledColour = glm::vec4( 0.3f, 0.3f, 0.3f, 0.0f );
		
		
		m_ImagePath = "";
		m_ToggledImagePath = "";

		m_DisabledClickEffect = false;
		m_ClickEffectDuration = 0.2f;
		m_ClickEffectCountDown = 0.0f;
	}
	
	Button::~Button()
	{
	
	}
	
	void Button::Initialize()
	{
	
	}
	
	void Button::Update( float deltaTime , glm::ivec2 parentPos )
	{
		Object::Update( deltaTime, parentPos );
		m_BoundingBox.Origin = parentPos;
		if( m_BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() )  && g_GUI.IsInputEnabled() )
		{
			m_Hover = true;
			if( m_Enabled )
			{
				if( g_Input->MouseDown( MOUSE_BUTTON_LEFT ) )
				{
					m_Down = true;
				}
				else
					m_Down = false;
				
				if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT )  )
				{
					Click();
					g_Input->ConsumeMouseButtons();
				}
					
				if( m_Down || g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
					g_Input->ConsumeMouseButtons();
			}
		}
		else
		{
			m_Down = false;
			m_Hover = false;
		}

		if( m_ClickEffectCountDown > 0 )
			m_ClickEffectCountDown -= deltaTime;
		
		
		if( m_WasHovered && !m_Hover )
			OnMouseLeave();
		else if( !m_WasHovered && m_Hover )
			OnMouseEnter();
		
		m_WasHovered = m_Hover;
	}
	
	void Button::Click()
	{
		
		if( ( !m_Toggled || !m_InGroup ) && m_Enabled )
		{
			if( !m_DisabledClickEffect )
				m_ClickEffectCountDown = m_ClickEffectDuration;

			if( m_Script != "" )
			{
				g_Script.Perform( rString( "PlayButtonSound('" + m_Name + "');" + m_Script).c_str() );
			}
			if( m_ToggleAble )
			{
				if( m_InGroup )
					g_GUI.ToggleButtonGroup( m_Name, m_ToggleGroup );
				else
					m_Toggled = !m_Toggled;
			}
		}
		//m_BringToFront = true; //TODOIA: This caused some issues with keybindings menu, might break other things without it though..
	}
	
	void Button::Render( glm::ivec2 parentPos )
	{
		m_Background.SetBounds( m_BoundingBox );
		
		m_Background.Origin = parentPos;
		
		if( !m_Enabled )
		{
			m_Background.Colour = m_Colour - m_DisabledColour;
			m_TextDefinition.Colour = m_TextColour - m_DisabledColour;
		}
		else if( m_Toggled || m_Down )
		{
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.2f );
			m_Image.Colour = m_ImageColour * ( m_ImageHighLightColour + 0.2f );
			m_TextDefinition.Colour = m_TextColour;
		}
		else if( m_Hover )
		{
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.3f );
			m_Image.Colour = m_ImageColour * ( m_ImageHighLightColour + 0.3f );
			m_TextDefinition.Colour = m_TextColour;
		}
		else
		{
			m_Background.Colour = m_Colour;
			m_Image.Colour = m_ImageColour;
			m_TextDefinition.Colour = m_TextColour;
		}


		if( m_ClickEffectCountDown > 0 && m_Enabled )
		{
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.2f * (m_ClickEffectCountDown / m_ClickEffectDuration ) );
			m_Background.BorderColour = (glm::vec4( 1.0f ) * (m_ClickEffectCountDown / m_ClickEffectDuration )) +  glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
		}
		else
			m_Background.BorderColour = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
			
		g_GUI.EnqueueSprite( &m_Background );
		
		
		if( m_ImagePath != "" )
		{
			m_Image.SetBounds( m_BoundingBox );
			
			if( m_ToggleAble && m_IsCheckbox )
			{
				
				if( m_Toggled )
				{
					
					m_Image.TopLeftUV = glm::vec2( 0.5f, 0.0f );
					m_Image.BottomRightUV = glm::vec2( 1.0f, 1.0f );
				}
				else
				{
					m_Image.TopLeftUV = glm::vec2( 0.0f, 0.0f );
					m_Image.BottomRightUV = glm::vec2( 0.5f, 1.0f );
				}
			}
			m_Image.Texture = m_ImagePath;
			g_GUI.EnqueueSprite( &m_Image );
		}
		
		if( m_TextDefinition.Text != "" )
		{
			m_TextDefinition.Origin = parentPos;
			
			m_TextDefinition.Position = glm::ivec2( m_BoundingBox.X + 1, m_BoundingBox.Y + 1 );
			
			if( !( m_Toggled || m_Down ) )
			{
				//Possibly temporary Text Shadowing, TODOIA
				m_TextShadow = m_TextDefinition;
				m_TextShadow.Position += glm::ivec2( 1, 2 );
				m_TextShadow.Colour = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
				g_GUI.EnqueueText( &m_TextShadow );
			}
			else
				m_TextDefinition.Position += glm::ivec2( 1, 2 );
			g_GUI.EnqueueText( &m_TextDefinition );
		}

		if( m_SecondaryTextDefinition.Text != "" )
		{
			m_SecondaryTextDefinition.Origin = parentPos;
			m_SecondaryTextDefinition.Position = glm::ivec2( m_BoundingBox.X, m_BoundingBox.Y );
			g_GUI.UseFont( FONT_ID_LEKTON_8 );
			g_GUI.EnqueueText( &m_SecondaryTextDefinition );

		}
	}

	void Button::SimulateClick()
	{
		Click();
		
	}
	
	void Button::OnMouseEnter()
	{
		if( m_MouseEnterScript != "" )
			g_Script.Perform( m_MouseEnterScript.c_str() );
		
	}
	
	void Button::OnMouseLeave()
	{
		if( m_MouseLeaveScript != "" )
			g_Script.Perform( m_MouseLeaveScript.c_str() );
	}
	
	void Button::OnMouseHover()
	{
	}
	
	bool Button::GetToggled()
	{
		return m_Toggled;
	}
	
	TextDefinition& Button::GetTextDefinitionRef()
	{
		return m_TextDefinition;
	}
	
	SpriteDefinition& Button::GetBackgroundRef()
	{
		return m_Background;
	}

	SpriteDefinition& Button::GetImageRef()
	{
		return m_Image;
	}

	glm::vec4& Button::GetColour()
	{
		if( m_Background.Colour.r > 1.0f )
			m_Background.Colour.r = 1.0f;
		if( m_Background.Colour.g > 1.0f )
			m_Background.Colour.g = 1.0f;
		if( m_Background.Colour.b > 1.0f )
			m_Background.Colour.b = 1;

		return m_Background.Colour;
	}
	
	void Button::SetText( const rString& text )
	{
		m_TextDefinition.Text = text.c_str();
	}
		
	void Button::SetTextAlignment( ALIGNMENT textAlign )
	{
		m_TextDefinition.Alignment = textAlign;
	}
	
	void Button::SetSecondaryText( const rString& text )
	{
		m_SecondaryTextDefinition.Text = text.c_str();
	}

	void Button::SetSecondaryTextAlignment( ALIGNMENT textAlign )
	{
		m_SecondaryTextDefinition.Alignment = textAlign;
	}

	void Button::SetIsCheckBox( bool isCheckBox )
	{
		m_IsCheckbox = isCheckBox;
	}

	bool Button::GetIsCheckBox()
	{
		return m_IsCheckbox;
	}
	
	void Button::SetToggled( bool value )
	{
		m_Toggled = value;
		m_ToggleAble = true;
	}
	
	void Button::SetBackgroundImage( const rString& imagePath )
	{
		m_Background.Texture = imagePath.c_str();
		m_Background.Colour = glm::vec4( 1.0f );
		m_Colour = m_Background.Colour;
	}
	
	void Button::SetImagePath( const rString& imagePath )
	{
		m_ImagePath = imagePath.c_str();
	}
	
	void Button::SetToggledImagePath( const rString& imagePath )
	{
		m_ToggledImagePath = imagePath.c_str();
	}
	
	void Button::SetColour( glm::vec4 colour )
	{
		m_Colour = colour;
	}
	
	void Button::SetHighLightColour( glm::vec4 colour )
	{
		m_HighLightColour = colour;
	}

	void Button::SetImageColour( glm::vec4 colour )
	{
		m_ImageColour = colour;
	}
	
	void Button::SetImageHighLightColour( glm::vec4 colour )
	{
		m_ImageHighLightColour = colour;
	}

	void Button::SetDisabledColour( glm::vec4 colour )
	{
		m_DisabledColour = colour;
		m_DisabledColour.a = 0.0f; //Just to avoid button being transparent
	}
	
	void Button::SetClickScript( const rString& script )
	{
		m_Script = script;
	}
	
	void Button::SetMouseEnterScript( const rString& script )
	{
		m_MouseEnterScript = script;
	}
	
	void Button::SetMouseLeaveScript( const rString& script )
	{
		m_MouseLeaveScript = script;
	}
	
	void Button::SetToggleGroup( const rString& group )
	{
		m_ToggleGroup = group;
		m_ToggleAble  = group != "";
		m_InGroup = true;
		
		g_GUI.AddButtonToGroup( this, group );
	}

	void Button::DisableClickEffect( )
	{
		m_DisabledClickEffect = true;
	}
	
}
