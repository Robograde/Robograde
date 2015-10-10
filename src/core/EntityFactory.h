/**************************************************
2015 Robograde Development Team
***************************************************/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <script/ScriptEngine.h>
#include "datadriven/ComponentTypes.h"
#include "component/WeaponComponent.h"
#include "component/OwnerComponent.h"
#include "component/SFXEmitterComponent.h"

#define DEFAULT_ORIENTATION	glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
#define DEFAULT_SCALE		glm::vec3(1.0f)

class EntityFactory
{
	public:
		static Entity CreateSquadUnit				( float posX, float posZ, int squadID, int ownerID );
		static Entity CreateSquadUnitWithModules	( float posX, float posZ, int squadID, int ownerID );
		static Entity CreateSquadUnitCopy			( Entity originalEntity );
		static Entity CreateSquadUnitModule			( Entity parent, int ownerID, int upgradeName);
		static Entity CreateTerrain();
		
		static Entity CreateResource				( float posX , float posZ , const glm::vec3& scale , const glm::quat& orientation , const rString& modelFileName, int spawnedBy = -1 );
		static Entity CreateProp					( float posX , float posZ , const glm::vec3& scale , const glm::quat& orientation , bool blockPath , const rString& modelFileName , int radius , const bool isTree );
		static Entity CreateAnimatedProp			( float posX , float posZ , const glm::vec3& scale , const glm::quat& orientation , bool blockPath , const rString& bindMeshFileName , const rString& animFileName , const int radius );
		static Entity CreateCombinedProp			( float posX , float posY , float posZ , const glm::vec3& scale , const glm::quat& orientation , const rString& modelFileName );
		static Entity CreateControlPoint			( float posX , float posZ , const glm::vec3& scale , const glm::quat& orientation , short OwnerID , const rString& modelFileName );
		static Entity CreateSoundEmitterObject		( const glm::vec3& pos, rString name, const float distanceMin, const float distanceMax, const bool looping, const float timeInterval, const bool editor);
		static Entity CreateParticleEmitterObject	(const glm::vec3& pos, rString name, const short particleType, glm::vec4 color);
		static Entity CreatePlacementGhostObject	();
		static void ScriptInitializeBindings();

	private:

		static int ScriptCreateSquadUnit		( IScriptEngine* scriptEngine );
		static int ScriptCreateSquadUnitModule	( IScriptEngine* scriptEngine );
		static int ScriptCreateTerrain			( IScriptEngine* scriptEngine );
		static int ScriptCreateResource			( IScriptEngine* scriptEngine );
		static int ScriptCreateProp				( IScriptEngine* scriptEngine );
		static int ScriptCreateControlPoint		( IScriptEngine* scriptEngine );
};
