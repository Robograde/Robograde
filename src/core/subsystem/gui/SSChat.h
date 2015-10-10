/**************************************************
2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Subscriber.h>
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSChat SSChat::GetInstance( )

class SSChat : public Subsystem, public Subscriber
{
public:

	enum CHATSTATE
	{
		CHATSTATE_LOBBY,
		CHATSTATE_INGAME,
		CHATSTATE_INGAME_NOINPUT
	};

	enum CHAT_TARGET
	{
		CHAT_TARGET_ALL,
		CHAT_TARGET_ALLIED,
	};

	static SSChat& GetInstance();

	void Startup() override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown() override;

	void InitializeChatWindow( const int x, const int y, const int width, const int height, const int maxChars, CHATSTATE chatstate );

	void SetChatState( CHATSTATE chatState );

private:
	// No external instancing allowed
	SSChat() : Subsystem( "Chat" ), Subscriber( "Chat" ) {}
	SSChat( const SSChat& rhs );
	~SSChat() {};
	SSChat& operator=( const SSChat& rhs );

	void				ManageFading( const float deltaTime );


	rVector<rString>	m_Messages;
	rVector<rString>	m_History;
	rVector<GUI::Text*> m_Texts;
	GUI::Window *		m_Window;

	GUI::Text*			m_Target;
	GUI::Text*			m_LobbyTitle;
	GUI::TextBox*		m_TextBox;
	const glm::ivec2	m_TextBoxSize = glm::ivec2( 128, 32 );

	const glm::ivec2	m_TextSize = glm::ivec2( 128, 16 );

	CHATSTATE			m_ChatState = CHATSTATE_LOBBY;
	CHAT_TARGET			m_ChatTarget = CHAT_TARGET_ALLIED;

	bool				m_KeepOpen = false;
	const float			m_FadeTime = 0.5f;
	const float			m_MessageFadeTime = 0.4f;
	const float			m_RecieveFadeTime = 10.0f;
	float				m_CurrentFadeTime = 0.0f;

	bool				m_RecievedFade = false;

	bool				m_IsInitialized = false;

	int					m_MaxChars = 20;
};
