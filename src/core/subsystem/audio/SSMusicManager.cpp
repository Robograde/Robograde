/**************************************************
2015 Jens Stjernkvist
***************************************************/

#include "SSMusicManager.h"

#include <utility/Logger.h>
#include <utility/Randomizer.h>

#include "SSAudio.h"
#include "../utility/SSMail.h"
#include "../../../script/ScriptEngine.h"
#include "../../input/GameMessages.h"
#include "../../utility/PlayerData.h"
#include "../../ai/Squad.h"
#include "../../ai/AIMessages.h"

#define PATTERNS_ELEMENTS_MAX	4		///Number of elements that a pattern contains(name, path, category)
#define BEATS_BAR				4		///Number if beats contained in a bar
//+----------------------------------------------------------------------------+
//|SSMusicManager& GetInstance(void)
//\----------------------------------------------------------------------------+
SSMusicManager& SSMusicManager::GetInstance(void)
{
	static SSMusicManager instance;
	return instance;
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSMusicManager::~SSMusicManager()
{

}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSMusicManager::Startup(void)
{
	m_ScriptEngine = &ScriptEngine::GetInstance(ScriptInstance::Audio);
	m_ScriptEngine->Reload(nullptr);

	m_UserInterests = MessageTypes::ORDER_UNITS | MessageTypes::AI_MSG;
	g_SSMail.RegisterSubscriber(this);

	m_PatternMain		= nullptr;
	m_PatternAmbient	= nullptr;
	m_PatternAdditional = nullptr;
	m_BeatCounts = 0;
	m_BeatsNum = 0;
	m_BarCounter = 0;
	m_Name = "";

	Subsystem::Startup();
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSMusicManager::Shutdown(void)
{
	if(m_ScriptEngine)
		m_ScriptEngine->Reload(nullptr);

	m_ScriptLoaded = false;
	g_SSMail.UnregisterSubscriber(this);

	Subsystem::Shutdown();
}
//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSMusicManager::UpdateUserLayer(const float deltaTime)
{
	if(!m_ScriptLoaded)
		return;

	///Handle all received messages
	for(int i = 0; i < m_UserMailbox.size(); ++i)
	{
		HandleMessage(m_UserMailbox[i]);
	}

	UpdateBeatCount();	

	m_ScriptEngine->Call("Update", {deltaTime});
}
//+----------------------------------------------------------------------------+
//|void Activate(void)
//|Connect handles to the loaded script and start the music
//\----------------------------------------------------------------------------+
void SSMusicManager::Activate(void)
{
	if(!m_ScriptLoaded)
	{
		Logger::Log("Activate. Trying to activate SSMusicManager without a script is loaded first", "SSMusicManager", LogSeverity::WARNING_MSG);
		return;
	}

	///Load variable values
	unsigned int bpm = m_ScriptEngine->GetUint("bpm");
	m_BeatsNum = m_ScriptEngine->GetInt("beats");
	m_Name = m_ScriptEngine->GetString("name");

	///Register scriptfunctions
	m_ScriptEngine->Register("SetPatternMain",  std::bind(&SSMusicManager::ScriptSetPatternMain, this, std::placeholders::_1));
	m_ScriptEngine->Register("SetPatternAdditional",  std::bind(&SSMusicManager::ScriptSetPatternAdditional, this, std::placeholders::_1));
	m_ScriptEngine->Register("SetPatternAmbient",  std::bind(&SSMusicManager::ScriptSetPatternAmbient, this, std::placeholders::_1));
	m_ScriptEngine->Register("SetRandomMainPattern",  std::bind(&SSMusicManager::ScriptSetRandomMainPattern, this, std::placeholders::_1));
	m_ScriptEngine->Register("SetRandomAdditionalPattern",  std::bind(&SSMusicManager::ScriptSetRandomAdditionalPattern, this, std::placeholders::_1));
	m_ScriptEngine->Register("KillAllPatterns",  std::bind(&SSMusicManager::ScriptKillAllPatterns, this, std::placeholders::_1));
	m_ScriptEngine->Register("ResetBeatCounter",  std::bind(&SSMusicManager::ScriptResetBeatCounter, this, std::placeholders::_1));
	m_ScriptEngine->Register("GetRandomInt",  std::bind(&SSMusicManager::ScriptGetRandomInt, this, std::placeholders::_1));


	g_SSAudio.SetBPM(bpm);

	m_ScriptEngine->Call("Start");
}
//+----------------------------------------------------------------------------+
//|void Reset(void)
//|Remove everything that was set during LoadScenceMusic and Activate
//\----------------------------------------------------------------------------+
void SSMusicManager::Reset(void)
{
	if(m_ScriptEngine)
	{
		m_ScriptEngine->Call("Shutdown");
		m_ScriptEngine->Reload(nullptr);
	}

	m_ScriptLoaded = false;

	UnloadAllSceneMusic();

	m_PatternMain		= nullptr;
	m_PatternAmbient	= nullptr;
	m_PatternAdditional = nullptr;
	m_BeatCounts = 0;
	m_BeatsNum = 0;
	m_BarCounter = 0;
	m_Name = "";
}
//+----------------------------------------------------------------------------+
//|const bool LoadSceneMusic(const rString &path)
//\----------------------------------------------------------------------------+
const bool SSMusicManager::LoadSceneMusic(const rString &path)
{
	///Open LUA File if the script is not loaded allready
	if(!m_ScriptLoaded)
	{
		m_ScriptEngine->Run(path.c_str());
		m_ScriptLoaded = true;

		///Get and store all the musicsets
		unsigned int patternsSize = m_ScriptEngine->GetUint("patternsSize");

		if(patternsSize == 0)
		{
			Logger::Log("LoadSceneMusic. No music pattrens available. Syntax error in script?", "MusicManager", LogSeverity::ERROR_MSG);
			return false;
		}

		for(unsigned int i = 1; i < patternsSize + 1; i++)
		{
			m_ScriptEngine->Call("GetPattern", {i});
			LoadPattern(i);
		}

		///Load all the music that will be used
		for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
		{
			if(!g_SSAudio.LoadMusic((*it).Path))
				return false;
		}

		return true;
	}

	return false;
}
//+----------------------------------------------------------------------------+
//|void UnloadAllSceneMusic(void)
//|Check all the sets loaded and try to unload the music not used anymore  
//\----------------------------------------------------------------------------+
void SSMusicManager::UnloadAllSceneMusic(void)
{
	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		g_SSAudio.UnloadMusic((*it).Path);
	}
	m_Patterns.clear();
}
//+----------------------------------------------------------------------------+
//|rString GetName(void)
//\----------------------------------------------------------------------------+
rString SSMusicManager::GetName(void) const
{
	return m_Name;
}
//+----------------------------------------------------------------------------+
//|rString GetScriptStringValue(const rString &varName)
//\----------------------------------------------------------------------------+
rString SSMusicManager::GetScriptStringValue(const rString &varName)
{
	return m_ScriptEngine->GetString(varName.c_str());
}
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SSMusicManager::SSMusicManager()
	:	Subsystem("SSMusicManager")
	,	Subscriber("SSMusicManager")
	,	m_ScriptLoaded(false)
	,	m_ScriptEngine(nullptr)
	,	m_PatternMain(nullptr)
	,	m_PatternAdditional(nullptr)
	,	m_PatternAmbient(nullptr)
	,	m_BeatCounts(0)
	,	m_BeatsNum(0)
	,	m_BarCounter(0)
{
}
//+----------------------------------------------------------------------------+
//|void LoadPattern(unsigned int id)
//\----------------------------------------------------------------------------+
void SSMusicManager::LoadPattern(unsigned int id)
{
	Pattern pattern;

	ScriptArg* arg = fNewArray(ScriptArg, PATTERNS_ELEMENTS_MAX);

	///Get Sub category
	unsigned int size = m_ScriptEngine->PopArray(arg, 1);

	pattern.SubCat = arg->asString.c_str();

	///Get Category
	size = m_ScriptEngine->PopArray(arg, PATTERNS_ELEMENTS_MAX);

	pattern.Category = (MusicCatergory)atoi(arg->asString.c_str());

	///Get Path to music files
	size = m_ScriptEngine->PopArray(arg, PATTERNS_ELEMENTS_MAX);
	
	pattern.Path = arg->asString.c_str();

	///Get Name
	size = m_ScriptEngine->PopArray(arg, PATTERNS_ELEMENTS_MAX);

	pattern.Name = arg->asString.c_str();

	fDeleteArray(arg);

	m_Patterns.push_back(pattern);
}
//+----------------------------------------------------------------------------+
//|MusicSet* GetPattern(const rString &name)
//\----------------------------------------------------------------------------+
SSMusicManager::Pattern* SSMusicManager::GetPattern(const rString &name)
{
	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		if((*it).Name == name)
			return &(*it);
	}

	return nullptr;
}
//+----------------------------------------------------------------------------+
//|void HandleMessage(const Message* message)
//\----------------------------------------------------------------------------+
void SSMusicManager::HandleMessage(const Message* message)
{
	///Message type
	if(message->Type == MessageTypes::ORDER_UNITS)
	{
		const OrderUnitsMessage* orderUnitsMessage = static_cast<const OrderUnitsMessage*> (message);

		///Only react to events on local machine
		if ( orderUnitsMessage->TeamID == g_PlayerData.GetPlayerID() )
		{
			///If any squad is attacking, notify the connected script
			if(orderUnitsMessage->MissionType == Squad::MISSION_TYPE_ATTACK || orderUnitsMessage->MissionType == Squad::MISSION_TYPE_ATTACK_MOVE)
			{
				m_ScriptEngine->Call("OnBattle");
				return;
			}
		}
	}
	else if(message->Type == MessageTypes::AI_MSG)
	{
		const AIMessage* aimessage = static_cast<const AIMessage*> (message);
		
		///only react to events on local machine
		if ( aimessage->TeamID == g_PlayerData.GetPlayerID() )
		{
			///if any squad is attacking us, notify the connected script
			if(static_cast<AIStateMessages>(aimessage->AIMsg) == AIStateMessages::UNDER_ATTACK)
			{
				m_ScriptEngine->Call("OnBattle");
				return;
			}
		}
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateBeatCount(void)
//|Update beat reltated stuff
//\----------------------------------------------------------------------------+
void SSMusicManager::UpdateBeatCount(void)
{
	if(g_SSAudio.GetAtBeat())
	{
		m_BeatCounts++;
		m_BarCounter++;

		///Pattern
		if(m_BeatCounts >= m_BeatsNum)
		{
			m_ScriptEngine->Call("CompletedPattern");
			m_BeatCounts = 0;
		}
		///Half Pattern
		else if(m_BeatCounts == static_cast<int>(m_BeatsNum / 2))
		{
			m_ScriptEngine->Call("CompletedPatternHalf");
		}

		///Bar
		if(m_BarCounter >= BEATS_BAR)
		{
			m_ScriptEngine->Call("CompletedBar");
			m_BarCounter = 0;
		}
	}
}
//+----------------------------------------------------------------------------+
//|void SetPatternMain(const rString &name, const float fadeOut, const float fadeIn)
//|Trigger a specific pattern as the main track using its name
//\----------------------------------------------------------------------------+
void SSMusicManager::SetPatternMain(const rString &name, const float fadeOut, const float fadeIn)
{
	///Dont set if its the same track playing that should be played
	if(m_PatternMain)
	{
		if(m_PatternMain->Name == name)
			return;
	}

	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		if((*it).Category == MusicCatergory::MAIN  && (*it).Name == name)
		{
			StartPattern((*it).Path, fadeIn);

			///Remove the previous track as the current pattern
			if(m_PatternMain)
			{
				StopPattern(m_PatternMain->Path, fadeOut);
			}
			m_PatternMain = &(*it);
			m_ScriptEngine->SetString("patternNameMain", m_PatternMain->Name);
			return;
		}
	}
}
//+----------------------------------------------------------------------------+
//|void SetPatternAdditional(const rString &name, const float fadeOut, const float fadeIn)
//|Trigger a specific pattern as the additional track using its name
//\----------------------------------------------------------------------------+
void SSMusicManager::SetPatternAdditional(const rString &name, const float fadeOut, const float fadeIn)
{
	///Dont set if its the same track playing that should be played
	if(m_PatternAdditional)
	{
		if(m_PatternAdditional->Name == name)
			return;
	}

	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		if((*it).Category == MusicCatergory::ADDITIONAL  && (*it).Name == name)
		{
			StartPattern((*it).Path, fadeIn);

			///Remove the previous track as the current pattern
			if(m_PatternAdditional)
			{
				StopPattern(m_PatternAdditional->Path, fadeOut);
			}
			m_PatternAdditional = &(*it);
			m_ScriptEngine->SetString("patternNameAdditional", m_PatternAdditional->Name);
			return;
		}
	}
}
//+----------------------------------------------------------------------------+
//|void SetPatternAmbient(const rString &name, const float fadeOut, const float fadeIn)
//|Trigger a specific pattern as the ambient track using its name
//\----------------------------------------------------------------------------+
void SSMusicManager::SetPatternAmbient(const rString &name, const float fadeOut, const float fadeIn)
{
	///Dont set if its the same track playing that should be played
	if(m_PatternAmbient)
	{
		if(m_PatternAmbient->Name == name)
			return;
	}

	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		if((*it).Category == MusicCatergory::AMBIENT  && (*it).Name == name)
		{
			StartPattern((*it).Path, fadeIn);

			///Remove the previous track as the current pattern
			if(m_PatternAmbient)
			{
				StopPattern(m_PatternAmbient->Path, fadeOut);
			}
			m_PatternAmbient = &(*it);
			m_ScriptEngine->SetString("patternNameAmbient", m_PatternAmbient->Name);
			return;
		}
	}
}
//+----------------------------------------------------------------------------+
//|void SetRandomMainPattern(const rString &subCat, const float fadeIn, const float fadeOut)
//\----------------------------------------------------------------------------+
void SSMusicManager::SetRandomMainPattern(const rString &subCat,const float fadeIn, const float fadeOut)
{
	rVector<Pattern*> tempPatterns;
	Pattern* pattern = nullptr;
	Pattern* patternPrev = nullptr;

	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		if((*it).Category == MusicCatergory::MAIN)
		{
			///If there was an input subcategory, it must match the pattern category to be picked
			if(subCat != "")
				if(subCat != (*it).SubCat)
					continue;
	
			///If there are not active pattern, just add all the matching patterns, otherwise add all patterns except the one that is active
			if(m_PatternMain == nullptr)
				tempPatterns.push_back(&(*it));
			else if((*it).Name != m_PatternMain->Name)
				tempPatterns.push_back(&(*it));
		}
	}

	if(tempPatterns.size() == 0)
	{
		return;
	}

	///If theres only one pattern top use, use that one. If theres more than one, select at random one
	if(tempPatterns.size() == 1)
	{
		pattern = tempPatterns[0];
	}
	else
	{
		const unsigned int max = static_cast<unsigned int>(tempPatterns.size() - 1); 
		const unsigned int at = g_Randomizer.UserRand(0, max);
		pattern = tempPatterns[at];
	}

	StartPattern(pattern->Path, fadeIn);

	if(m_PatternMain)
	{
		StopPattern(m_PatternMain->Path, fadeOut);
	}
	m_PatternMain = pattern;
	m_ScriptEngine->SetString("patternNameMain", m_PatternMain->Name);
}
//+----------------------------------------------------------------------------+
//|void SetRandomAdditionalPattern(const float fadeIn, const float fadeOut)
//\----------------------------------------------------------------------------+
void SSMusicManager::SetRandomAdditionalPattern(const float fadeIn, const float fadeOut)
{
	rVector<Pattern*> tempPatterns;
	Pattern* pattern = nullptr;
	Pattern* patternPrev = nullptr;

	for(rVector<Pattern>::iterator it = m_Patterns.begin(); it != m_Patterns.end(); it++)
	{
		if((*it).Category == MusicCatergory::ADDITIONAL)
		{
			///If ther are not active pattern, just add them all in, otherwise add all patterns except the one that is active
			if(m_PatternAdditional == nullptr)
				tempPatterns.push_back(&(*it));
			else if((*it).Name != m_PatternAdditional->Name)
				tempPatterns.push_back(&(*it));
		}
	}

	if(tempPatterns.size() == 0)
	{
		return;
	}

	///If theres only one pattern top use, use that one. If theres more than one, select at random one
	if(tempPatterns.size() == 1)
	{
		pattern = tempPatterns[0];
	}
	else
	{
		const unsigned int max = static_cast<unsigned int>(tempPatterns.size() - 1); 
		const unsigned int at = g_Randomizer.UserRand(0, max);
		pattern = tempPatterns[at];
	}

	StartPattern(pattern->Path, fadeIn);

	if(m_PatternAdditional)
	{
		StopPattern(m_PatternAdditional->Path, fadeOut);
	}
	m_PatternAdditional = pattern;
	m_ScriptEngine->SetString("patternNameAdditional", m_PatternAdditional->Name);
}
//+----------------------------------------------------------------------------+
//|void KillAllPatterns(void)
//\----------------------------------------------------------------------------+
void SSMusicManager::KillAllPatterns(void)
{
}
//+----------------------------------------------------------------------------+
//|void ResetBeatCounter(void)
//\----------------------------------------------------------------------------+
void SSMusicManager::ResetBeatCounter(void)
{
	m_BeatCounts = 0;
	m_BarCounter = 0;
}
//+----------------------------------------------------------------------------+
//|void StartPattern(const rString &path, const float fadeIn)
//\----------------------------------------------------------------------------+
void SSMusicManager::StartPattern(const rString &path, const float fadeIn)
{
	MusicEvent event;
	event.AtBeat = BeatType::BEAT_100;
	event.Type = MusicEventType::ADD;
	event.Name = path;
	event.FadeDuration = fadeIn;	
	g_SSAudio.PostEventMusic(event);
}
//+----------------------------------------------------------------------------+
//|void StopPattern(const rString &path, const float fadeOut)
//\----------------------------------------------------------------------------+
void SSMusicManager::StopPattern(const rString &path, const float fadeOut)
{
	MusicEvent event;
	event.AtBeat = BeatType::BEAT_100;
	event.Type = MusicEventType::STOP;
	event.Name = path;
	event.FadeDuration = fadeOut;
	g_SSAudio.PostEventMusic(event);
}
//+----------------------------------------------------------------------------+
//|int ScriptSetPatternMain(IScriptEngine* engine)
//|Trigger a specific pattern as the main track using its name
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptSetPatternMain(IScriptEngine* engine)
{
	float fadeOut = m_ScriptEngine->PopFloat();
	float fadeIn = m_ScriptEngine->PopFloat();
	rString name = m_ScriptEngine->PopString();
	SetPatternMain(name, fadeOut, fadeIn);

	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptSetPatternAdditional(IScriptEngine* engine)
//|Trigger a specific pattern as the additional track using its name
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptSetPatternAdditional(IScriptEngine* engine)
{
	float fadeOut = m_ScriptEngine->PopFloat();
	float fadeIn = m_ScriptEngine->PopFloat();
	rString name = m_ScriptEngine->PopString();
	SetPatternAdditional(name, fadeOut, fadeIn);

	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptSetPatternAmbient(IScriptEngine* engine)
//|Trigger a specific pattern as the ambient track using its name
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptSetPatternAmbient(IScriptEngine* engine)
{
	float fadeOut = m_ScriptEngine->PopFloat();
	float fadeIn = m_ScriptEngine->PopFloat();
	rString name = m_ScriptEngine->PopString();
	SetPatternAmbient(name, fadeOut, fadeIn);

	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptSetRandomMainPattern(IScriptEngine* engine)
//|Find all the patterns that belongs to the input category and choose one of them to play
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptSetRandomMainPattern(IScriptEngine* engine)
{
	float fadeOut = m_ScriptEngine->PopFloat();
	float fadeIn = m_ScriptEngine->PopFloat();
	rString subcat = m_ScriptEngine->PopString();
	SetRandomMainPattern(subcat, fadeIn, fadeOut);

	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptSetRandomAdditionalPattern(IScriptEngine* engine)
//|Find all the patterns that belongs to the input category and choose one of them to play
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptSetRandomAdditionalPattern(IScriptEngine* engine)
{
	float fadeOut = m_ScriptEngine->PopFloat();
	float fadeIn = m_ScriptEngine->PopFloat();
	SetRandomAdditionalPattern(fadeIn, fadeOut);

	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptKillAllPatterns(IScriptEngine* engine)
//|Find all the patterns that belongs to the input category and stop them
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptKillAllPatterns(IScriptEngine* engine)
{
	KillAllPatterns();

	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptResetBeatCounter(IScriptEngine* engine)
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptResetBeatCounter(IScriptEngine* engine)
{
	ResetBeatCounter();
	return 0;
}
//+----------------------------------------------------------------------------+
//|int ScriptGetRandomInt(IScriptEngine* engine)
//\----------------------------------------------------------------------------+
int SSMusicManager::ScriptGetRandomInt(IScriptEngine* engine)
{
	int to = m_ScriptEngine->PopInt();
	int from = m_ScriptEngine->PopInt();

	int out = g_Randomizer.UserRand(from, to);
	m_ScriptEngine->PushInt(out);

	return 1;
}
