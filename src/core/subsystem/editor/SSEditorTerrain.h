/**************************************************
2015 Mattias Willemsen
***************************************************/
#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include "../../datadriven/ComponentTypes.h"
#include <messaging/Subscriber.h>


#define g_SSEditorTerrain SSEditorTerrain::GetInstance()

class SSEditorTerrain : public Subsystem, public Subscriber
{
	// public
public:
	static SSEditorTerrain&		GetInstance(void);

	void						Startup(void) override;
	void						Shutdown(void) override;
	void						UpdateUserLayer(const float deltaTime) override;
	void						UpdateSimLayer(const float timeStep) override;

	// GUI
	void						ScriptWindowOnOpen(void);
	void						ScriptWindowOnClose(void);
	void						ScriptOpenTerrainWindow(void);

	// private
private:
	SSEditorTerrain() : Subsystem("EditorTerrain"), Subscriber("EditorTerrain") {}
	void InitializeGUI(void);
	void InitializeDecal(void);
	void SetDecalToPosition(const glm::vec3& position);
	void HandleMouseClick(bool isLeftClick, float deltaTime);

	// members
	float m_BrushStrength = 1.0f;
	float m_BrushRadius = 10.0f;
	float m_BrushHardness = 0.01f;
	float m_MaxTimeOut = 0.1f;
	float m_CurrTimeOut = 0.0f;
	bool m_OnTimeOut = false;
	int m_LastMousePosX = -100000;
	int m_LastMousePosY = -100000;
	float m_LastPickedX = 0.0f;
	float m_LastPickedZ = 0.0f;
	Entity m_DecalEntity = -1;

	// GUI stuff
	GUI::Window*				m_WindowMain;
	GUI::Window*				m_BrushToolbox;
	GUI::Window*				m_BrushWindow;
	GUI::Slider*				m_BrushSizeSlider;
	GUI::Text*					m_BrushSizeSliderValue;
	GUI::Slider*				m_BrushStrengthSlider;
	GUI::Text*					m_BrushStrengthSliderValue;
	GUI::Slider*				m_BrushHardnessSlider;
	GUI::Text*					m_BrushHardnessSliderValue;
	GUI::Button*				m_SelectBrushToolButton;


};