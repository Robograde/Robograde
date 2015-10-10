/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/SpriteDefinition.h"
#include "../graphics2D/TextDefinition.h"
#include "Window.h"


namespace GUI
{
	struct ComboBoxItem
	{
		SpriteDefinition	Background;
		TextDefinition		Text;
		Rectangle			BoundingBox;
		bool				Visible = false;
	};
	
	class ComboBox : public Object
	{
	public:
		ComboBox();
		ComboBox( const rString& name, const rString& parent, Rectangle boundingBox );
		~ComboBox();
		
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		void					Click(); 
		void					Render( glm::ivec2 parentPos );
		
		//Getters
		GUI_API TextDefinition&	GetTextDefinitionRef();
		GUI_API SpriteDefinition& GetBackgroundRef();
		GUI_API unsigned int	GetSelectedIndex();
		GUI_API int				GetNumItems();
		
		//Setters
		GUI_API void			SetText( const rString& text );
		GUI_API void			SetTextAlignment( ALIGNMENT textAlign );
		
		GUI_API void			SetSelectedIndex( unsigned int index );
		GUI_API void			SetMaxShownItems( int amount );

		GUI_API void			SetIndex( unsigned int index );
		
		
		GUI_API void			SetBackgroundImage( const rString& imagePath );
		GUI_API void			SetItemBackgroundImage( const rString& imagePath );
		GUI_API void			SetColour( glm::vec4 colour );
		GUI_API void			SetHighLightColour( glm::vec4 colour );
		GUI_API void			SetClickScript( const rString& script );
		
		GUI_API void			AddItem( const rString& item );
		GUI_API void			ClearItems();

		GUI_API ComboBoxItem&	GetItem( unsigned int index );

		GUI_API void			SetDirectionUp( );
		
	private:
		TextDefinition				m_TextDefinition;
		TextDefinition				m_TextShadow;
									
		rString						m_Script;
									
		bool						m_Hover = false;
		bool						m_Down = false;
									
		glm::vec4					m_Colour;
		glm::vec4					m_HighLightColour;
		
		
		SpriteDefinition			m_Background;
		SpriteDefinition			m_ArrowIcon;
		
		SpriteDefinition			m_ScrollBarBackground;
		SpriteDefinition			m_ScrollBar;
		
		rString						m_ItemTexture;

		int							m_MaxShownItems;
		int							m_ScrollOffset;
		
		//ComboBox specific
		pVector<ComboBoxItem>		m_Items;
		
		Rectangle					m_ItemBoxBounds;
		
		unsigned int				m_SelectedIndex;
		bool						m_IsOpen = false;
		bool						m_KeepOpen = false;

		bool						m_OverrideDirection = false;
		
		FONT_ID						m_FontID;
		
		Window						m_Window;
	};
}
