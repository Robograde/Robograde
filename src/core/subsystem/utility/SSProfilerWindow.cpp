/**************************************************
2015 Johan Melin
***************************************************/

#include "SSProfilerWindow.h"
#include <profiler/Profiler.h>
#include <input/KeyBindings.h>
#include <algorithm>
#include <utility/ConfigManager.h>
#include <utility/Logger.h>
#include <script/ScriptEngine.h>
#include "../input/SSKeyBinding.h"
#include "../../CompileFlags.h"

SSProfilerWindow& SSProfilerWindow::GetInstance( )
{
	static SSProfilerWindow instance;
	return instance;
}

void SSProfilerWindow::Startup( )
{
	CallbackConfig* cfg = g_ConfigManager.GetConfig( m_ConfigPath );
	assert( cfg );
	Cleanup( );
	InitializeWindows( cfg );
	
	m_ConfigRegisterID = cfg->RegisterInterest( [this]( CallbackConfig* cfg )
	{
		Cleanup( );
		InitializeWindows( cfg );
	} );
	
	g_Script.Register( "GE_SwitchProfilerSortMethod", [this]( IScriptEngine* ) -> int
	{
		SwitchSortingMethod( );
		return 0;
	} );
	
	g_Script.Register( "GE_ResetProfilerMaxEntries", []( IScriptEngine* ) -> int
	{
		g_Profiler.ResetMaxEntries( );
		return 0;
	} );
}

void SSProfilerWindow::UpdateUserLayer( const float deltaTime )
{
	DEV(
		if( g_SSKeyBinding.ActionUpDown( ACTION_RESET_MAX_PROFILER_TIMES ) )
			g_Profiler.ResetMaxEntries( );
		if( g_SSKeyBinding.ActionUpDown( ACTION_SWITCH_PROFILER_SORT ) )
			SwitchSortingMethod( );
		if( g_SSKeyBinding.ActionUpDown( ACTION_TOGGLE_PROFILE_WINDOW ) )
			m_MainWindow->ToggleOpen( );
	);
		
	if( m_MainWindow->IsOpen( ) )
	{
		// Update text on all category windows
		for( auto& window : m_CategoryWindows )
		{
			GUI::TextDefinition* names = &window.TextNames->GetTextDefinitionRef( );
			GUI::TextDefinition* averages = &window.TextAverages->GetTextDefinitionRef( );
			GUI::TextDefinition* maxes = &window.TextMaxes->GetTextDefinitionRef( );
			names->Text = "";
			averages->Text = "";
			maxes->Text = "";
			
			auto mapForCategory = g_Profiler.GetMapForCategory( window.Category );
			m_ToBeSorted.reserve( mapForCategory->size( ) );
			// Put map collection in vector
			std::for_each( mapForCategory->begin( ), mapForCategory->end( ), [this]( const std::pair<rString, Profiler::ProfileEntry>& p )
			{
				m_ToBeSorted.push_back( p );
			} );
			SortToBeSorted( );
			unsigned int c = 0;
			for( auto& entry : m_ToBeSorted )
			{
				names->Text += entry.first + "\n";
				averages->Text += rToString( entry.second.GetAverage( ) * g_Profiler.GetConversionFactorMilliSeconds( ) ) + "/\n";
				maxes->Text += rToString( entry.second.GetMax( ) * g_Profiler.GetConversionFactorMilliSeconds( ) ) + "\n";
				
				c++;
				// Only display given amount of entries
				if( c >= window.NumberOfEntriesToShow )
					break;
			}
			// Vector will be reused for later iterations
			m_ToBeSorted.clear( );
		}
		
		// Update text for specific window
		for( auto& swindow : m_SpecificInterest )
		{
			const Profiler::ProfileEntry* entry = g_Profiler.GetEntry( swindow );
			if( entry != nullptr )
				m_ToBeSorted.push_back( std::pair<rString, Profiler::ProfileEntry>( swindow, *entry ) );
		}
		SortToBeSorted( );
		// Update text with each entry
		unsigned int c = 0;
		GUI::TextDefinition* names = &m_SpecificWindow.TextNames->GetTextDefinitionRef( );
		GUI::TextDefinition* times = &m_SpecificWindow.TextAverages->GetTextDefinitionRef( );
		GUI::TextDefinition* maxes = &m_SpecificWindow.TextMaxes->GetTextDefinitionRef( );
		names->Text = "";
		times->Text = "";
		maxes->Text = "";
		for( auto& sentry : m_ToBeSorted )
		{
			names->Text += sentry.first + "\n";
			times->Text += rToString( sentry.second.GetAverage( ) * g_Profiler.GetConversionFactorMilliSeconds( ) ) + "/\n";
			maxes->Text += rToString( sentry.second.GetMax( ) * g_Profiler.GetConversionFactorMilliSeconds( ) ) + "\n";
			c++;
			// Only display given amount of entries
			if( c >= m_SpecificWindow.NumberOfEntriesToShow )
				break;
		}
		m_ToBeSorted.clear( );
	}
}

void SSProfilerWindow::Shutdown( )
{
	CallbackConfig* cfg = g_ConfigManager.GetConfig( m_ConfigPath );
	assert( cfg );
	cfg->UnregisterInterest( m_ConfigRegisterID );
	Cleanup( );
}

