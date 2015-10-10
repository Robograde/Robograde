/**************************************************
2015 Jens Stjernkvist
***************************************************/

#include "SSEditorSFXEmitter.h"

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
#include "../../subsystem/audio/SSSFXEmitter.h"
#include "../utility/SSMail.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/Catalogue.h"
#include "../gamelogic/SSCamera.h"

#define LOOP_INTERVAL_MIN			 0.0f
#define LOOP_INTERVAL_MAX			30.0f
#define LOOP_INTERVAL_MAX_OFFSET	 0.1f
#define DEFAULT_NAME				"noName"
#define DEFAULT_MODEL_PATH			"projectileCone.robo"
#define SOUND_PATH_ROOT				"../../../asset/audio/sfx/custom/"
#define SFX_MAX_INSTANCES			1 //Limit to 1 so looping sound wont overlap

#define CIRCULAR_SLIDER_COLOR_MIN	glm::vec4(1.0f, 0.0f, 0.0f, 0.2f)
#define CIRCULAR_SLIDER_COLOR_MAX	glm::vec4(0.0f, 1.0f, 0.0f, 0.2f)

//+----------------------------------------------------------------------------+
//|SSEditorSFXEmitter& GetInstance(void)
//\----------------------------------------------------------------------------+
SSEditorSFXEmitter& SSEditorSFXEmitter::GetInstance(void)
{
	static SSEditorSFXEmitter instance;
	return instance;
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSEditorSFXEmitter::~SSEditorSFXEmitter()
{
}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::Startup(void)
{
	Subsystem::Startup();
	m_SimInterests = MessageTypes::EDITOR_SFXEMITTER;
	g_SSMail.RegisterSubscriber(this);

	InitializeGUI();
	UpdateAudioFileCollection(SOUND_PATH_ROOT);

	//Register functions for gui buttons
	g_Script.Register("EDITOR_TogglePlaySound",						[this](IScriptEngine*)->int{ ScriptTogglePlaySound();					return 0;});
	g_Script.Register("EDITOR_SFXEmitterOnOpen",					[this](IScriptEngine*)->int{ScriptWindowOnOpen();						return 0;});
	g_Script.Register("EDITOR_SFXEmitterOnClose",					[this](IScriptEngine*)->int{ScriptWindowOnClose();						return 0;});
	g_Script.Register("EDITOR_SFXEmitterCatalogueButtonPressed",	[&](IScriptEngine*){ScriptCatalogueButtonPressed(g_Script.PopInt());	return 0;});
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::Shutdown(void)
{
	Subsystem::Shutdown();
	m_Emitters.clear();
	pDelete(m_Catalogue);
}
//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateUserLayer(const float deltaTime)
{
	Subsystem::UpdateUserLayer(deltaTime);
	
	if(g_GUI.IsWindowOpen(m_WindowMain->GetName()))
	{
		UpdateGUI();
		HandleInput();
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateSimLayer(const float timeStep)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateSimLayer(const float timeStep)
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
SSEditorSFXEmitter::SSEditorSFXEmitter()
	:	Subsystem("EditorSFXEmitter")
	,	Subscriber("EditorSFXEmitter")
	,	m_PlaySound(false)
	,	m_SelectedEmitter(ENTITY_INVALID)
{
}
//+----------------------------------------------------------------------------+
//|void InitializeGUI(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::InitializeGUI(void)
{
	//Create child windows
	m_WindowMain = g_GUI.GetWindow("SoundEmittersWindow");
	m_WindowMain->SetOnOpenScript("EDITOR_SFXEmitterOnOpen()");
	m_WindowMain->SetOnCloseScript("EDITOR_SFXEmitterOnClose()");

	glm::ivec2 windowSize = m_WindowMain->GetSize();

	///Creator mode window
	m_WindowCreator = g_GUI.AddWindow("SFXEmitterWindowCreator", GUI::Rectangle(0, 0, windowSize.x, windowSize.y), m_WindowMain->GetName());

	///Modify mode window
	m_WindowModify = g_GUI.AddWindow("SFXEmitterWindowModify", GUI::Rectangle(0, 0, windowSize.x, windowSize.y), m_WindowMain->GetName());

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
 	m_Catalogue = pNew(Catalogue, "SFXEmitterCatalogue", GUI::Rectangle(posOriginX + x, posOriginY + y, m_WindowMain->GetBoundingBoxRef().GetSize().x, m_WindowMain->GetBoundingBoxRef().GetSize().y), "SFXEmitterWindowCreator", "EDITOR_SFXEmitterCatalogueButtonPressed");

	///Modify - mode
	name = m_WindowModify->GetName();
	///Name
	x = 0;
	y = 0;
	label = g_GUI.AddText("", GUI::TextDefinition("Name ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel ), name);
	label->SetTextAlignment(GUI::ALIGNMENT_BOTTOM_LEFT);
	y += heightLabel + offsetLabelY;
	m_TextBoxName = g_GUI.AddTextBox("", GUI::Rectangle(posOriginX + x, posOriginY + y, widthTextbox, heightTextbox ), name);
	m_TextBoxName->SetText(DEFAULT_NAME);

	///Distance min
	x = 0;
	y = 40;
	label = g_GUI.AddText("", GUI::TextDefinition("Distance Min ", posOriginX + x, posOriginY + y, 400, heightLabel ), name);
	label->SetTextAlignment(GUI::ALIGNMENT_BOTTOM_LEFT);
	y += heightLabel + offsetLabelY;
	m_TextBoxDistMin = g_GUI.AddTextBox("", GUI::Rectangle(posOriginX + x, posOriginY + y, 128, heightTextbox ), name);
	m_TextBoxDistMin->SetText("10.0");

	///Distance max
	x = 150;
	y = 40;
	label = g_GUI.AddText("", GUI::TextDefinition("Distance Max ", posOriginX + x, posOriginY + y, 400, heightLabel ), name);
	label->SetTextAlignment(GUI::ALIGNMENT_BOTTOM_LEFT);
	y += heightLabel + offsetLabelY;
	m_TextBoxDistMax = g_GUI.AddTextBox("", GUI::Rectangle(posOriginX + x, posOriginY + y, 128, heightTextbox ), name);
	m_TextBoxDistMax->SetText("100.0");

	///Loop interval
	x = 0;
	y = 100;
	label = g_GUI.AddText("", GUI::TextDefinition("Loop interval ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel), name);
	label->SetTextAlignment(GUI::ALIGNMENT_TOP_LEFT);
	x = 6;
	y += heightLabel + offsetLabelY - 16;
	m_SliderLoopInterval = g_GUI.AddSlider("", GUI::Rectangle(posOriginX + x, posOriginY + y, widthTextbox, heightTextbox), name);
	m_SliderLoopInterval->SetBackgroundImage("Button_256x16.png");
	m_SliderLoopInterval->SetSliderImage("Button_32.png");
	m_SliderLoopInterval->SetMaxValue(LOOP_INTERVAL_MAX + LOOP_INTERVAL_MAX_OFFSET);
	m_SliderLoopInterval->SetValue(LOOP_INTERVAL_MIN);
	x = m_SliderLoopInterval->GetPosition().x + m_SliderLoopInterval->GetSize().x + 6;
	m_TextLoopIntervalValue = g_GUI.AddText("", GUI::TextDefinition("", posOriginX + x, posOriginY + y), name);

	///File
	x = 350;
	y = 0;
	label = g_GUI.AddText("", GUI::TextDefinition("File ", posOriginX + x, posOriginY + y, 600, heightLabel), name);
	label->SetTextAlignment(GUI::ALIGNMENT_BOTTOM_LEFT);
	y += heightLabel + offsetLabelY;
	m_ComboBoxFilePath = g_GUI.AddComboBox("", GUI::Rectangle(posOriginX + x, posOriginY + y, 600, heightTextbox), name);
	m_ComboBoxFilePath->SetBackgroundImage("Button_128x32.png");
	m_ComboBoxFilePath->SetItemBackgroundImage("Button_128x32.png");

	///Play - Stop
	x = g_GUI.GetWindowSize(name).x - 75;
	y = 32;
	m_ButtonPlay_Stop = g_GUI.AddButton("", GUI::Rectangle(posOriginX + x, posOriginY + y, 64, 64), name);
	m_ButtonPlay_Stop->SetBackgroundImage("Button.png");
	m_ButtonPlay_Stop->SetText("Play");
	m_ButtonPlay_Stop->SetClickScript("EDITOR_TogglePlaySound()");

	///Min - max circle sliders
	m_CircularSliderDistMin = g_GUI.AddCircularSlider("", glm::ivec2(200, 200), "EditorCanvasWindow");
	m_CircularSliderDistMax = g_GUI.AddCircularSlider("", glm::ivec2(400, 400), "EditorCanvasWindow");

	m_CircularSliderDistMin->SetRadius(32);
	m_CircularSliderDistMax->SetRadius(32);

	m_CircularSliderDistMin->SetColour(CIRCULAR_SLIDER_COLOR_MIN);
	m_CircularSliderDistMax->SetColour(CIRCULAR_SLIDER_COLOR_MAX);

	m_CircularSliderDistMax->SetVisible(false);
	m_CircularSliderDistMin->SetVisible(false);
}
//+----------------------------------------------------------------------------+
//|void HandleInput(void)
//|Handle input depending on the active mode
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::HandleInput(void)
{
	///Set mode depending on tool
	switch(g_SSEditorToolbox.GetSelectedTool())
	{
		///Creating
		case Tool::PlaceSFXEmitter:
		{
			if(g_Input->MouseDown(MOUSE_BUTTON_LEFT))
			{
				const glm::vec3& position = g_SSEditorToolbox.GetPickedPosition();
				if(position != INVALID_PICKED_POSITON)
				{
					SendEmitterActionMessageCreate(position);
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

			UpdateCircularSliderInput();
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
void SSEditorSFXEmitter::HandleMessage(const Message* &message)
{
	switch(message->Type)
	{
		case MessageTypes::EDITOR_SFXEMITTER:
		{
			const EditorSFXEmitterMessage* sfxMessage = static_cast<const EditorSFXEmitterMessage*>(message);

			switch(sfxMessage->Type)
			{
				case EDITORSFX_CREATE:
				{
					Entity emitter = CreateEmitter(sfxMessage->Pos, sfxMessage->Name, sfxMessage->Path, sfxMessage->TimeInterval, sfxMessage->DistanceMin, sfxMessage->DistanceMax);

					//Auto select if the creating player is the same is this client
					if(sfxMessage->PlayerId == g_PlayerData.GetPlayerID())
						SelectEmitter(emitter);
				}
				break;

				case EDITORSFX_DELETE:
				{
					DeleteEmitter(sfxMessage->EntityID);
				}
				break;

				case EDITORSFX_MODIFY:
				{
					UpdateEmitterFromMessage(sfxMessage);
				}
				break;
			};
		} 
		break;
		default:
		{
			Logger::Log("Received unknown message type", "EditorSFXEmitter", LogSeverity::WARNING_MSG);
		}
		break;
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateGUI(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateGUI(void)
{
	switch(g_SSEditorToolbox.GetSelectedTool())
	{
		case Tool::MoveTool:
		{
			///Slider text update
			if(m_SliderLoopInterval->GetValue() > LOOP_INTERVAL_MAX)
			{
				m_TextLoopIntervalValue->GetTextDefinitionRef().Text = "Play once";
			}
			else
			{
				m_TextLoopIntervalValue->GetTextDefinitionRef().Text = rToString(m_SliderLoopInterval->GetValue());
			}

			///Save emitter values every frame in case something changes
			SaveSelected();
		}
		break;
	};
}
//+----------------------------------------------------------------------------+
//|void UpdateCircularSliderInput(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateCircularSliderInput(void)
{
	if(m_SelectedEmitter != ENTITY_INVALID)
	{
		m_CircularSliderDistMax->SetVisible(true);
		m_CircularSliderDistMin->SetVisible(true);
		SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(m_SelectedEmitter);
		if(sfxEmitter)
		{
			PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>(m_SelectedEmitter);
			const glm::vec3& position =	placementComponent->Position;
			SFXTrigger& trigger = sfxEmitter->SFXTriggers[0]; ///Assume there wont be an emitter without a trigger

			if(m_CircularSliderDistMin->GetChanged())
			{
				glm::ivec2 pickPos = m_CircularSliderDistMin->GetPosition() + glm::ivec2(m_CircularSliderDistMin->GetRadius(), 0);
				//TODOIA: Need to do some trignomety-fu here to convert back to word coords from screen coords
			}

			glm::ivec2 screenPos = g_SSEditorToolbox.WorldToScreen(position);
			m_CircularSliderDistMin->SetPosition(screenPos.x, screenPos.y);
			m_CircularSliderDistMax->SetPosition(screenPos.x, screenPos.y);

			glm::vec3 cameraRight = g_SSCamera.GetActiveCamera()->GetRight();

			glm::vec3 minPosition = position + cameraRight * trigger.DistanceMin;
			glm::vec3 maxPosition = position + cameraRight * trigger.DistanceMax;

			glm::ivec2 minScreenPos = g_SSEditorToolbox.WorldToScreen(minPosition);
			glm::ivec2 maxScreenPos = g_SSEditorToolbox.WorldToScreen(maxPosition);

			int minRadius = static_cast<int>(glm::length(glm::vec2(minScreenPos - screenPos)));
			int maxRadius = static_cast<int>(glm::length(glm::vec2(maxScreenPos - screenPos)));

			m_CircularSliderDistMin->SetMaxRadius(maxRadius - 32);
			m_CircularSliderDistMax->SetMinRadius(minRadius + 32);

			m_CircularSliderDistMax->SetRadius(maxRadius);
			m_CircularSliderDistMin->SetRadius(minRadius);
		}
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateEmitterFromGUI(Entity emitter)
//|Update the input emitter with the values obtained from the GUI
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateEmitterFromGUI(Entity emitter)
{
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(emitter);
	SFXTrigger &trigger = sfxEmitter->SFXTriggers[0];

	trigger.Name = m_TextBoxName->GetText();
	trigger.Path = SOUND_PATH_ROOT + m_ComboBoxFilePath->GetItem(m_ComboBoxFilePath->GetSelectedIndex()).Text.Text;
	trigger.DistanceMin = static_cast<float>(atof(m_TextBoxDistMin->GetText().c_str()));
	trigger.DistanceMax = static_cast<float>(atof(m_TextBoxDistMax->GetText().c_str()));

	if(m_SliderLoopInterval->GetValue() > LOOP_INTERVAL_MAX)
	{
		trigger.Looping = false;
		trigger.TimeInterval = 0.0f;
	}
	else
	{
		trigger.Looping = true;
		trigger.TimeInterval = m_SliderLoopInterval->GetValue();
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateEmitterFromMessage(const EditorSFXEmitterMessage* &message)
//|Update the an emitter with the values obtained from the input message
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateEmitterFromMessage(const EditorSFXEmitterMessage* &message)
{
	Entity emitter = message->EntityID;
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(emitter);
	SFXTrigger &trigger = sfxEmitter->SFXTriggers[0];

	trigger.Name = message->Name;
	trigger.Path = message->Path;
	trigger.DistanceMin = message->DistanceMin;
	trigger.DistanceMax = message->DistanceMax;
	trigger.TimeInterval = message->TimeInterval;

	///Update the GUI if the selected emitter was create
	if(emitter == m_SelectedEmitter)
	{
		UpdateGUIFromSelected();
	}
	else
	{
		UpdateComboboxFromPath(trigger.Path, m_ComboBoxFilePath);
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateAudioFileCollection(rString rootPath)
//|Read all the filenames contained in the input path
//|Create buttons in the catalogue for the create menu using the items loaded in the collection
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateAudioFileCollection(rString rootPath)
{
	m_AudioFiles.clear();
	//m_Catalogue->clear(); Request clear function
	m_ComboBoxFilePath->ClearItems();

	FileUtility::GetListOfContentInDirectory(rootPath.c_str(), m_AudioFiles);

	for(rVector<rString>::iterator it = m_AudioFiles.begin(); it != m_AudioFiles.end(); it++)
	{
		///Add to catalogue in create menu
		m_Catalogue->AddButton("AudioFile.png" ,glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ),"", (*it));

		///Add to combobox in modification menu
		rString completePath = rootPath + (*it);
		m_ComboBoxFilePath->AddItem((*it));

		if(!g_SSAudio.GetIsLoaded(completePath))
			g_SSAudio.LoadSFX(completePath, SFX_MAX_INSTANCES);
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateComboboxFromPath(const rString &path, GUI::ComboBox* &comboBox)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateComboboxFromPath(const rString &path, GUI::ComboBox* &comboBox)
{
	//Locate sfx file id in combobox by comparing the file paths
	for(int i = 0; i < comboBox->GetNumItems(); i++)
	{
		///Remove root path from file name path
		rString filename = path;
		rString guiName = comboBox->GetItem(i).Text.Text;
		std::string::size_type location = filename.find(guiName, 0);

		if(location == std::string::npos)
			continue;

		filename = filename.substr(location, filename.size());

		if(guiName == filename)
		{
			comboBox->SetSelectedIndex(i);
			return;
		}
	}

	Logger::Log("Error, rootpath to filename " + path + "was not found.", "SSEditorSFXEmitter", LogSeverity::WARNING_MSG);
}
//+----------------------------------------------------------------------------+
//|void SendEmitterActionMessageCreate(const glm::vec3& position)
//|Send a message to all clients to create an emitter, the actual emitter are created when the message is received
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SendEmitterActionMessageCreate(const glm::vec3 &position)
{
	///Only frame, playerID, position and type are relevant here.
	//TODO: Use a specific message for creation in the future?
	EditorSFXEmitterMessage message;
	message.ExecutionFrame = g_GameData.GetFrameCount() + 1;
	message.EntityID = ENTITY_INVALID;
	message.PlayerId = g_PlayerData.GetPlayerID();
	message.Type = static_cast<short>(EditorSFXMessage::EDITORSFX_CREATE);
	message.Pos = position;
	message.Name = "";
	message.Path = "";
	message.DistanceMin = 0.0f;
	message.DistanceMax = 0.0f;
	message.TimeInterval = 0.0f;
	g_SSMail.PushToNextFrame(message);
}
//+----------------------------------------------------------------------------+
//|void SendEmitterActionMessageModify(Entity emitter)
//|Send a sync message to all clients to update theier version of the input emitter
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SendEmitterActionMessageModify(Entity emitter)
{
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(emitter);
	SFXTrigger &trigger = sfxEmitter->SFXTriggers[0];

	EditorSFXEmitterMessage message;
	message.ExecutionFrame = g_GameData.GetFrameCount() + 1;
	message.EntityID = emitter;
	message.PlayerId = g_PlayerData.GetPlayerID();
	message.Type = static_cast<short>(EditorSFXMessage::EDITORSFX_MODIFY);
	message.Name = trigger.Name;
	message.Path = trigger.Path;
	message.DistanceMin = trigger.DistanceMin;
	message.DistanceMax = trigger.DistanceMax;
	message.TimeInterval = trigger.TimeInterval;
	g_SSMail.PushToNextFrame(message);
}
//+----------------------------------------------------------------------------+
//|void SendEmitterActionMessageDelete(Entity emitter)
//|Send a sync message to all clients to delete the input emitter
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SendEmitterActionMessageDelete(Entity emitter)
{
	///Only frame, playerID, type are relevant here.
	//TODO: Use a specific message for deleting emitters in the future?
	EditorSFXEmitterMessage message;
	message.ExecutionFrame = g_GameData.GetFrameCount() + 1;
	message.EntityID = emitter;
	message.PlayerId = g_PlayerData.GetPlayerID();
	message.Type = static_cast<short>(EditorSFXMessage::EDITORSFX_DELETE);
	message.Pos = glm::vec3(-1.0f);
	message.Name = "";
	message.Path = "";
	message.DistanceMin = 0.0f;
	message.DistanceMax = 0.0f;
	message.TimeInterval = 0.0f;
	g_SSMail.PushToNextFrame(message);
}
//+----------------------------------------------------------------------------+
//|void TogglePlaySound(Entity emitter)
//|Toggle the playpack sfx of the input emitter
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::TogglePlaySound(Entity emitter)
{
	m_PlaySound = !m_PlaySound;

	if(m_PlaySound)
		m_ButtonPlay_Stop->SetText("Stop");
	else
		m_ButtonPlay_Stop->SetText("Play");

	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(emitter);
	SFXTrigger &trigger = sfxEmitter->SFXTriggers[0];

	if(sfxEmitter)
	{
		if(m_PlaySound)
		{
			trigger.Timer = 0.0f;
			trigger.Triggered = true;
			g_SSSFXEmitter.AddEvent(sfxEmitter, SFXTriggerType::AT_CREATION);
		}
		else
		{
			StopSound(trigger);
		}
	}
}
//+----------------------------------------------------------------------------+
//|void StopSound(SFXTrigger &trigger)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::StopSound(SFXTrigger &trigger)
{
	trigger.Timer = 0.0f;
	trigger.Triggered = false;
	g_SSAudio.SetSFXStop(trigger.SFXHandle);
}
//+----------------------------------------------------------------------------+
//|Entity TrySelect(void)
//\----------------------------------------------------------------------------+
Entity SSEditorSFXEmitter::TrySelect(void)
{
	Entity selectedEntity = g_SSEditorToolbox.GetMouseOverEntity();
	if ( selectedEntity != ENTITY_INVALID )
	{
		EntityMask entityMask = EntityManager::GetInstance().GetEntityMask( selectedEntity );
		if ( !( ( entityMask & GetDenseComponentFlag<SFXEmitterComponent>() ) == GetDenseComponentFlag<SFXEmitterComponent>() ) )
			selectedEntity = ENTITY_INVALID;
	}
		
	return selectedEntity;
}
//+----------------------------------------------------------------------------+
//|void SelectEmitter(Entity emitter)
//|Mark the input emitter as the selected emitter that will have its values updated by the GUI
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SelectEmitter(Entity emitter)
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
void SSEditorSFXEmitter::Unselect(void)
{
	///Stop playing sound from selected emitter
	if(m_PlaySound)
		TogglePlaySound(m_SelectedEmitter);

	m_SelectedEmitter = ENTITY_INVALID;

	SetModeSelect();

	m_CircularSliderDistMax->SetVisible(false);
	m_CircularSliderDistMin->SetVisible(false);
}
//+----------------------------------------------------------------------------+
//|void UpdateGUIFromSelected(void)
//|Read all the data from the selected SFXEmitter and put it in the GUI
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::UpdateGUIFromSelected(void)
{
	if(m_SelectedEmitter != ENTITY_INVALID)
	{
		SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(m_SelectedEmitter);

		if(sfxEmitter)
		{
			SFXTrigger trigger = sfxEmitter->SFXTriggers[0]; ///Assume there wont be an emitter without a trigger
			m_TextBoxName->SetText(trigger.Name);
			m_TextBoxDistMin->SetText(rToString(trigger.DistanceMin));
			m_TextBoxDistMax->SetText(rToString(trigger.DistanceMax));
			m_SliderLoopInterval->SetValue(trigger.TimeInterval);
			
			UpdateComboboxFromPath(trigger.Path, m_ComboBoxFilePath);
		}
	}
}
//+----------------------------------------------------------------------------+
//|void SetModeCreate(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SetModeCreate(void)
{
	g_SSEditorToolbox.SelectTool(Tool::PlaceSFXEmitter);
	m_WindowModify->Close();
	m_WindowCreator->Open();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(true);
}
//+----------------------------------------------------------------------------+
//|void SetModeModify(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SetModeModify(void)
{
	g_SSEditorToolbox.SelectTool(Tool::MoveTool);
	m_WindowModify->Open();
	m_WindowCreator->Close();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);
}
//+----------------------------------------------------------------------------+
//|void SetModeSelect(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::SetModeSelect(void)
{
	g_SSEditorToolbox.SelectTool(Tool::SelectionTool);
	m_WindowModify->Close();
	m_WindowCreator->Open();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);
}
//+----------------------------------------------------------------------------+
//|Entity CreateEmitter(const glm::vec3 pos, const rString &name, const rString &path, const float timeInterval, const float distanceMin, const float distanceMax)
//\----------------------------------------------------------------------------+
Entity SSEditorSFXEmitter::CreateEmitter(const glm::vec3 pos, const rString &name, const rString &path, const float timeInterval, const float distanceMin, const float distanceMax)
{
	///Create emitter with some default values
	Entity emitter = EntityFactory::CreateSoundEmitterObject(pos, path, distanceMin, distanceMax, true, timeInterval, true);
	UpdateEmitterFromGUI(emitter);

	m_Emitters.push_back(emitter);

	return emitter;
}
//+----------------------------------------------------------------------------+
//|void DeleteEmitter(Entity emitter)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::DeleteEmitter(Entity emitter)
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
void SSEditorSFXEmitter::SaveSelected(void)
{
	if(m_SelectedEmitter != ENTITY_INVALID)
		UpdateEmitterFromGUI(m_SelectedEmitter);
}
//+----------------------------------------------------------------------------+
//|void ScriptTogglePlaySound(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::ScriptTogglePlaySound(void)
{
	TogglePlaySound(m_SelectedEmitter);
}
//+----------------------------------------------------------------------------+
//|void ScriptWindowOnOpen(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::ScriptWindowOnOpen(void)
{
	m_WindowCreator->Open();
	g_SSEditorToolbox.SetLocalPlacementGhostModel(DEFAULT_MODEL_PATH);
	SetModeSelect();
}
//+----------------------------------------------------------------------------+
//|void ScriptWindowOnClose(void)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::ScriptWindowOnClose(void)
{
	m_WindowModify->Close();
	m_WindowCreator->Close();
	g_SSEditorToolbox.SetLocalPlacementGhostVisible(false);

	m_CircularSliderDistMax->SetVisible(false);
	m_CircularSliderDistMin->SetVisible(false);
}
//+----------------------------------------------------------------------------+
//|void ScriptCatalogueButtonPressed(int buttonIndex)
//\----------------------------------------------------------------------------+
void SSEditorSFXEmitter::ScriptCatalogueButtonPressed(int buttonIndex)
{
	SetModeCreate();
}