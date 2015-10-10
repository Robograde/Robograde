/**************************************************
2015 Isak Almgren
***************************************************/

#pragma once
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_LoadingScreen LoadingScreen::GetInstance( )

class LoadingScreen
{
public:
		LoadingScreen();

	static LoadingScreen& GetInstance();

	void Render( const rString& name, int loaded, int total );

private:
	// No external instancing allowed
	LoadingScreen( const LoadingScreen& rhs );
	~LoadingScreen() {};
	LoadingScreen& operator=( const LoadingScreen& rhs );

	bool					m_Render = false;
	int						m_ProgressBarHeight = 32;
};
