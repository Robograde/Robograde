/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <memory/Alloc.h>

#include "../../component/SFXEmitterComponent.h"

#define g_SSSFXEmitter SSSFXEmitter::GetInstance()

//+===+=========================+======================
//----| SFXEmitterSubsystem	|
//----+-------------------------+----------------------
//----|The manager of all the SFXemitters, requesting the AudioSubsystem to play the sound effects
//+===+================================================
class SSSFXEmitter :
	public Subsystem
{
//Public Functions	 
public:
	static SSSFXEmitter&	GetInstance(void);
							~SSSFXEmitter();

	void					Startup(void) override;
	void					Shutdown(void) override;

	void					UpdateUserLayer(const float deltaTime) override;

	void					AddSFXTrigger(SFXEmitterComponent* &sfxEmitter, const rString &name, const rString &path, SFXTriggerType triggerType, const bool Looping,
											const float timeInterval, const float distanceMin, const float distanceMax);

	bool					RemoveSFXTrigger(SFXEmitterComponent* &sfxEmitter, const rString &name);

	void					AddEvent(SFXEmitterComponent* &sfxEmitter, const SFXTriggerType &type);
	
//Private Functions
private:
							SSSFXEmitter();
							SSSFXEmitter(const SSSFXEmitter &ref);
	SSSFXEmitter&			operator=(const SSSFXEmitter &ref);
		
	void					HandleEvents(const int entityID, SFXEmitterComponent* &sfxEmitter);
	void					UpdateTriggeredSFX(const float deltaTime, const int entityID, SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger);
	void					PlaySFXTrigger(SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger);

//Private Variables
private:
};