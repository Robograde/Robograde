/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Subscriber.h>

#include <memory/Alloc.h>

#include "SSAudio.h"
#include "../../input/GameMessages.h"

#define g_SSSFXNotification SSSFXNotification::GetInstance()

enum class SFXNotificationEventType
{
	///Order units
	ORDER_UNITS_MOVE,
	ORDER_UNITS_ATTACK,
	ORDER_UNITS_ATTACK_MOVE,
	ORDER_UNITS_MINE,

	///Research
	RESEARCH_STARTED,
	RESEARCH_FINISHED,

	///Upgrade
	UPGRADE_STARTED,
	UPGRADE_END,
	UPGRADE_END_FORTIFICATION,

	///Controlpoint
	CONTROL_POINT_CAPTURED,
	CONTROL_POINT_CONTESTED,
	CONTROL_POINT_LOST,

	///Enemy engage
	ENEMY_ENGAGE,

	///Ping
	PING_PLAYER,
	PING_UNITS_SPAWN,

	///GUI
	GUI_BUTTON_MENU_C,		
	GUI_BUTTON_MENU_F,	
	GUI_BUTTON_MENU_SINGLE,	
	GUI_BUTTON_MENU_MULTI,	
	GUI_BUTTON_MENU_REPLAY,	
	GUI_BUTTON_MENU_OPTIONS,	
	GUI_BUTTON_MENU_HOW,		
};

//+===+=====================+==========================
//----| SSSFXNotification	|
//----+---------------------+--------------------------
//----| Handles SFXs notifications that should be played globaly to the player
//+===+================================================
class SSSFXNotification :
	public Subsystem,
	public Subscriber
{
	//+===+================================================
	//----| Notification	|
	//----+-----------------+------------------------------
	//----|Contains the information about the SFX that should be played when a correct message is received
	//+===+================================================
	struct Notification
	{
		Notification(void)
		{
			Timer = 0.0f;
			Constraint = 0.0f;
			SfxHandle = 0;
		}

		rString						Path;
		MessageTypes::MessageType	MessageType;
		SFXNotificationEventType	EventType;

		float						Timer,
									Constraint;
		SFX_HANDLE					SfxHandle;
	};

//Public Functions	 
public:
	static SSSFXNotification&	GetInstance(void);
								~SSSFXNotification();

	void						Startup(void) override;
	void						Shutdown(void) override;

	void						UpdateUserLayer(const float deltaTime) override;

	void						Register(MessageTypes::MessageType messageType, SFXNotificationEventType eventType, const rString &path, float duration = 0.0f);

//Private Functions
private:
								SSSFXNotification();
								SSSFXNotification(const SSSFXNotification &ref);
	SSSFXNotification&			operator=(const SSSFXNotification &ref);

	void						HandleMessage(const Message* &message);
	void						UpdateNotificationTimer(const float deltaTime);
	void						TryPlaySFX(Notification* &notification, const bool toBeat = false);
	Notification*				GetNotification(SFXNotificationEventType eventType);

	void						HandleOrderUnitsCommand(const Message* &message);
	void						HandleResearch(const Message* &message);
	void						HandleUpgradeStart(const Message* &message);
	void						HandleUpgradeEnd(const Message* &message);
	void						HandleControlPoint(const Message* &message);
	void						HandleEnemyEngage(const Message* &message);
	void						HandlePing(const Message* &message);
	void						HandleGUIEvent(const Message* &message);
//Private Variables
private:
	rUnorderedMap<SFXNotificationEventType, Notification>		m_Notifications;

};