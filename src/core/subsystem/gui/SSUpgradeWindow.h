/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include "../gamelogic/SSUpgrades.h"

#define g_SSUpgradeWindow SSUpgradeWindow::GetInstance()

class SSUpgradeWindow : public Subsystem
{
	struct UpgradeButton
	{
		UPGRADE_NAME	Upgrade;
		GUI::Button*	Button;
		GUI::ProgressBar*	ProgressBar;
		GUI::Text*		ResearchTime;
		GUI::Sprite*	TimeBackground;
		int				ButtonIndex;
		int				ParentIndex;
	};
	
public:
	static SSUpgradeWindow& GetInstance( );
	
	void Startup() override;
	void UpdateUserLayer( const float dt ) override;
	void Shutdown() override;
	
	
	
private:
	// No external instancing allowed
	SSUpgradeWindow		() : Subsystem( "UpgradeWindow" ) {}
	SSUpgradeWindow		( const SSUpgradeWindow & rhs );
	~SSUpgradeWindow	() {}
	SSUpgradeWindow&	operator = (const SSUpgradeWindow & rhs);

	void						StartupResearchQueue();
	void						UpdateResearchQueue();
	
	const rString				m_UpgradeWindowName = "UpgradeWindow";

	int							m_NumFrontButtons;
	int							m_NumTopButtons;
	int							m_NumBackButtons;
	int							m_NumSidesButtons;
	int							m_ProgressBarHeight = 12;

	rVector<std::pair<UPGRADE_NAME, UpgradeData>>		m_SortedUpgradeData;
	rVector<UpgradeButton>		m_Buttons;
	rVector<GUI::Sprite*>		m_ProgressBars;
	
	GUI::ProgressBar*			m_CurrentResearchProgress;
	GUI::Text*					m_CurrentResearchText;
	GUI::Button*				m_CurrentResearch;
	//GUI::Sprite*				m_CurrentResearchProgress;
	
	GUI::Text*					m_CurrentResearchTime;
	
	
	//Progressbar colours
	const glm::vec4				m_ProgressColour =		glm::vec4( 0.0f, 0.1f, 0.8f, 0.5f );
	
	//Button colours
	const glm::vec4				m_UnResearchedColour =	glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	const glm::vec4				m_ResearchingColour =	glm::vec4( 1.0f, 1.0f, 0.2f, 1.0f );
	
	const glm::vec4				m_ResearchedColour =	glm::vec4( 0.6f, 1.0f, 0.6f, 1.0f );
	const glm::vec4				m_UpgradedColour =		glm::vec4( 0.6f, 0.6f, 1.0f, 1.0f );
	const glm::vec4				m_CantAffordColour =	glm::vec4( 1.0f, 0.6f, 0.6f, 1.0f );

	//const glm::vec4				m_UnResearchedColour =	glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f );
	//const glm::vec4				m_ResearchingColour =	glm::vec4( 0.4f, 0.4f, 0.4f, 1.0f );
	//
	//const glm::vec4				m_ResearchedColour =	glm::vec4( 0.3f, 0.4f, 0.3f, 1.0f );
	//const glm::vec4				m_UpgradedColour =		glm::vec4( 0.3f, 0.3f, 0.5f, 1.0f );
	
	const glm::ivec2			m_CurrentResearchSize = glm::ivec2( 256 - 32, 32 );
	const int					m_UpgradeButtonSize =	64;
	const int					m_ResearchTimeBackgroundHeight =	11;
	
	rString						m_UpgradeStatus;
	rString						m_UpgradeStatusInfo;

	//Queue
	rVector<GUI::Button*>		m_ResearchQueueButtons;


	//Glow effect
	GUI::Sprite*				m_ResearchGlow;
	float						m_GlowTime;
};
