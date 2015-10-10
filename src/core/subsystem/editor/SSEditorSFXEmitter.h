/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Subsystem.h"

#include <gui/GUIEngine.h>
#include <messaging/Subscriber.h>

#include "../../datadriven/ComponentTypes.h"
#include "../../component/SFXEmitterComponent.h"
#include "../../input/GameMessages.h"

#define g_SSEditorSFXEmitter SSEditorSFXEmitter::GetInstance()

enum EditorSFXMessage : short
{
	EDITORSFX_CREATE	=	0,
	EDITORSFX_DELETE	=	1,
	EDITORSFX_MODIFY	=	2,
};

//+===+=========================+======================
//----| SSEditorSFXEmitter		|
//----+-------------------------+----------------------
//+===+================================================
class SSEditorSFXEmitter :
	public Subsystem,
	public Subscriber
{

//Public Functions	 
public:
	static SSEditorSFXEmitter&	GetInstance(void);
								~SSEditorSFXEmitter();

	void						Startup(void) override;
	void						Shutdown(void) override;

	void						UpdateUserLayer(const float deltaTime) override;
	void						UpdateSimLayer(const float timeStep) override;
	
//Private Functions
private:
								SSEditorSFXEmitter();
								SSEditorSFXEmitter(const SSEditorSFXEmitter &ref);
	SSEditorSFXEmitter&			operator=(const SSEditorSFXEmitter &ref);

	void						InitializeGUI(void);

	void						HandleInput(void);
	void						HandleMessage(const Message* &message);
	void						UpdateGUI(void);
	void						UpdateCircularSliderInput(void);
	void						UpdateEmitterFromGUI(Entity emitter);
	void						UpdateEmitterFromMessage(const EditorSFXEmitterMessage* &message);
	void						UpdateAudioFileCollection(rString rootPath);
	void						UpdateComboboxFromPath(const rString &path, GUI::ComboBox* &comboBox);

	void						SendEmitterActionMessageCreate(const glm::vec3 &position);
	void						SendEmitterActionMessageModify(Entity emitter);
	void						SendEmitterActionMessageDelete(Entity emitter);

	void						TogglePlaySound(Entity emitter);
	void						StopSound(SFXTrigger &trigger);

	Entity						TrySelect(void);
	void						SelectEmitter(Entity emitter);
	void						Unselect(void);
	void						UpdateGUIFromSelected(void);

	Entity						CreateEmitter(const glm::vec3 pos, const rString &name, const rString &path, const float timeInterval, const float distanceMin, const float distanceMax);
	void						DeleteEmitter(Entity emitter);
	void						SaveSelected(void);

	void						SetModeCreate(void);
	void						SetModeModify(void);
	void						SetModeSelect(void);

	void						ScriptTogglePlaySound();
	void						ScriptWindowOnOpen();
	void						ScriptWindowOnClose();
	void						ScriptCatalogueButtonPressed(int buttonIndex);
//Private Variables
private:
	GUI::Window*				m_WindowMain;
	GUI::Window*				m_WindowCreator;
	GUI::Window*				m_WindowModify;
	GUI::TextBox*				m_TextBoxName;
	GUI::ComboBox*				m_ComboBoxFilePath;
	GUI::TextBox*				m_TextBoxDistMin;		//Temp, replace with selectable sphere
	GUI::TextBox*				m_TextBoxDistMax;		//Temp, replace with selectable sphere
	GUI::CircularSlider*		m_CircularSliderDistMin;
	GUI::CircularSlider*		m_CircularSliderDistMax;
	GUI::Slider*				m_SliderLoopInterval;
	GUI::Text*					m_TextLoopIntervalValue;
	GUI::Button*				m_ButtonPlay_Stop;
	bool						m_PlaySound;

	class Catalogue*			m_Catalogue;

	rVector<Entity>				m_Emitters;
	rVector<rString>			m_AudioFiles;
	Entity						m_SelectedEmitter;
};