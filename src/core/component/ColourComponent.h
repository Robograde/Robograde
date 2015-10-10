/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include <glm/glm.hpp>



struct ColourComponent
{
	glm::vec4 Colour = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );

	ColourComponent( const glm::vec4& colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) ) :Colour( colour )
	{ }
};