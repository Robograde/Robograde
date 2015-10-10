#include "SSEditorTerrain.h"
#include <input/Input.h>
#include "SSEditorToolbox.h"
#include <gfx\GraphicsEngine.h>
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/DecalComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/TerrainFollowComponent.h"
#include "../../component/CollisionComponent.h"
#include <glm/gtx/transform.hpp>
#include <collisionDetection/ICollisionDetection.h>
#include "SSEditorToolbox.h"
#include "../utility/SSMail.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameData.h"
#include <utility\Logger.h>

SSEditorTerrain& SSEditorTerrain::GetInstance(void)
{
	static SSEditorTerrain instance;
	return instance;
}

void SSEditorTerrain::Startup(void)
{	
	
	// Set up the subscriber
	m_SimInterests = MessageTypes::EDITOR_TERRAIN_BRUSH;
	g_SSMail.RegisterSubscriber( this );

	Subsystem::Startup();
	InitializeGUI();
	InitializeDecal();
	
	// GUI script bindings
	g_Script.Register("EDITOR_TerrainOnOpen", [this](IScriptEngine*)->int{ScriptWindowOnOpen(); return 0; });
	g_Script.Register("EDITOR_TerrainOnClose", [this](IScriptEngine*)->int{ScriptWindowOnClose(); return 0; });
	g_Script.Register("EDITOR_TerrainOpenBrushWindow", [this](IScriptEngine*)->int{ScriptOpenTerrainWindow(); return 0; });

	g_SSEditorToolbox.SelectTool(Tool::TerrainEditTool);
}

void SSEditorTerrain::Shutdown(void)
{
	Subsystem::Shutdown();
}

void SSEditorTerrain::UpdateUserLayer(const float deltaTime)
{
	Subsystem::UpdateUserLayer(deltaTime);
	if (m_OnTimeOut)
	{
		m_CurrTimeOut -= deltaTime;
		if (m_CurrTimeOut < 0)
			m_OnTimeOut = false;
	}

	if (g_SSEditorToolbox.GetSelectedTool() != Tool::TerrainEditTool)
	{
		m_BrushWindow->Close();
		m_BrushToolbox->Open();
		DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
		decal->AlwaysDraw = false;
		SetDecalToPosition(glm::vec3(-100, 0, -100));
		return;
	}

	if (g_GUI.IsWindowOpen(m_WindowMain->GetName()))
	{	
		// Set position for decal
		const glm::vec3& position = g_SSEditorToolbox.GetPickedPosition();
		if (position != INVALID_PICKED_POSITON)
		{
			SetDecalToPosition(position);
			PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_DecalEntity);
			pc->Position = position;
		}

		// Set size for decal
		m_BrushRadius = m_BrushSizeSlider->GetValue();
		DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
		glm::mat4 w = glm::translate(vec3(position.x, position.y + 1.0f, position.z)) * glm::rotate(3.14f * 0.5f, vec3(1, 0, 0)) * glm::scale(vec3(decal->Scale));
		decal->Decal.World = w;
		decal->Scale = m_BrushRadius;
		m_BrushSizeSliderValue->SetText(rToString(m_BrushRadius));
		
		// Set brush strength
		m_BrushStrength = m_BrushStrengthSlider->GetValue();
		m_BrushStrengthSliderValue->SetText(rToString(m_BrushStrength));
		m_BrushHardness = m_BrushHardnessSlider->GetValue();
		m_BrushHardnessSliderValue->SetText(rToString(m_BrushHardness));

		if (g_Input->MouseDown(MOUSE_BUTTON_LEFT) && !m_OnTimeOut)
		{
			HandleMouseClick(true, deltaTime);
		}		
		else if (g_Input->MouseDown(MOUSE_BUTTON_RIGHT) && !m_OnTimeOut)
		{

			HandleMouseClick(false, deltaTime);
		}
	}
}

void SSEditorTerrain::UpdateSimLayer(const float timeStep)
{
	for (int i = 0; i < m_SimMailbox.size(); ++i)
	{
		const Message* message = m_SimMailbox[i];
		switch (m_SimMailbox[i]->Type)
		{
		case MessageTypes::EDITOR_TERRAIN_BRUSH:
		{
			const EditorTerrainBrushMessage* brushMessage = static_cast<const EditorTerrainBrushMessage*>(message);
			gfx::g_GFXTerrain.ApplyHeightbrush(brushMessage->Center, brushMessage->Strength, brushMessage->Hardness, brushMessage->Radius);
		} break;

		default:
			Logger::Log("Received unknown message type", "SSEditorTerrain", LogSeverity::WARNING_MSG);
			break;
		}
	}
}

