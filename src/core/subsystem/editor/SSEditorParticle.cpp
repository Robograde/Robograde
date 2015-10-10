/**************************************************
2015 Jens Stjernkvist
***************************************************/

#include "SSEditorParticle.h"

#include <input/Input.h>
#include <utility/Logger.h>
#include <utility/FileUtility.h>
#include <utility/Colours.h>

#include "SSEditorToolbox.h"
#include "../src/core/EntityFactory.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/PlacementComponent.h"
#include "../../component/ModelComponent.h"
#include "../../component/ColourComponent.h"
#include "../../component/ParticleEmitterComponent.h"
#include "../../subsystem/editor/SSColourPicker.h"
#include "../utility/SSMail.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/Catalogue.h"
#include "../../../utility/Randomizer.h"
#include "../gamelogic/SSCamera.h"


#define LOOP_INTERVAL_MIN				 0.0f
#define LOOP_INTERVAL_MAX				30.0f
#define LOOP_INTERVAL_MAX_OFFSET		 0.1f
#define DEFAULT_NAME					"noName"
#define DEFAULT_MODEL_PATH				"projectileCone.robo"
#define PARTICLE_START_APLHA			0.5f

static glm::vec4	g_Color =			glm::vec4(1.0f, 0.0f, 0.0f, PARTICLE_START_APLHA);
static short		g_SelectedType =	0;

