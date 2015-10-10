/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Object.h"

namespace GUI
{
	Object::Object( )
	{
	}

	Object::Object( const rString& name, const rString& parent, Rectangle boundingbox )
	{
		m_Name = name.c_str();
		m_Parent = parent.c_str();
		m_BoundingBox = boundingbox;
	}
	
	Object::~Object()
	{
	}
	
	void Object::OnWindowOpen()
	{
	}
	
	void Object::OnWindowClose()
	{
	}
	
	void Object::OnFocus()
	{
	}
	
	bool Object::GetBringToFront()
	{
		bool bringToFront = m_BringToFront;
		m_BringToFront = false;
		return bringToFront;
	}
	
	void Object::BringToFront()
	{
		m_BringToFront = true;
	}
	
	void Object::Initialize()
	{
	}
	
	void Object::Update(float deltaTime , glm::ivec2 parentPos)
	{
	}
	
	void Object::Render( glm::ivec2 parentPos )
	{
	}

	void Object::SetOrigin( int x, int y )
	{
		m_BoundingBox.Origin = glm::ivec2( x, y );
	}
	
	void Object::SetPosition( int x, int y )
	{
		m_BoundingBox.X = x;
		m_BoundingBox.Y = y;
	}

	void Object::SetSize( int width, int height )
	{
		m_BoundingBox.Width = width;
		m_BoundingBox.Height = height;
	}

	glm::ivec2 Object::GetPosition( )
	{
		return m_BoundingBox.GetPosition();
	}

	glm::ivec2 Object::GetSize( )
	{
		return m_BoundingBox.GetSize();
	}




}
