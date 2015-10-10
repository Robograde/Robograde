/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include <list> //TODOIA: Remove when added to allocator
#include "../graphics2D/SpriteDefinition.h"

namespace GUI
{
	class Window : public Object
	{
	public:
					Window();
					Window( const rString& name, const rString& parent, Rectangle boundingBox, bool border = false );
					~Window();
		
		void		Initialize();
		void		Update( float deltaTime, glm::ivec2 parentPos );
		void		Render(  glm::ivec2 parentPos );
		
		bool		AddChild( const rString& name, Object* child);
		Object*		GetChild( const rString& name );
		
		void		DeleteChild( const rString& name );
		
		GUI_API glm::ivec2	GetSize();
		
		GUI_API void		SetMoveable( bool moveable );
		GUI_API void		SetScrollable( bool scrollable );
		GUI_API void		SetCulling( bool culling );
		GUI_API void		SetClickThrough( bool clickThrough );
		
		GUI_API void		Open();
		GUI_API void		Close();
		GUI_API void		ToggleOpen();
		GUI_API void		SetToggleGroup( const rString& groupName );

		GUI_API void		SetOnOpenScript( const rString& script );
		GUI_API void		SetOnCloseScript( const rString& script );

		GUI_API void		SlideOpen( glm::ivec2 start, glm::ivec2 end, int pixelsPerSecond );
		GUI_API void		SlideClose( glm::ivec2 start, glm::ivec2 end, int pixelsPerSecond );
		
		GUI_API bool		IsOpen() { return m_Open; }
		
		SpriteDefinition&	GetBackgroundRef() { return m_Background; }

		GUI_API	void		SetBackgroundColour( glm::vec4 colour );
		
	private:
		
		
		pMap<rString, Object*>	m_Children;
		std::list<Object*>		m_OrderedChildren;
		bool					m_Open = false;

		rString					m_OnOpenScript;
		rString					m_OnCloseScript;
		
		SpriteDefinition		m_Background;
		glm::vec4				m_BackgroundColour;
		glm::vec4				m_BorderColour;
		bool					m_Border = false;
		
		bool					m_Moveable = false;
		bool					m_Moving = false;
		bool					m_ClickThrough = false;
		glm::ivec2				m_ClickPos;
		
		
		rString					m_ToggleGroup;
		bool					m_InGroup = false;
		
		int						m_ScrollOffset = 0;
		bool					m_Scrollable = false;
		
		bool					m_CullContent = false;


		glm::ivec2				m_SlideStart;
		glm::ivec2				m_SlideEnd;
		bool					m_SlideOpen = false;
		bool					m_SlideClose = false;
		glm::vec2				m_SlidePos;

		int						m_SlideSpeed;
	};
}
