/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once 
#include <script/IScriptEngine.h>

namespace GUI
{
	class ScriptFunctions
	{
	public:
		
		void RegisterFunctions();
		
	private:
		//Add
		int AddTextSF				( IScriptEngine* scriptEngine );
		int	AddButtonSF				( IScriptEngine* scriptEngine );
		int	AddToggleButtonSF		( IScriptEngine* scriptEngine );
		int	AddImageButtonSF		( IScriptEngine* scriptEngine );
		int	AddToggleImageButtonSF	( IScriptEngine* scriptEngine );
		int AddWindowSF				( IScriptEngine* scriptEngine );
		
		//Getters
		int GetWindowSizeSF		( IScriptEngine* scriptEngine );
		
		//Setters
		//int SetButtonImage		( IScriptEngine* scriptEngine );
		int SetWindowMoveableSF	( IScriptEngine* scriptEngine );
		
		
		//Other
		int OpenWindowSF		( IScriptEngine* scriptEngine );
		int CloseWindowSF		( IScriptEngine* scriptEngine );
		int ToggleWindowSF		( IScriptEngine* scriptEngine );
		int BringWindowToFrontSF( IScriptEngine* scriptEngine );
		
		int UseFontSF			( IScriptEngine* scriptEngine );
		
		int ToggleButtonGroupSF	( IScriptEngine* scriptEngine );
		int ToggleWindowGroupSF	( IScriptEngine* scriptEngine );
		int SetButtonToggleSF	( IScriptEngine* scriptEngine );
		
		int SetEnabledSF		( IScriptEngine* scriptEngine );
	}; 
}
