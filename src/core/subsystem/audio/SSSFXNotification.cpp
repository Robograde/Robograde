/**************************************************
2015 Jens Stjernkvist
***************************************************/

#include "SSSFXNotification.h"

#include <utility/Logger.h>

#include "../utility/SSMail.h"
#include "../../input/GameMessages.h"
#include "../../ai/Squad.h"
#include "../../utility/PlayerData.h"
#include "../gamelogic/SSControlPoint.h"
#include "../../ai/AIMessages.h"
#include "../gamelogic/SSResearch.h"
#include "../gamelogic/SSCamera.h"
#include "../gamelogic/SSAI.h"
#include "SSMusicManager.h"

#define ENEMY_ENGAGE_MIN_DISTANCE_SQ 10000.0f

//+----------------------------------------------------------------------------+
//|SSSFXNotification& GetInstance(void)
//\----------------------------------------------------------------------------+
SSSFXNotification& SSSFXNotification::GetInstance(void)
{
	static SSSFXNotification instance;
	return instance;
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSSFXNotification::~SSSFXNotification()
{

}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSSFXNotification::Startup(void)
{
	m_UserInterests =	MessageTypes::ORDER_UNITS		| MessageTypes::RESEARCH			| MessageTypes::CONTROL_POINT |
						MessageTypes::UPGRADE			| MessageTypes::UPGRADE_COMPLETE	| MessageTypes::AI_MSG		  |
						MessageTypes::SFX_PING_MESSAGE	| MessageTypes::SFX_BUTTON_CLICK	;
	g_SSMail.RegisterSubscriber(this);
	
	Subsystem::Startup();
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSSFXNotification::Shutdown(void)
{
	m_Notifications.clear();
	g_SSMail.UnregisterSubscriber(this);

	Subsystem::Shutdown();
}
//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSSFXNotification::UpdateUserLayer(float deltaTime)
{
	Subsystem::UpdateUserLayer(deltaTime);

	///Handle all received messages
	for(int i = 0; i < m_UserMailbox.size(); ++i)
	{
		HandleMessage(m_UserMailbox[i]);
	}

	UpdateNotificationTimer(deltaTime);
}
//+----------------------------------------------------------------------------+
//|void Register(MessageTypes::MessageType  messageType, SFXNotificationEventType eventType, const rString &path, float constraint)
//|Register a message that a SFX should trigger upon recieving
//\----------------------------------------------------------------------------+
void SSSFXNotification::Register(MessageTypes::MessageType  messageType, SFXNotificationEventType eventType, const rString &path, float constraint)
{
	Notification notification;
	notification.MessageType = messageType;
	notification.Path = path;
	notification.Constraint = constraint;
	m_Notifications.emplace(eventType, notification);
}
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SSSFXNotification::SSSFXNotification()
	:	Subsystem("SFXNotification")
	,	Subscriber("SFXNotification")
{
}
//+----------------------------------------------------------------------------+
//|void HandleMessage(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleMessage(const Message* &message)
{
	///Different messagetypes should be handled different
	switch(message->Type)
	{
		///Order units commands
		case MessageTypes::ORDER_UNITS:
		{
			HandleOrderUnitsCommand(message);
			break;
		}

		///Research started and completed
		case MessageTypes::RESEARCH:
		{
			HandleResearch(message);
			break;
		}

		///Upgrade started
		case MessageTypes::UPGRADE:
		{
			HandleUpgradeStart(message);
			break;
		}

		///Upgrade End
		case MessageTypes::UPGRADE_COMPLETE:
		{
			HandleUpgradeEnd(message);
			break;
		}

		///Enemy engaging
		case MessageTypes::AI_MSG:
		{
			HandleEnemyEngage(message);
			break;
		}

		///Controlpoint capture, loss and contest
		case MessageTypes::CONTROL_POINT:
		{
			HandleControlPoint(message);
			break;
		}

		///Ping
		case MessageTypes::SFX_PING_MESSAGE:
		{
			HandlePing(message);
			break;
		}

		///GUI Event
		case MessageTypes::SFX_BUTTON_CLICK:
		{
			HandleGUIEvent(message);
			break;
		}

		///No matching case
		default:
		{
			Logger::Log("Received unhandled message type: " + rToString( message->Type ), "SSSFXNotification", LogSeverity::WARNING_MSG);
			return;
		}

	}
}
//+----------------------------------------------------------------------------+
//|void UpdateNotificationTimer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSSFXNotification::UpdateNotificationTimer(const float deltaTime)
{
	for(rUnorderedMap<SFXNotificationEventType, Notification>::iterator it = m_Notifications.begin(); it != m_Notifications.end(); it++)
	{
		if((*it).second.Constraint == 0.0f)
			continue;

		(*it).second.Timer += deltaTime;
 	}
}
//+----------------------------------------------------------------------------+
//|void TryPlaySFX(Notification* &notification, const bool toBeat)
//\----------------------------------------------------------------------------+
void SSSFXNotification::TryPlaySFX(Notification* &notification, const bool toBeat)
{
	if(notification->Timer >= notification->Constraint)
	{
		SFXEvent event;

		if(toBeat)
			event.AtBeat = BeatType::BEAT_100;
		else
			event.AtBeat = BeatType::NONE;

		event.Name = notification->Path;
		notification->SfxHandle = g_SSAudio.PostEventSFX(event);
		notification->Timer = 0.0f;
	}
}
//+----------------------------------------------------------------------------+
//|Notification* SSSFXNotification::GetNotification(SFXNotificationEventType eventType)
//\----------------------------------------------------------------------------+
SSSFXNotification::Notification* SSSFXNotification::GetNotification(SFXNotificationEventType eventType)
{
	rUnorderedMap<SFXNotificationEventType, Notification>::iterator it = m_Notifications.find(eventType);

	if(it == m_Notifications.end())
	{
		return nullptr;
	}

	return &(*it).second;
}
//+----------------------------------------------------------------------------+
//|void HandleOrderUnitsCommand(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleOrderUnitsCommand(const Message* &message)
{
	const OrderUnitsMessage* orderUnitsMessage = static_cast<const OrderUnitsMessage*> (message);

	///Identify which event the message is coupled to
	if(orderUnitsMessage->TeamID == g_PlayerData.GetPlayerID())
	{
		switch(orderUnitsMessage->MissionType)
		{
			case Squad::MissionType::MISSION_TYPE_MOVE:
			{
				Notification* note = GetNotification(SFXNotificationEventType::ORDER_UNITS_MOVE);

				if(note)
					TryPlaySFX(note);

				break;
			}

			case Squad::MissionType::MISSION_TYPE_ATTACK:
			{
				Notification* note = GetNotification(SFXNotificationEventType::ORDER_UNITS_ATTACK);

				if(note)
					TryPlaySFX(note);
				
				break;
			}

			case Squad::MissionType::MISSION_TYPE_ATTACK_MOVE:
			{
				Notification* note = GetNotification(SFXNotificationEventType::ORDER_UNITS_ATTACK_MOVE);

				if(note)
					TryPlaySFX(note);

				break;
			}

			case Squad::MissionType::MISSION_TYPE_MINE:
			{
				Notification* note = GetNotification(SFXNotificationEventType::ORDER_UNITS_MINE);

				if(note)
					TryPlaySFX(note);

				break;
			}

			default:
			{
				///No matching event
				break;
			}
		};
	}
}
//+----------------------------------------------------------------------------+
//|void HandleResearch(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleResearch(const Message* &message)
{
	const ResearchMessage* researchMessage = static_cast<const ResearchMessage*> (message);

	///Identify which event the message is coupled to
	if(researchMessage->PlayerID == g_PlayerData.GetPlayerID())
	{
		switch(researchMessage->StatusChange)
		{
			case RESEARCH_STATUS_CHANGE::RESEARCH_STATUS_CHANGE_STARTED:
			{
				Notification* note = GetNotification(SFXNotificationEventType::RESEARCH_STARTED);

				if(note)
					TryPlaySFX(note);

				break;
			}

			case RESEARCH_STATUS_CHANGE::RESEARCH_STATUS_CHANGE_FINISHED:
			{
				Notification* note = GetNotification(SFXNotificationEventType::RESEARCH_FINISHED);

				if(note)
					TryPlaySFX(note);

				break;
			}

			default:
			{
				///No matching event
				break;
			}
		};
	}
}
//+----------------------------------------------------------------------------+
//|void HandleUpgradeStart(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleUpgradeStart(const Message* &message)
{
	const UpgradeMessage* ugMessage = static_cast<const UpgradeMessage*> (message);

	if(ugMessage->PlayerID == g_PlayerData.GetPlayerID())
	{
		Notification* note = GetNotification(SFXNotificationEventType::UPGRADE_STARTED);

		if(note)
			TryPlaySFX(note);

		return;
	}
}//+----------------------------------------------------------------------------+
//|void HandleUpgradeEnd(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleUpgradeEnd(const Message* &message)
{
	const UpgradeCompleteMessage* ugMessage = static_cast<const UpgradeCompleteMessage*> (message);

	if(ugMessage->PlayerID == g_PlayerData.GetPlayerID())
	{
		Notification* note = nullptr;
		
		if(ugMessage->UpgradeType == MOVEMENT_FORTIFICATION)
			GetNotification(SFXNotificationEventType::UPGRADE_END_FORTIFICATION);
		else
			GetNotification(SFXNotificationEventType::UPGRADE_END);

		if(note)
			TryPlaySFX(note);

		return;
	}
}
//+----------------------------------------------------------------------------+
//|void HandleControlPoint(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleControlPoint(const Message* &message)
{
	const ControlPointMessage* cpMessage = static_cast<const ControlPointMessage*> (message);

	///Identify which event the message is coupled to
	switch(cpMessage->StatusChange)
	{
		case CONTROL_POINT_STATE_CHANGE_CAPTURED:
		{
			if(cpMessage->NewOwnerID == g_PlayerData.GetPlayerID())
			{
				Notification* note = GetNotification(SFXNotificationEventType::CONTROL_POINT_CAPTURED);

				if(note)
					TryPlaySFX(note);

				break;
			}
		}

		case CONTROL_POINT_STATE_CHANGE_LOST:
		{
			if(cpMessage->OldOwnerID == g_PlayerData.GetPlayerID())
			{
				Notification* note = GetNotification(SFXNotificationEventType::CONTROL_POINT_LOST);

				if(note)
					TryPlaySFX(note);

				break;
			}
		}

		case CONTROL_POINT_STATE_CHANGE_CONTESTED:
		{
			if(cpMessage->OldOwnerID == g_PlayerData.GetPlayerID() ) // Old owner ID is the current owner ID in this case
			{
				Notification* note = GetNotification(SFXNotificationEventType::CONTROL_POINT_CONTESTED);

				if(note)
					TryPlaySFX(note);

				return;
			}
		}

		default:
		{
			///No matching event
			break;
		}
	};
}
//+----------------------------------------------------------------------------+
//|void HandleEnemyEngage(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleEnemyEngage(const Message* &message)
{
	const AIMessage* aimessage = static_cast<const AIMessage*> (message);
	if(aimessage->TeamID == g_PlayerData.GetPlayerID())
	{
		if(static_cast<AIStateMessages>(aimessage->AIMsg) == AIStateMessages::UNDER_ATTACK)
		{
			Squad* squad = g_SSAI.GetSquadWithID(aimessage->TeamID, aimessage->SquadID);

			if(squad)
			{
				///The camera must be far away from the battle if a notificaion about it should be heard
				glm::vec3 camPos = g_SSCamera.GetActiveCamera()->GetPosition();
				glm::vec2 diff = glm::vec2(camPos.x, camPos.z) - squad->GetPosition(); ///Don't account for camera height
				float distSq = diff.x * diff.x + diff.y * diff.y; 

				if(distSq < ENEMY_ENGAGE_MIN_DISTANCE_SQ)
					return;

				Notification* note = GetNotification(SFXNotificationEventType::ENEMY_ENGAGE);

				if(note)
					TryPlaySFX(note);
			}
		}
	}
}
//+----------------------------------------------------------------------------+
//|void HandlePing(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandlePing(const Message* &message)
{
	const SFXPingMessage* pingMessage = static_cast<const SFXPingMessage*> (message);

	///Only local notification
	if(pingMessage->PlayerID == g_PlayerData.GetPlayerID())
	{
		Notification* note = nullptr;

		if(pingMessage->PingType == 0) ///Player ping
			note = GetNotification(SFXNotificationEventType::PING_PLAYER);
		else if(pingMessage->PingType == 1) ///Units spawn
			note = GetNotification(SFXNotificationEventType::PING_UNITS_SPAWN);

		if(note)
			TryPlaySFX(note);
	}
}
//+----------------------------------------------------------------------------+
//|void HandleGUIEvent(const Message* &message)
//\----------------------------------------------------------------------------+
void SSSFXNotification::HandleGUIEvent(const Message* &message)
{
	///Only play button sound in main menu
	if(g_SSMusicManager.GetName() == "main_menu")
	{	
		const SFXButtonClickMessage* guiMessage = static_cast<const SFXButtonClickMessage*>(message);

		Notification* note = nullptr;

		if(guiMessage->ButtonName == "Singleplayer")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_SINGLE);
		else if(guiMessage->ButtonName == "Multiplayer")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_MULTI);
		else if(guiMessage->ButtonName == "Replay")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_REPLAY);
		else if(guiMessage->ButtonName == "How to play")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_HOW);
		else if(guiMessage->ButtonName == "Options")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_OPTIONS);

		if(note)
			TryPlaySFX(note, false);

		rString chord = g_SSMusicManager.GetScriptStringValue("chordName");

		if(chord == "C")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_C);
		else if(chord == "F")
			note = GetNotification(SFXNotificationEventType::GUI_BUTTON_MENU_F);

		if(note)
			TryPlaySFX(note, false);
	}

}