void SSEditorTerrain::InitializeGUI(void)
{
	
	//Create child windows
	m_WindowMain = g_GUI.GetWindow("TerrainEditWindow");
	m_WindowMain->SetOnOpenScript("EDITOR_TerrainOnOpen()");
	m_WindowMain->SetOnCloseScript("EDITOR_TerrainOnClose()");

	m_BrushWindow = g_GUI.AddWindow("BrushWindow", GUI::Rectangle(0, 0, m_WindowMain->GetSize().x, m_WindowMain->GetSize().y), "TerrainEditWindow");
	m_BrushToolbox = g_GUI.AddWindow("BrushToolbox", GUI::Rectangle(0, 0, m_WindowMain->GetSize().x, m_WindowMain->GetSize().y), "TerrainEditWindow");

	glm::ivec2 windowSize = m_WindowMain->GetSize();

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

	// Main brush toolbox mode
	name = m_BrushToolbox->GetName();

	// Brush button
	x = 0;
	y = 32;
	m_SelectBrushToolButton = g_GUI.AddButton("", GUI::Rectangle(posOriginX + x, posOriginY + y, 64, 64), name);
	m_SelectBrushToolButton->SetBackgroundImage("Button.png");
	m_SelectBrushToolButton->SetText("Terrain \nBrush");
	m_SelectBrushToolButton->SetClickScript("EDITOR_TerrainOpenBrushWindow()");
	// Brush mode
	name = m_BrushWindow->GetName();

	// size slider
	x = 0;
	y = 24;
	label = g_GUI.AddText("", GUI::TextDefinition("Brush Size ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel), name);
	label->SetTextAlignment(GUI::ALIGNMENT_TOP_LEFT);
	x = 6;
	y += heightLabel + offsetLabelY - 16;
	m_BrushSizeSlider = g_GUI.AddSlider("", GUI::Rectangle(posOriginX + x, posOriginY + y, widthTextbox, heightTextbox), name);
	m_BrushSizeSlider->SetBackgroundImage("Button_256x16.png");
	m_BrushSizeSlider->SetSliderImage("Button_32.png");
	m_BrushSizeSlider->SetMaxValue(200);
	m_BrushSizeSlider->SetValue(10);
	x = m_BrushSizeSlider->GetPosition().x + m_BrushSizeSlider->GetSize().x + 6;
	m_BrushSizeSliderValue = g_GUI.AddText("", GUI::TextDefinition("", posOriginX + x, posOriginY + y), name);

	// strength slider
	x = 0;
	y = 64;
	label = g_GUI.AddText("", GUI::TextDefinition("Brush Strength ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel), name);
	label->SetTextAlignment(GUI::ALIGNMENT_TOP_LEFT);
	x = 6;
	y += heightLabel + offsetLabelY - 16;
	m_BrushStrengthSlider = g_GUI.AddSlider("", GUI::Rectangle(posOriginX + x, posOriginY + y, widthTextbox, heightTextbox), name);
	m_BrushStrengthSlider->SetBackgroundImage("Button_256x16.png");
	m_BrushStrengthSlider->SetSliderImage("Button_32.png");
	m_BrushStrengthSlider->SetMaxValue(10);
	m_BrushStrengthSlider->SetValue(1);
	x = m_BrushStrengthSlider->GetPosition().x + m_BrushStrengthSlider->GetSize().x + 6;
	m_BrushStrengthSliderValue = g_GUI.AddText("", GUI::TextDefinition("", posOriginX + x, posOriginY + y), name);
	
	
	// hardness slider
	x = 0;
	y = 104;
	label = g_GUI.AddText("", GUI::TextDefinition("Brush Hardness ", posOriginX + x, posOriginY + y, widthTextbox, heightLabel), name);
	label->SetTextAlignment(GUI::ALIGNMENT_TOP_LEFT);
	x = 6;
	y += heightLabel + offsetLabelY - 16;
	m_BrushHardnessSlider = g_GUI.AddSlider("", GUI::Rectangle(posOriginX + x, posOriginY + y, widthTextbox, heightTextbox), name);
	m_BrushHardnessSlider->SetBackgroundImage("Button_256x16.png");
	m_BrushHardnessSlider->SetSliderImage("Button_32.png");
	m_BrushHardnessSlider->SetMaxValue(1);
	m_BrushHardnessSlider->SetValue(0);
	x = m_BrushHardnessSlider->GetPosition().x + m_BrushHardnessSlider->GetSize().x + 6;
	m_BrushHardnessSliderValue = g_GUI.AddText("", GUI::TextDefinition("", posOriginX + x, posOriginY + y), name);


	m_BrushWindow->Open();
	
}

void SSEditorTerrain::InitializeDecal(void)
{
	// Create decal entity
	m_DecalEntity = g_EntityManager.CreateEntity();
	g_EntityManager.AddComponent(m_DecalEntity, GetDenseComponentTypeIndex<DecalComponent>());
	g_EntityManager.AddComponent(m_DecalEntity, GetDenseComponentTypeIndex<PlacementComponent>());
	g_EntityManager.AddComponent(m_DecalEntity, GetDenseComponentTypeIndex<TerrainFollowComponent>());
	g_EntityManager.AddComponent(m_DecalEntity, GetDenseComponentTypeIndex<CollisionComponent>());

	DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
	decal->Decal.Texture = g_DecalManager.GetTextureAtlas()->GetHandle("circle.png");
	decal->Decal.Tint = glm::vec4(1, 1, 1, 1);
	decal->AlwaysDraw = true;
	decal->Scale = 10.0f;
	glm::mat4 w = glm::translate(vec3(0)) * glm::rotate(3.14f * 0.5f, vec3(1, 0, 0)) * glm::scale(vec3(decal->Scale));
	decal->Decal.World = w;

	PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_DecalEntity);
	pc->Position = glm::vec3(50);
	pc->Scale = glm::vec3(10.0f);

	TerrainFollowComponent* tf = GetDenseComponent<TerrainFollowComponent>(m_DecalEntity);
	tf->Offset = 0.0f;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData(m_DecalEntity);
	collisionEntity->SetGroupID(PICKING_TYPE_BRUSH);
	g_CollisionDetection.AddCollisionVolumeSphere(collisionEntity, glm::vec3(0.0f), 1.0f); //adjust hitbox so we still render decal when we have the control point outside window
	GetDenseComponent<CollisionComponent>(m_DecalEntity)->CollisionEntity = collisionEntity;
}

void SSEditorTerrain::SetDecalToPosition(const glm::vec3& position)
{
	DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
	glm::mat4 w = glm::translate(vec3(position.x, position.y + 1.0f, position.z)) * glm::rotate(3.14f * 0.5f, vec3(1, 0, 0)) * glm::scale(vec3(decal->Scale));
	decal->Decal.World = w;
	decal->Scale = m_BrushRadius;
}

void SSEditorTerrain::ScriptWindowOnOpen(void)
{
	// m_WindowCreator->Open();
	g_SSEditorToolbox.SelectTool(Tool::TerrainEditTool);
	DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
	decal->AlwaysDraw = true;
}

void SSEditorTerrain::ScriptWindowOnClose(void)
{
	//m_WindowModify->Close();
	//m_WindowCreator->Close();

	g_SSEditorToolbox.SelectTool(Tool::TerrainEditTool);
	DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
	decal->AlwaysDraw = false;
	SetDecalToPosition(glm::vec3(-100, 0, -100));
}

void SSEditorTerrain::HandleMouseClick(bool isLeftClick, float deltaTime)
{
	bool invalidPos = false;
	if (g_Input->GetMousePosX() != m_LastMousePosX || g_Input->GetMousePosY() != m_LastMousePosY)
	{
		m_LastMousePosX = g_Input->GetMousePosX();
		m_LastMousePosY = g_Input->GetMousePosY();


		const glm::vec3& position = g_SSEditorToolbox.GetPickedPosition();

		invalidPos = position == INVALID_PICKED_POSITON;
		if (!invalidPos)
		{
			m_LastPickedX = position.x;
			m_LastPickedZ = position.z;
		}
	}

	bool MouseIsOnGUI = m_LastMousePosY >  m_WindowMain->GetBoundingBoxRef().GetTop();

	if (!MouseIsOnGUI)
	{
		int direction = isLeftClick ? 1 : -1;
		EditorTerrainBrushMessage brushMessage(g_GameData.GetFrameCount() + 1, glm::vec2(m_LastPickedX, m_LastPickedZ), direction * m_BrushStrength * deltaTime, m_BrushHardness, m_BrushRadius);
		g_SSMail.PushToNextFrame(brushMessage);
		m_OnTimeOut = true;
		m_CurrTimeOut = m_MaxTimeOut;
	}
}

void SSEditorTerrain::ScriptOpenTerrainWindow()
{
	m_BrushToolbox->Close();
	m_BrushWindow->Open();
	g_SSEditorToolbox.SelectTool(Tool::TerrainEditTool);
	DecalComponent* decal = GetDenseComponent<DecalComponent>(m_DecalEntity);
	decal->AlwaysDraw = true;
}