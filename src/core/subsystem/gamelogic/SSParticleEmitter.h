/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <memory/Alloc.h>

#include "../../component/ParticleEmitterComponent.h"

#define g_SSParticleEmitter SSParticleEmitter::GetInstance()

//+===+=========================+======================
//----| SSParticleEmitter|
//----+-------------------------+----------------------
//+===+================================================
class SSParticleEmitter :
	public Subsystem
{
//Public Functions	 
public:
	static SSParticleEmitter&	GetInstance(void);
								~SSParticleEmitter();

	void						Startup(void) override;
	void						Shutdown(void) override;

	void						UpdateUserLayer(const float deltaTime) override;
	
//Private Functions
private:
								SSParticleEmitter();
								SSParticleEmitter(const SSParticleEmitter &ref);
	SSParticleEmitter&			operator=(const SSParticleEmitter &ref);
		
	void						HandleEvents(const int entityID, ParticleEmitterComponent* &sfxEmitter);

//Private Variables
private:
};