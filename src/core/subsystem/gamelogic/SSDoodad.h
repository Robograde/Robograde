/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once

#include "../Subsystem.h"

#define g_SSDoodad SSDoodad::GetInstance()		// Conveniency access function

class SSDoodad : public Subsystem
{
public:
	static SSDoodad&	GetInstance();

	void						Startup				( ) override;
	void						UpdateUserLayer		( const float deltaTime ) override;
	void						Shutdown			( ) override;

private:
			
	float m_Wind = 0.0f;
	// No external instancing allowed
	SSDoodad() : Subsystem("SSDoodad") {}
	SSDoodad(const SSDoodad & rhs);
	~SSDoodad() {};
	SSDoodad&			operator=			(const SSDoodad & rhs);
};