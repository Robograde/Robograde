/**************************************************
2015 Jens Stjernkvist
***************************************************/

#include "SSAudioSettings.h"
#include "SSAudio.h"
#include "SSSFXNotification.h"
#include "SSMusicManager.h"

#include "../gamelogic/SSControlPoint.h"

#include "../../ai/AIMessages.h"

SSAudioSettings& SSAudioSettings::GetInstance( )
{
	static SSAudioSettings instance;
	return instance;
}

void SSAudioSettings::Startup( )
{
	g_SSAudio.ReadConfig("audio.cfg");

	rString origin = "../../../asset/audio/";

	///Weapons
	g_SSAudio.LoadSFXCollection(origin + "collection/weapon/weapon_cutter.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/weapon/weapon_pincer.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/weapon/weapon_machinegun.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/weapon/weapon_tank.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/weapon/weapon_laser.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/weapon/weapon_plasma.sfxc");

	///Robotvoices
	g_SSAudio.LoadSFXCollection(origin + "collection/robot/robot_eating.sfxc");

	g_SSAudio.LoadSFXCollection(origin + "collection/robot/robot_move_walk.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/robot/robot_move_attack.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/robot/robot_move_mine.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/robot/robot_death.sfxc");

	///Global notifications
	g_SSAudio.LoadSFX(origin + "sfx/notification/research_center/rc_captured.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/research_center/rc_contest.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/research_center/rc_lost.wav", 1);

	g_SSAudio.LoadSFX(origin + "sfx/notification/upgrade/upgrade_start.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/upgrade/upgrade_end.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/upgrade/fortification_activate.wav", 1);

	g_SSAudio.LoadSFX(origin + "sfx/notification/research/research_complete.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/research/research_start.wav", 1);

	g_SSAudio.LoadSFXCollection(origin + "collection/notification/enemy_engage.sfxc");

	g_SSAudio.LoadSFX(origin + "sfx/notification/ping/ping.wav", 5);
	g_SSAudio.LoadSFX(origin + "sfx/notification/ping/ping_units_spawn.wav", 1);

	///GUI
	g_SSAudio.LoadSFXCollection(origin + "collection/notification/gui_button_menu_c.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/notification/gui_button_menu_f.sfxc");

	g_SSAudio.LoadSFX(origin + "sfx/notification/gui/button/menu/announce_single_player.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/gui/button/menu/announce_multiplayer.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/gui/button/menu/announce_replay.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/gui/button/menu/announce_options.wav", 1);
	g_SSAudio.LoadSFX(origin + "sfx/notification/gui/button/menu/announce_how_to_play.wav", 1);

	///Misc
	g_SSAudio.LoadSFXCollection(origin + "collection/misc/explosion.sfxc");
	g_SSAudio.LoadSFXCollection(origin + "collection/misc/resource_fall.sfxc");

	///Register global SFX notifications
	g_SSSFXNotification.Register(MessageTypes::ORDER_UNITS,			SFXNotificationEventType::ORDER_UNITS_MOVE,			origin + "collection/robot/robot_move_walk.sfxc",							1.0f);
	g_SSSFXNotification.Register(MessageTypes::ORDER_UNITS,			SFXNotificationEventType::ORDER_UNITS_ATTACK,		origin + "collection/robot/robot_move_attack.sfxc",							1.0f);
	g_SSSFXNotification.Register(MessageTypes::ORDER_UNITS,			SFXNotificationEventType::ORDER_UNITS_ATTACK_MOVE,	origin + "collection/robot/robot_move_attack.sfxc",							1.0f);
	g_SSSFXNotification.Register(MessageTypes::ORDER_UNITS,			SFXNotificationEventType::ORDER_UNITS_MINE,			origin + "collection/robot/robot_move_mine.sfxc",							1.0f);

	g_SSSFXNotification.Register(MessageTypes::RESEARCH,			SFXNotificationEventType::RESEARCH_STARTED,			origin + "sfx/notification/research/research_start.wav",					1.0f);
	g_SSSFXNotification.Register(MessageTypes::RESEARCH,			SFXNotificationEventType::RESEARCH_FINISHED,		origin + "sfx/notification/research/research_complete.wav",					1.0f);

	g_SSSFXNotification.Register(MessageTypes::UPGRADE,				SFXNotificationEventType::UPGRADE_STARTED,			origin + "sfx/notification/upgrade/upgrade_start.wav",						1.0f);
	g_SSSFXNotification.Register(MessageTypes::UPGRADE,				SFXNotificationEventType::UPGRADE_END,				origin + "sfx/notification/upgrade/upgrade_end.wav",						10.0f);
	g_SSSFXNotification.Register(MessageTypes::UPGRADE,				SFXNotificationEventType::UPGRADE_END_FORTIFICATION,origin + "sfx/notification/upgrade/fortification_activate.wav",				10.0f);

	g_SSSFXNotification.Register(MessageTypes::CONTROL_POINT,		SFXNotificationEventType::CONTROL_POINT_CAPTURED,	origin + "sfx/notification/research_center/rc_captured.wav",				1.0f);
	g_SSSFXNotification.Register(MessageTypes::CONTROL_POINT,		SFXNotificationEventType::CONTROL_POINT_CONTESTED,	origin + "sfx/notification/research_center/rc_contest.wav",					1.0f);
	g_SSSFXNotification.Register(MessageTypes::CONTROL_POINT,		SFXNotificationEventType::CONTROL_POINT_LOST,		origin + "sfx/notification/research_center/rc_lost.wav",					1.0f);

	g_SSSFXNotification.Register(MessageTypes::AI_MSG,				SFXNotificationEventType::ENEMY_ENGAGE,				origin + "collection/notification/enemy_engage.sfxc",						10.0f);

	g_SSSFXNotification.Register(MessageTypes::SFX_PING_MESSAGE,	SFXNotificationEventType::PING_PLAYER,				origin + "sfx/notification/ping/ping.wav",									0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_PING_MESSAGE,	SFXNotificationEventType::PING_UNITS_SPAWN,			origin + "sfx/notification/ping/ping_units_spawn.wav",						10.0f);

	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_C,		origin + "collection/notification/gui_button_menu_c.sfxc",					0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_F,		origin + "collection/notification/gui_button_menu_f.sfxc",					0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_SINGLE,	origin + "sfx/notification/gui/button/menu/announce_single_player.wav",		0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_MULTI,	origin + "sfx/notification/gui/button/menu/announce_multiplayer.wav",		0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_REPLAY,	origin + "sfx/notification/gui/button/menu/announce_replay.wav",			0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_OPTIONS,	origin + "sfx/notification/gui/button/menu/announce_options.wav",			0.0f);
	g_SSSFXNotification.Register(MessageTypes::SFX_BUTTON_CLICK,	SFXNotificationEventType::GUI_BUTTON_MENU_HOW,		origin + "sfx/notification/gui/button/menu/announce_how_to_play.wav",		0.0f);
	Subsystem::Startup();
}
