/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include <profiler/Profiler.h>
#include <utility/CallbackConfig.h>

// Conveniency access function
#define g_SSProfilerWindow SSProfilerWindow::GetInstance( )

class SSProfilerWindow : public Subsystem
{
	public:
	static SSProfilerWindow& GetInstance( );

	void Startup( ) override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown( ) override;

	private:
	struct ProfileCategoryWindow
	{
		GUI::Window* Window = nullptr;
		GUI::Text* TextNames = nullptr;
		GUI::Text* TextAverages = nullptr;
		GUI::Text* TextMaxes = nullptr;
		Profiler::PROFILER_CATEGORY Category = Profiler::PROFILER_CATEGORY_STANDARD;
		unsigned int NumberOfEntriesToShow = 10;
	};
	// No external instancing allowed
	SSProfilerWindow( ) : Subsystem( "ProfilerWindow" )
	{
	}
	SSProfilerWindow( const SSProfilerWindow& rhs );
	~SSProfilerWindow( ){};
	SSProfilerWindow& operator=( const SSProfilerWindow& rhs );

	void InitializeWindows( CallbackConfig* cfg );
	void Cleanup( );
	void AddProfilerCategoryWindow( const rString& name, const rString& title, int x, int y, int width, int height,
									Profiler::PROFILER_CATEGORY category, unsigned int numberOfEntriesToShow, bool isSpecificWindow = false );
	void LoadSpecificInterest( CallbackConfig& cfg );
	void SwitchSortingMethod( );
	void SortToBeSorted( );

	enum class ToSortOn
	{
		Averages,
		Maxes,
	};

	CallbackConfigRegisterID m_ConfigRegisterID = 0;

	GUI::Window* m_MainWindow;
	rVector<ProfileCategoryWindow> m_CategoryWindows;
	rVector<std::pair<rString, Profiler::ProfileEntry>> m_ToBeSorted;
	// Stores keys that is used for the specific window,
	// allowing users to define systems that they are interested in
	rVector<rString> m_SpecificInterest;
	ProfileCategoryWindow m_SpecificWindow;
	const rString m_ConfigPath = "profiler.cfg";

	const rString m_MainWindowName = "ProfilerMainWindow";
	const rString m_MainWindowParentName = "DebugWindow";
	const int m_TitleSize = 30;
	const int m_RowSize = 14;
	const int m_NamesX = 178;
	const int m_AveragesX = 4;
	const int m_MaxesX = 86;
	const int m_TopSpacing = 32;

	ToSortOn m_SortOn = ToSortOn::Maxes;
};
