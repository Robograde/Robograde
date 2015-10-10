/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Subscriber.h>
#include <gui/GUIEngine.h>
#include "../../utility/Catalogue.h"

enum class Tool;

// Conveniency access function
#define g_SSEditorObjectPlacer SSEditorObjectPlacer::GetInstance()

class SSEditorObjectPlacer : public Subsystem, public Subscriber
{
public:
	static SSEditorObjectPlacer&	GetInstance();

	void							Startup() override;
	void							Shutdown() override;
	void							UpdateUserLayer( const float deltaTime ) override;
	void							UpdateSimLayer( const float timeStep ) override;
	void							Reset();

	Tool							GetLastSelectedTool();

private:
	// No external instancing allowed
	SSEditorObjectPlacer() : Subsystem( "EditorObjectPlacer" ), Subscriber( "EditorObjectPlacer" ) {}
	SSEditorObjectPlacer( const SSEditorObjectPlacer& rhs );
	~SSEditorObjectPlacer() {};
	SSEditorObjectPlacer& operator=( const SSEditorObjectPlacer& rhs );

	GUI::Window*		m_PlacementWindow;
	GUI::Window*		m_PropPlacementWindow;
	GUI::Window*		m_ResourcePlacementWindow;
	GUI::Window*		m_ResearchStationPlacementWindow;
	Catalogue*			m_PropCatalogue;
	Catalogue*			m_ResourceCatalogue;
	Catalogue*			m_ResearchStationCatalogue;

	rVector<rString>	m_PropModels;
	rVector<rString>	m_ResourceModels;
	rVector<rString>	m_ResearchStationModels;

	rString				m_SelectedModel;

	Tool				m_LastSelectedTool;

	void				OnPropButtonPressed();
	void				OnResouceButtonPressed();
	void				OnResearchStationButtonPressed();
	void				OnCategoryOpen();
	void				OnCategoryClose();
	void				OnPropCatalogueButtonPressed( int buttonIndex );
	void				OnResourceCatalogueButtonPressed( int buttonIndex );
	void				OnResearchStationCatalogueButtonPressed( int buttonIndex );
};