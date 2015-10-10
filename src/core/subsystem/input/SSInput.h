/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson & Johan Melin
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <memory/Alloc.h>
#include <glm/glm.hpp>

#define g_SSInput SSInput::GetInstance()

class SSInput : public Subsystem
{
public:
	static SSInput& GetInstance( );
	
	void Startup();
	void UpdateUserLayer( const float deltaTime )	override;

private:
	// No external instancing allowed
	SSInput( ) : Subsystem( "Input" ) { }
	SSInput( const SSInput& rhs );
	~SSInput( ) { }
	SSInput& operator=(const SSInput& rhs);

	void CheckActions();
	void CheckCTRLCommands();
	void CheckShiftCommands();
	void SendOrder(int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand);
	void SendOrderInvoke(const rVector<int>& squads, int command, const int teamID, int genericValue);
	void SelectControlGroup( short controlGroupID );
};