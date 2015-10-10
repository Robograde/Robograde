#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSColourPicker SSColourPicker::GetInstance( )

class SSColourPicker : public Subsystem
{
public:

	static SSColourPicker& GetInstance();

	void Startup() override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown() override;

	void Open( glm::ivec2 position, GUI::Button* colourResultButton );
	glm::vec4& GetColour();

private:
	// No external instancing allowed
	SSColourPicker() : Subsystem( "ColourPicker" ) {}
	SSColourPicker( const SSColourPicker& rhs );
	~SSColourPicker() {};
	SSColourPicker& operator=( const SSColourPicker& rhs );


	const rString m_WindowName = "ColourPickerWindow";
	GUI::Window* m_Window;

	GUI::Sprite* m_HVPicker;
	GUI::Sprite* m_SPicker;
	GUI::Sprite* m_HVMarker;
	GUI::Sprite* m_SMarker;
	GUI::Text* m_R;
	GUI::Text* m_G;
	GUI::Text* m_B;
	GUI::Text* m_RGB;
	int m_HVSize = 256;

	glm::vec4 m_Colour;
	int m_H;
	float m_S;
	float m_V;

	GUI::Button* m_ColourResult = nullptr;
};