void SSProfilerWindow::InitializeWindows( CallbackConfig* cfg )
{
	glm::ivec2 parentPos = g_GUI.GetWindowPos( m_MainWindowParentName );
	glm::ivec2 parentSize = g_GUI.GetWindowSize( m_MainWindowParentName );
	m_MainWindow = g_GUI.AddWindow( m_MainWindowName, GUI::Rectangle( parentPos.x, parentPos.y, parentSize.x, parentSize.y ), m_MainWindowParentName, false );
	m_MainWindow->SetClickThrough( true );
	int nrOfSpecificEntries = cfg->GetInt( "nrofspecificentries", 10 );
	int nrOfMajorEntries = cfg->GetInt( "nrofmajorentries", 10 );
	int nrOfGPUEntries = cfg->GetInt( "nrofgpuentries", 10 );
	int nrOfStandardEntries = cfg->GetInt( "nrofstandardentries", 10 );
	int yoffset = 25;
	AddProfilerCategoryWindow( "Specific", "User defined entry times in milliseconds", 0, yoffset, 700,
	                           nrOfSpecificEntries * m_RowSize + m_TitleSize, Profiler::PROFILER_CATEGORY_STANDARD, nrOfSpecificEntries, true );
	yoffset += nrOfSpecificEntries * m_RowSize + m_TitleSize;
	AddProfilerCategoryWindow( "Major", "Times for major parts in milliseconds", 0, yoffset, 700,
	                           nrOfMajorEntries * m_RowSize + m_TitleSize, Profiler::PROFILER_CATEGORY_MAJOR, nrOfMajorEntries );
	yoffset += nrOfMajorEntries * m_RowSize + m_TitleSize;
	AddProfilerCategoryWindow( "GPU", "GPU times in milliseconds", 0, yoffset, 700, nrOfGPUEntries * m_RowSize + m_TitleSize,
	                           Profiler::PROFILER_CATEGORY_GPU, nrOfGPUEntries );
	yoffset += nrOfGPUEntries * m_RowSize + m_TitleSize;
	AddProfilerCategoryWindow( "Standard", "Standard times in milliseconds", 0, yoffset, 700, nrOfStandardEntries * m_RowSize + m_TitleSize,
	                           Profiler::PROFILER_CATEGORY_STANDARD, nrOfStandardEntries );
	yoffset += nrOfStandardEntries * m_RowSize + m_TitleSize;
	
	LoadSpecificInterest( *cfg );
}

void SSProfilerWindow::Cleanup( )
{
	g_GUI.DeleteObject( m_MainWindowName );
	m_CategoryWindows.clear( );
	m_ToBeSorted.clear( );
	m_SpecificInterest.clear( );
}

void SSProfilerWindow::AddProfilerCategoryWindow( const rString& name, const rString& title, int x, int y, int width, int height,
        Profiler::PROFILER_CATEGORY category, unsigned int numberOfEntriesToShow, bool isSpecificWindow )
{
	ProfileCategoryWindow window;
	window.Window = g_GUI.AddWindow( "ProfilerWindow" + name, GUI::Rectangle( x, y, width, height ), m_MainWindowName, true );
	window.Window->Open( );
	window.Window->SetMoveable( true );
	window.Window->SetClickThrough( true );
	GUI::Text* tempText;
	g_GUI.UseFont( FONT_ID_CONSOLE_14 );
	tempText = g_GUI.AddText( "Title" + name, GUI::TextDefinition( title.c_str( ), 0, 15 ), window.Window->GetName( ) );
	tempText->SetTextAlignment( GUI::ALIGNMENT_TOP_CENTER );
	g_GUI.UseFont( FONT_ID_CONSOLE_11 );
	window.TextNames = g_GUI.AddText( "Names" + name, GUI::TextDefinition( "", m_NamesX, m_TopSpacing ), window.Window->GetName( ) );
	window.TextAverages = g_GUI.AddText( "Average" + name, GUI::TextDefinition( "", m_AveragesX, m_TopSpacing ), window.Window->GetName( ) );
	window.TextMaxes = g_GUI.AddText( "Max" + name, GUI::TextDefinition( "", m_MaxesX, m_TopSpacing ), window.Window->GetName( ) );
	window.Category = category;
	window.NumberOfEntriesToShow = numberOfEntriesToShow;
	if( isSpecificWindow )
		m_SpecificWindow = window;
	else
		m_CategoryWindows.push_back( window );
}

void SSProfilerWindow::LoadSpecificInterest( CallbackConfig& cfg )
{
	rVector<Config::ConfigEntry*>* interestArray = cfg.GetArray( "interests" );
	for( auto& interest : *interestArray )
	{
		if( interest->Type != Config::Type::String )
			Logger::Log( "Tried to load non-string value as specific interest. Value will be ignored.", "SSProfilerWindow", LogSeverity::WARNING_MSG );
		else
			m_SpecificInterest.push_back( interest->Value.StringVal );
	}
}

void SSProfilerWindow::SwitchSortingMethod( )
{
	m_SortOn = m_SortOn == ToSortOn::Maxes ? ToSortOn::Averages : ToSortOn::Maxes;
}

void SSProfilerWindow::SortToBeSorted( )
{
	switch( m_SortOn )
	{
		case ToSortOn::Averages:
		{
			// Sort by average
			sort( m_ToBeSorted.begin( ), m_ToBeSorted.end( ),
			      []( const std::pair<rString, Profiler::ProfileEntry>& a, const std::pair<rString, Profiler::ProfileEntry>& b ) -> bool
			{
				return a.second.GetAverage( ) > b.second.GetAverage( );
			} );
		}
		break;
		case ToSortOn::Maxes:
		{
			// Sort by maxes
			sort( m_ToBeSorted.begin( ), m_ToBeSorted.end( ),
			      []( const std::pair<rString, Profiler::ProfileEntry>& a, const std::pair<rString, Profiler::ProfileEntry>& b ) -> bool
			{
				return a.second.GetMax( ) > b.second.GetMax( );
			} );
		}
		break;
	}
}