//+----------------------------------------------------------------------------+
//|SSEditorParticle& GetInstance(void)
//\----------------------------------------------------------------------------+
SSEditorParticle& SSEditorParticle::GetInstance(void)
{
	static SSEditorParticle instance;
	return instance;
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSEditorParticle::~SSEditorParticle()
{
}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::Startup(void)
{
	Subsystem::Startup();
	m_SimInterests = MessageTypes::EDITOR_PARTICLE_EMITTER;
	g_SSMail.RegisterSubscriber(this);

	InitializeGUI();

	//Register functions for gui buttons
	g_Script.Register("EDITOR_ParticleEmitterOnOpen",			[this](IScriptEngine*)->int{ScriptWindowOnOpen();						return 0;});
	g_Script.Register("EDITOR_ParticleEmitterOnClose",			[this](IScriptEngine*)->int{ScriptWindowOnClose();						return 0;});
	g_Script.Register("EDITOR_ParticleCatalogueButtonPressed",	[&](IScriptEngine*){ScriptCatalogueButtonPressed(g_Script.PopInt());	return 0;});
	g_Script.Register("EDITOR_SelectColorStart",				[this](IScriptEngine*)->int{ScriptSelectColor();						return 0;});
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::Shutdown(void)
{ 
	Subsystem::Shutdown();
	m_Emitters.clear();
	pDelete(m_Catalogue);
}
//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSEditorParticle::UpdateUserLayer(const float deltaTime)
{
	Subsystem::UpdateUserLayer(deltaTime);
	
	if(g_GUI.IsWindowOpen(m_WindowMain->GetName()))
	{
		UpdateGUI();
		HandleInput();

		//Temporary permanent render
		if(m_SelectedEmitter != ENTITY_INVALID)
		{
			ScriptSelectColor();

			ParticleEmitterComponent* emitter = GetDenseComponent<ParticleEmitterComponent>(m_SelectedEmitter);

			if(emitter->ParticleType == PARTICLE_EMITTER_TYPE_SMOKE)
			{
				m_ParticleSmoke = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_SMOKE_TRAIL);
				m_ParticleSmoke.Colour = g_Color;
				m_ParticleSmoke.ParticlesSpeed = 5;
				m_ParticleSmoke.TimeToLive = 5.0f;
				m_ParticleSmoke.ParticlesTimeToLive = m_ParticleSmoke.TimeToLive;
				m_ParticleSmoke.EmitterPosition = GetDenseComponent<PlacementComponent>(m_SelectedEmitter)->Position;
				m_ParticleSmoke.Direction = glm::normalize(glm::vec3(static_cast<float>(g_Randomizer.UserRand(0, 1000)), static_cast<float>(g_Randomizer.UserRand(500, 1000)), static_cast<float>(g_Randomizer.UserRand(0, 1000))));
				g_SSParticle.SpawnParticleSystem(PARTICLE_TYPE_SMOKE_TRAIL, m_ParticleSmoke);
			}
			else if(emitter->ParticleType == PARTICLE_EMITTER_TYPE_FIRE)
			{
				m_ParticleFire = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_FIRE_BALL);
				m_ParticleFire.Colour = g_Color;
				m_ParticleFire.ParticlesSpeed = 5;
				m_ParticleFire.TimeToLive = 3.0f;
				m_ParticleFire.ParticlesTimeToLive = m_ParticleFire.TimeToLive;
				m_ParticleFire.EmitterPosition = GetDenseComponent<PlacementComponent>(m_SelectedEmitter)->Position;
				m_ParticleFire.Direction = glm::normalize(glm::vec3(static_cast<float>(g_Randomizer.UserRand(0, 500)), static_cast<float>(g_Randomizer.UserRand(600, 800)), static_cast<float>(g_Randomizer.UserRand(0, 500))));
				g_SSParticle.SpawnParticleSystem(PARTICLE_TYPE_FIRE_BALL, m_ParticleFire);
			}

		}
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateSimLayer(const float timeStep)
//\----------------------------------------------------------------------------+
void SSEditorParticle::UpdateSimLayer(const float timeStep)
{
	for(int i = 0; i < m_SimMailbox.size(); ++i)
	{
		const Message* message = m_SimMailbox[i];
		HandleMessage(message);
	}
}
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SSEditorParticle::SSEditorParticle()
	:	Subsystem("EditorParticleEmitter")
	,	Subscriber("EditorParticleEmitter")
	,	m_SelectedEmitter(ENTITY_INVALID)
{
}
//+----------------------------------------------------------------------------+
//|void InitializeGUI(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::InitializeGUI(void)
{
	//Create child windows
	m_WindowMain = g_GUI.GetWindow("ParticlesWindow");
	m_WindowMain->SetOnOpenScript("EDITOR_ParticleEmitterOnOpen()");
	m_WindowMain->SetOnCloseScript("EDITOR_ParticleEmitterOnClose()");

	glm::ivec2 windowSize = m_WindowMain->GetSize();

	///Creator mode window
	m_WindowCreator = g_GUI.AddWindow("ParticleEmitterWindowCreator", GUI::Rectangle(0, 0, windowSize.x, windowSize.y), m_WindowMain->GetName());

	///Modify mode window
	m_WindowModify = g_GUI.AddWindow("ParticleEmitterWindowModify", GUI::Rectangle(0, 0, windowSize.x, windowSize.y), m_WindowMain->GetName());

	g_GUI.UseFont(FONT_ID_LEKTON_11);

	const int posOriginX = 4;
	const int posOriginY = -16;
	int x = 0;
	int y = 0;
	const int heightLabel = 32;
	const int offsetLabelY = 2;

	const int widthTextbox = 319;
	const int heightTextbox = 16;

	GUI::Text* label;

	rString name;

	///Create - mode
	name = m_WindowCreator->GetName();

	///Create emitter
	x = 0;
	y = 32;
 	m_Catalogue = pNew(Catalogue, "ParticleEmitterCatalogue", GUI::Rectangle(posOriginX + x, posOriginY + y, m_WindowMain->GetBoundingBoxRef().GetSize().x, m_WindowMain->GetBoundingBoxRef().GetSize().y), name, "EDITOR_ParticleCatalogueButtonPressed");
	m_Catalogue->AddButton("Particles.png", glm::vec4( 0.1f, 0.1f, 0.1f, 1.0f ), "", "Smoke");
	m_Catalogue->AddButton("Particles.png", glm::vec4( 0.0f, 0.6f, 0.0f, 1.0f ), "", "Fire");

	///Modify - mode
	name = m_WindowModify->GetName();
	///Name
	x = 0;
	y = 0;
	label = g_GUI.AddText("", GUI::TextDefinition("Name ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel ), name);
	label->SetTextAlignment(GUI::ALIGNMENT_BOTTOM_LEFT);
	y += heightLabel + offsetLabelY;
	m_TextBoxName = g_GUI.AddTextBox("", GUI::Rectangle(posOriginX + x, posOriginY + y, widthTextbox, heightTextbox ), name);
	m_TextBoxName->SetText("noName");

	///Color
	x = 0;
	y = 50;
	label = g_GUI.AddText("", GUI::TextDefinition("Color ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel ), name);
	label->SetTextAlignment(GUI::ALIGNMENT_BOTTOM_LEFT);
	y += heightLabel + offsetLabelY;
	m_ButtonColor = g_GUI.AddButton("EditorParticleSelectColorStart", GUI::Rectangle(posOriginX + x, posOriginY + y, 100, 64), name);
	m_ButtonColor->SetClickScript("GE_OpenColourPicker( 'EditorParticleSelectColorStart', '" + name + "');");
}
//+----------------------------------------------------------------------------+
//|void HandleInput(void)
//|Handle input depending on the active mode
//\----------------------------------------------------------------------------+
void SSEditorParticle::HandleInput(void)
{
	///Set mode depending on tool
	switch(g_SSEditorToolbox.GetSelectedTool())
	{
		///Creating
		case Tool::PlaceParticleEmitter:
		{
			if(g_Input->MouseDown(MOUSE_BUTTON_LEFT))
			{
				const glm::vec3& position = g_SSEditorToolbox.GetPickedPosition();

				if(position != INVALID_PICKED_POSITON)
				{
					SendEmitterActionMessageCreate(position, g_SelectedType);
				}
			}
			else if(g_Input->MouseUpDown(MOUSE_BUTTON_RIGHT))
			{
				SetModeSelect();
			}
			break;
		}

		///Modify
		case Tool::MoveTool:
		{
			///Return to selection
			if(g_Input->MouseUpDown(MOUSE_BUTTON_RIGHT))
			{
				SendEmitterActionMessageModify(m_SelectedEmitter);
				Unselect();
			}

			///Keyboard
			if(g_Input->KeyUpDown(SDL_Scancode::SDL_SCANCODE_DELETE))
			{
				SendEmitterActionMessageDelete(m_SelectedEmitter);
			}
		}

		break;

		///Select
		case Tool::SelectionTool:
		{
			if(g_Input->MouseUpDown(MOUSE_BUTTON_LEFT))
			{
				Entity entity = TrySelect();

				if(entity != ENTITY_INVALID)
					SelectEmitter(entity);
			}
			break;
		}

		default:
		break;
	}
}
//+----------------------------------------------------------------------------+
//|void HandleMessage(const Message* &message)
//|Handle a message that was received
//\----------------------------------------------------------------------------+
void SSEditorParticle::HandleMessage(const Message* &message)
{
	switch(message->Type)
	{
		case MessageTypes::EDITOR_PARTICLE_EMITTER:
		{
			const EditorParticleEmitterMessage* emitterMessage = static_cast<const EditorParticleEmitterMessage*>(message);

			switch(emitterMessage->Type)
			{
				case EDITOR_EMITTER_CREATE:
				{
					Entity emitter = CreateEmitter(emitterMessage->Pos, emitterMessage->Name, emitterMessage->ParticleType);

					//Auto select if the creating player is the same is this client
					if(emitterMessage->PlayerId == g_PlayerData.GetPlayerID())
						SelectEmitter(emitter);
				}
				break;

				case EDITOR_EMITTER_DELETE:
				{
					DeleteEmitter(emitterMessage->EntityID);
				}
				break;

				case EDITOR_EMITTER_MODIFY:
				{
					UpdateEmitterFromMessage(emitterMessage);
				}
				break;
			};
		} 
		break;
		default:
		{
			Logger::Log("Received unknown message type", "EditorParticleEmitter", LogSeverity::WARNING_MSG);
		}
		break;
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateGUI(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::UpdateGUI(void)
{
	switch(g_SSEditorToolbox.GetSelectedTool())
	{
		case Tool::MoveTool:
		{
			///Save emitter values every frame in case something changes
			SaveSelected();
		}
		break;
	};
}
//+----------------------------------------------------------------------------+
//|void UpdateEmitterFromGUI(Entity emitter)
//|Update the input emitter with the values obtained from the GUI
//\----------------------------------------------------------------------------+
void SSEditorParticle::UpdateEmitterFromGUI(Entity emitter)
{
	//trigger.Name = m_TextBoxName->GetText();
}
//+----------------------------------------------------------------------------+
//|void UpdateEmitterFromMessage(const EditorParticleEmitterMessage* &message)
//|Update the an emitter with the values obtained from the input message
//\----------------------------------------------------------------------------+
void SSEditorParticle::UpdateEmitterFromMessage(const EditorParticleEmitterMessage* &message)
{
	ParticleEmitterComponent* particle = GetDenseComponent<ParticleEmitterComponent>(message->EntityID);
	particle->Name = message->Name;
	particle->Col = message->Col;
	particle->ParticleType = static_cast<ParticleEmitterType>(message->ParticleType);
}
//+----------------------------------------------------------------------------+
//|void SendEmitterActionMessageCreate(const glm::vec3& position)
//|Send a message to all clients to create an emitter, the actual emitter are created when the message is received
//\----------------------------------------------------------------------------+
void SSEditorParticle::SendEmitterActionMessageCreate(const glm::vec3 &position, short particleType)
{
	///Only frame, playerID, position and type are relevant here.
	//TODO: Use a specific message for creation in the future?
	EditorParticleEmitterMessage message;
	message.ExecutionFrame = g_GameData.GetFrameCount() + 1;
	message.EntityID = ENTITY_INVALID;
	message.PlayerId = g_PlayerData.GetPlayerID();
	message.ParticleType = particleType;
	message.Col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	message.Type = static_cast<short>(EditorEmitterMessage::EDITOR_EMITTER_CREATE);
	message.Pos = position;
	message.Name = "";
	g_SSMail.PushToNextFrame(message);
}
//+----------------------------------------------------------------------------+
//|void SendEmitterActionMessageModify(Entity emitter)
//|Send a sync message to all clients to update theier version of the input emitter
//\----------------------------------------------------------------------------+
void SSEditorParticle::SendEmitterActionMessageModify(Entity emitter)
{
	ParticleEmitterComponent* particle = GetDenseComponent<ParticleEmitterComponent>(emitter);
	EditorParticleEmitterMessage message;
	message.ExecutionFrame = g_GameData.GetFrameCount() + 1;
	message.EntityID = emitter;
	message.PlayerId = g_PlayerData.GetPlayerID();
	message.ParticleType = particle->ParticleType;
	message.Col = particle->Col;
	message.Type = static_cast<short>(EditorEmitterMessage::EDITOR_EMITTER_MODIFY);
	message.Name = particle->Name;
	g_SSMail.PushToNextFrame(message);
}
//+----------------------------------------------------------------------------+
//|void SendEmitterActionMessageDelete(Entity emitter)
//|Send a sync message to all clients to delete the input emitter
//\----------------------------------------------------------------------------+
void SSEditorParticle::SendEmitterActionMessageDelete(Entity emitter)
{
	///Only frame, playerID, type are relevant here.
	//TODO: Use a specific message for deleting emitters in the future?
	EditorParticleEmitterMessage message;
	message.ExecutionFrame = g_GameData.GetFrameCount() + 1;
	message.EntityID = emitter;
	message.PlayerId = g_PlayerData.GetPlayerID();
	message.ParticleType = 0;
	message.Col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	message.Type = static_cast<short>(EditorEmitterMessage::EDITOR_EMITTER_DELETE);
	message.Pos = glm::vec3(-1.0f);
	message.Name = "";
	g_SSMail.PushToNextFrame(message);
}
//+----------------------------------------------------------------------------+
//|Entity TrySelect(void)
//\----------------------------------------------------------------------------+
Entity SSEditorParticle::TrySelect( void )
{
	Entity selectedEntity = g_SSEditorToolbox.GetMouseOverEntity();
	if ( selectedEntity != ENTITY_INVALID )
	{
		EntityMask entityMask = EntityManager::GetInstance().GetEntityMask( selectedEntity );
		if ( !( ( entityMask & GetDenseComponentFlag<ParticleEmitterComponent>() ) == GetDenseComponentFlag<ParticleEmitterComponent>() ) )
			selectedEntity = ENTITY_INVALID;
	}

	return selectedEntity;
}
//+----------------------------------------------------------------------------+
//|void SelectEmitter(Entity emitter)
//|Mark the input emitter as the selected emitter that will have its values updated by the GUI
//\----------------------------------------------------------------------------+
void SSEditorParticle::SelectEmitter(Entity emitter)
{
	if(emitter != ENTITY_INVALID)
	{
		m_SelectedEmitter = emitter;
		g_SSEditorToolbox.SelectTool(Tool::MoveTool);
		m_WindowModify->Open();
		m_WindowCreator->Close();

		UpdateGUIFromSelected();

		g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);
	}
}
//+----------------------------------------------------------------------------+
//|void Unselect(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::Unselect(void)
{
	m_SelectedEmitter = ENTITY_INVALID;

	SetModeSelect();
}
//+----------------------------------------------------------------------------+
//|void UpdateGUIFromSelected(void)
//|Read all the data from the selected SFXEmitter and put it in the GUI
//\----------------------------------------------------------------------------+
void SSEditorParticle::UpdateGUIFromSelected(void)
{
	if(m_SelectedEmitter != ENTITY_INVALID)
	{

	}
}
//+----------------------------------------------------------------------------+
//|void SetModeCreate(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::SetModeCreate(void)
{
	g_SSEditorToolbox.SelectTool(Tool::PlaceParticleEmitter);
	m_WindowModify->Close();
	m_WindowCreator->Open();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(true);
}
//+----------------------------------------------------------------------------+
//|void SetModeModify(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::SetModeModify(void)
{
	g_SSEditorToolbox.SelectTool(Tool::MoveTool);
	m_WindowModify->Open();
	m_WindowCreator->Close();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);
}
//+----------------------------------------------------------------------------+
//|void SetModeSelect(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::SetModeSelect(void)
{
	g_SSEditorToolbox.SelectTool(Tool::SelectionTool);
	m_WindowModify->Close();
	m_WindowCreator->Open();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);
}
//+----------------------------------------------------------------------------+
//|Entity CreateEmitter(const glm::vec3 pos, const rString name, const short particleType)
//\----------------------------------------------------------------------------+
Entity SSEditorParticle::CreateEmitter(const glm::vec3 pos, const rString name, const short particleType)
{
	///Create emitter with some default values
	Entity emitter = EntityFactory::CreateParticleEmitterObject(pos, name, particleType, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	UpdateEmitterFromGUI(emitter);

	m_Emitters.push_back(emitter);

	return emitter;
}
//+----------------------------------------------------------------------------+
//|void DeleteEmitter(Entity emitter)
//\----------------------------------------------------------------------------+
void SSEditorParticle::DeleteEmitter(Entity emitter)
{
	if(emitter != ENTITY_INVALID)
	{
		for(rVector<Entity>::iterator it = m_Emitters.begin(); it != m_Emitters.end(); it++)
		{
			if((*it) == emitter)
			{
				if(m_SelectedEmitter == emitter) ///In case another client deletes it when selected
					Unselect();

				g_EntityManager.RemoveEntity((*it));
				m_Emitters.erase(it);
				return;
			}
		}
	}
}
//+----------------------------------------------------------------------------+
//|void SaveSelected(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::SaveSelected(void)
{
	if(m_SelectedEmitter != ENTITY_INVALID)
		UpdateEmitterFromGUI(m_SelectedEmitter);
}
//+----------------------------------------------------------------------------+
//|void ScriptWindowOnOpen(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::ScriptWindowOnOpen(void)
{
	m_WindowCreator->Open();
	g_SSEditorToolbox.SetLocalPlacementGhostModel(DEFAULT_MODEL_PATH);
	SetModeSelect();
}
//+----------------------------------------------------------------------------+
//|void ScriptWindowOnClose(void)
//\----------------------------------------------------------------------------+
void SSEditorParticle::ScriptWindowOnClose(void)
{
	m_WindowModify->Close();
	m_WindowCreator->Close();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);
}
//+----------------------------------------------------------------------------+
//|void ScriptCatalogueButtonPressed(int buttonIndex)
//\----------------------------------------------------------------------------+
void SSEditorParticle::ScriptCatalogueButtonPressed(int buttonIndex)
{
	SetModeCreate();
	g_SelectedType = buttonIndex;
}
//+----------------------------------------------------------------------------+
//|void ScriptSelectColor()
//\----------------------------------------------------------------------------+
void SSEditorParticle::ScriptSelectColor()
{
	g_Color = g_SSColourPicker.GetColour();
	g_Color.a = PARTICLE_START_APLHA;
}