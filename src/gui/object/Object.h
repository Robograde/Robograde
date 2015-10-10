/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include "../graphics2D/RenderDefinition.h"
#include "../graphics2D/Rectangle.h"

#ifdef _WIN32
	#ifdef GUI_DLL_EXPORT
		#define GUI_API __declspec(dllexport) // Exports symbols
	#else
		#define GUI_API __declspec(dllimport) // Imports symbols
	#endif
#else
	#define GUI_API // Unix is not annoying :D
#endif

namespace GUI
{
	class Object
	{
	public:
									Object();
									Object( const rString& name, const rString& parent, Rectangle boundingbox );
									
		virtual						~Object();
		
		virtual void				Initialize( ) = 0 ;
		virtual void 				Update( float deltaTime, glm::ivec2 parentPos ) = 0;
		virtual void 				Render( glm::ivec2 parentPos ) = 0;
		virtual void				OnWindowOpen();
		virtual void				OnWindowClose();

		virtual void				SetOrigin( int x, int y );
		GUI_API virtual void		SetPosition( int x, int y );
		GUI_API virtual void		SetSize( int width, int height );

		GUI_API virtual glm::ivec2	GetPosition( );
		GUI_API virtual glm::ivec2	GetSize( );
							
		virtual void				OnFocus();
		
		rString						GetName() { return rString( m_Name.c_str() ); }
		const Rectangle& 			GetBoundingBoxRef() { return m_BoundingBox; }
		bool						GetBringToFront();
		GUI_API void				BringToFront();
		

		GUI_API void				SetOpacity( float opacity ) { m_Opacity = opacity; }
		void						SetEnabled( bool enabled ) { m_Enabled = enabled; }
		void						SetVisible( bool visible ) { m_Visible = visible; }
		void						SetCulled( bool culled ){ m_Culled = culled; }
		bool						GetCulled() { return m_Culled; }
		
		//Used to find Window Objects
		bool						IsWindow() { return m_IsWindow; } 
		
		bool						IsEnabled() { return m_Enabled; }
		bool						IsVisible() { return m_Visible; }
		
	protected:
		rString						m_Name;
		rString						m_Parent;
		Rectangle 					m_BoundingBox;
		
		bool						m_Focused = false;
		bool						m_Enabled = true;
		bool						m_Visible = true;
		bool						m_IsWindow = false;
		bool						m_BringToFront = false;
		
		bool						m_Culled = false;

		float						m_Opacity = 1.0f;
	};
}
