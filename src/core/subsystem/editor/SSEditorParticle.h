/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Subsystem.h"

#include <gui/GUIEngine.h>
#include <messaging/Subscriber.h>

#include "../../datadriven/ComponentTypes.h"
#include "../../input/GameMessages.h"
#include "../../subsystem/gfx/SSParticle.h"

#define g_SSEditorParticle SSEditorParticle::GetInstance()

enum EditorEmitterMessage : short
{
	EDITOR_EMITTER_CREATE	=	0,
	EDITOR_EMITTER_DELETE	=	1,
	EDITOR_EMITTER_MODIFY	=	2,
};

//+===+=========================+======================
//----| SSEditorParticle		|
//----+-------------------------+----------------------
//+===+================================================
class SSEditorParticle :
	public Subsystem,
	public Subscriber
{

//Public Functions	 
public:
	static SSEditorParticle&	GetInstance(void);
								~SSEditorParticle();

	void						Startup(void) override;
	void						Shutdown(void) override;

	void						UpdateUserLayer(const float deltaTime) override;
	void						UpdateSimLayer(const float timeStep) override;
	
//Private Functions
private:
								SSEditorParticle();
								SSEditorParticle(const SSEditorParticle &ref);
	SSEditorParticle&			operator=(const SSEditorParticle &ref);

	void						InitializeGUI(void);

	void						HandleInput(void);
	void						HandleMessage(const Message* &message);
	void						UpdateGUI(void);
	void						UpdateEmitterFromGUI(Entity emitter);
	void						UpdateEmitterFromMessage(const EditorParticleEmitterMessage* &message);

	void						SendEmitterActionMessageCreate(const glm::vec3 &position, short particleType);
	void						SendEmitterActionMessageModify(Entity emitter);
	void						SendEmitterActionMessageDelete(Entity emitter);

	Entity						TrySelect(void);
	void						SelectEmitter(Entity emitter);
	void						Unselect(void);
	void						UpdateGUIFromSelected(void);

	Entity						CreateEmitter(const glm::vec3 pos, const rString name, const short particleType);
	void						DeleteEmitter(Entity emitter);
	void						SaveSelected(void);

	void						SetModeCreate(void);
	void						SetModeModify(void);
	void						SetModeSelect(void);

	void						ScriptWindowOnOpen();
	void						ScriptWindowOnClose();
	void						ScriptCatalogueButtonPressed(int buttonIndex);
	void						ScriptSelectColor();
//Private Variables
private:
	GUI::Window*				m_WindowMain;
	GUI::Window*				m_WindowCreator;
	GUI::Window*				m_WindowModify;
	GUI::TextBox*				m_TextBoxName;
	GUI::Button*				m_ButtonColor;

	class Catalogue*			m_Catalogue;

	rVector<Entity>				m_Emitters;
	Entity						m_SelectedEmitter;

	gfx::ParticleSystem			m_ParticleSmoke;
	gfx::ParticleSystem			m_ParticleFire;
};