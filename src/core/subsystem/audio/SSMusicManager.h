/**************************************************
Copyright 2015 Jens Stjernkvist
***************************************************/

#pragma once
#include <vector>
#include <string>
#include <messaging/Subscriber.h>
#include <memory/Alloc.h>
#include "../Subsystem.h"
#include "SSAudio.h"

#define g_SSMusicManager SSMusicManager::GetInstance()

//----| MusicCatergory
enum class MusicCatergory : unsigned int
{
	MAIN,		//"Background music"
	ADDITIONAL,	//Patterns used independently of other patterns, bringing lead melody to the music 
	STINGER,	//Only about a bar long. Used to get smaller variations in the music. Is used independently of other patterns
	AMBIENT,	//Patterns used for plain looping enviroment sound like rain, forest birds, wind etc. Independant from the current status
};

//+===+=================+==============================
//----| SSMusicManager	|
//----+-----------------+------------------------------
//----|A.k.a "The conductor" 
//----|Handles which music to be played in a scene, when the layers of the music should swap by fading in / out etc
//+===+================================================
class SSMusicManager :
	public Subsystem,
	public Subscriber
{
	//----+-----------------+------------------------------
	//----| Pattern			|
	//----| A musictrack bundled together with a category
	//----+-----------------+------------------------------
	struct Pattern
	{
		rString				Name;
		rString				Path;
		MusicCatergory		Category;
		rString				SubCat;
	};

//Public Functions
public:
	static SSMusicManager&	GetInstance(void);
							~SSMusicManager();

	void					Startup(void) override;
	void					Shutdown(void) override;
	void					Reset(void);

	void					UpdateUserLayer(const float deltaTime) override;
	void					Activate(void);
	const bool				LoadSceneMusic(const rString &path);
	void					UnloadAllSceneMusic(void);

	///Setters

	///Getters
	rString					GetName(void) const;
	rString					GetScriptStringValue(const rString &varName);

//Private Functions
private:
							SSMusicManager();
							SSMusicManager(const SSMusicManager &ref);
	SSMusicManager&			operator=(const SSMusicManager &ref);
	void					LoadPattern(unsigned int id);

	Pattern*				GetPattern(const rString &name);

	void					HandleMessage(const Message* message);
	void					UpdateBeatCount(void);

	void					SetPatternMain(const rString &name, const float fadeOut, const float fadeIn);
	void					SetPatternAdditional(const rString &name, const float fadeOut, const float fadeIn);
	void					SetPatternAmbient(const rString &name, const float fadeOut, const float fadeIn);
	void					SetRandomMainPattern(const rString &subCat, const float fadeIn, const float fadeOut);
	void					SetRandomAdditionalPattern(const float fadeIn, const float fadeOut);
	void					KillAllPatterns(void);
	void					ResetBeatCounter(void);

	void					StartPattern(const rString &path, const float fadeIn);
	void					StopPattern(const rString &path, const float fadeOut);

	///Script functions
	int						ScriptSetPatternMain(class IScriptEngine* engine);
	int						ScriptSetPatternAdditional(class IScriptEngine* engine);
	int						ScriptSetPatternAmbient(class IScriptEngine* engine);
	int						ScriptSetRandomMainPattern(class IScriptEngine* engine);
	int						ScriptSetRandomAdditionalPattern(class IScriptEngine* engine);
	int						ScriptKillAllPatterns(class IScriptEngine* engine);
	int						ScriptResetBeatCounter(class IScriptEngine* engine);
	int						ScriptGetRandomInt(class IScriptEngine* engine);

//Private Variables
private:
	bool					m_ScriptLoaded;
	rString					m_Name;
	rVector<Pattern>		m_Patterns;
	class ScriptEngine*		m_ScriptEngine;

	Pattern*				m_PatternMain;
	Pattern*				m_PatternAdditional;
	Pattern*				m_PatternAmbient;
	unsigned int			m_BeatCounts,
							m_BeatsNum,
							m_BarCounter;			///Counting beats until a bar is reached

};