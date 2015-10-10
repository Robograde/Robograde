/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/SpriteDefinition.h"
#include "../graphics2D/TextDefinition.h"


namespace GUI
{
	class Button : public Object
	{
	public:
		Button();
		Button( const rString& name, const rString& parent, Rectangle boundingBox );
		~Button();
		
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		
		void					Render( glm::ivec2 parentPos );

		GUI_API void			SimulateClick();
		GUI_API void			OnMouseEnter();
		GUI_API void			OnMouseLeave();
		GUI_API void			OnMouseHover();
		
		//Getters
		GUI_API bool			GetToggled();
		GUI_API TextDefinition&	GetTextDefinitionRef();
		GUI_API SpriteDefinition& GetBackgroundRef();
		GUI_API SpriteDefinition& GetImageRef();
		
		//Setters
		GUI_API void			SetText( const rString& text );
		GUI_API void			SetTextAlignment( ALIGNMENT textAlign );

		GUI_API void			SetSecondaryText( const rString& text );
		GUI_API void			SetSecondaryTextAlignment( ALIGNMENT textAlign );
		
		//For radio button behaviour
		GUI_API void			SetToggled( bool value );

		GUI_API void			SetIsCheckBox( bool isCheckBox );
		GUI_API bool			GetIsCheckBox();
		
		
		GUI_API void			SetBackgroundImage( const rString& imagePath );
		GUI_API void			SetImagePath( const rString& imagePath );
		GUI_API void			SetToggledImagePath( const rString& imagePath );
		
		
		GUI_API void			SetColour( glm::vec4 colour );
		GUI_API void			SetHighLightColour( glm::vec4 colour );
		GUI_API void			SetImageColour( glm::vec4 colour );
		GUI_API void			SetImageHighLightColour( glm::vec4 colour );

		GUI_API void			SetDisabledColour( glm::vec4 colour );


		GUI_API void			SetClickScript( const rString& script );
		GUI_API void			SetMouseEnterScript( const rString& script );
		GUI_API void			SetMouseLeaveScript( const rString& script );
		
		GUI_API void			SetToggleGroup( const rString& group );

		GUI_API void			DisableClickEffect( );
	private:
		TextDefinition	m_TextShadow;
		TextDefinition	m_TextDefinition;
		
		TextDefinition	m_SecondaryTextDefinition;
		
		rString			m_Script;
		rString			m_MouseEnterScript = "";
		rString			m_MouseLeaveScript = "";
		
		bool			m_ToggleAble = false;
		bool			m_Toggled = false;
		bool			m_InGroup = false;
		
		bool			m_Hover = false;
		bool			m_Down = false;
		bool			m_WasHovered = false;
		bool			m_IsCheckbox = false;
		
		glm::vec4		m_Colour;
		glm::vec4		m_TextColour;
		glm::vec4		m_HighLightColour;
		glm::vec4		m_ToggleColour;
		glm::vec4		m_ImageColour;
		glm::vec4		m_ImageHighLightColour;
		glm::vec4		m_DisabledColour;
		
		
		SpriteDefinition	m_Background;
		SpriteDefinition	m_Image;
		
		rString				m_ImagePath;
		rString				m_ToggledImagePath;
		
		rString				m_ToggleGroup;

		void				Click();


		float				m_ClickEffectCountDown;
		float				m_ClickEffectDuration;

		bool				m_DisabledClickEffect;
	};
}
