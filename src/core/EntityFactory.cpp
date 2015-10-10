/**************************************************
2015 Robograde Development Team
***************************************************/

#include "EntityFactory.h"
#include <bitset>
#include <collisionDetection/ICollisionDetection.h>
#include <gfx/ModelBank.h>
#include <gfx/AnimationBank.h>
#include <gfx/GraphicsEngine.h>
#include <utility/Colours.h>
#include <utility/Logger.h>
#include "utility/GameModeSelector.h"
#include "datadriven/EntityManager.h"
#include "datadriven/DenseComponentCollection.h"
#include "subsystem/gamelogic/SSAI.h"
#include "subsystem/gamelogic/SSControlPoint.h"
#include "subsystem/audio/SSSFXEmitter.h"
#include "subsystem/gamelogic/SSUpgrades.h"
#include "subsystem/gamelogic/SSControlPoint.h"
#include "component/AgentComponent.h"
#include "component/PlacementComponent.h"
#include "component/ModelComponent.h"
#include "component/CollisionComponent.h"
#include "component/ResourceComponent.h"
#include "component/HealthComponent.h"
#include "component/SelectionComponent.h"
#include "component/ColourComponent.h"
#include "component/ChildComponent.h"
#include "component/ParentComponent.h"
#include "component/ControlPointComponent.h"
#include "component/UpgradeComponent.h"
#include "component/VisionComponent.h"
#include "component//AnimationComponent.h"
#include "component/TerrainFollowComponent.h"
#include "component/DecalComponent.h"
#include "component/StealthComponent.h"
#include "component/FollowMouseComponent.h"
#include "component/ParticleEmitterComponent.h"

#include "picking/PickingType.h"
#include "utility/GameData.h"
#include "utility/PlayerData.h"
#include "ai/Terrain.h"
#include <utility/Randomizer.h>
#include <gfx/DecalManager.h> //TODOHJ: Remove this when there is a decal subsystem
#include <glm/gtx/transform.hpp> //TODOHJ: Same here as ^
#include "utility/Alliances.h"

#define UNIT_SCALE_FACTOR 2.0f
Entity EntityFactory::CreateSquadUnit(float posX, float posZ, int squadID, int ownerID)
{
	Entity entity = g_EntityManager.CreateEntity();
	
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<PlacementComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ModelComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<AgentComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<CollisionComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<HealthComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<SFXEmitterComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<SelectionComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<OwnerComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ColourComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ParentComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<VisionComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<TerrainFollowComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<DecalComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<StealthComponent>( ) );

	Tile* freeTile = Terrain::GetInstance()->GetSurroundingFreeTile( posX, posZ, ENTITY_INVALID );
	// AISubsystem will destruct the agent.
	Agent* agent = tNew( Agent, freeTile->X, freeTile->Y, squadID, entity, ownerID );
	GetDenseComponent<AgentComponent>( entity )->Agent = agent;

	g_SSAI.AddAgent( agent );

	PlacementComponent* placementComp = GetDenseComponent<PlacementComponent>(entity);
	placementComp->Scale = glm::vec3(UNIT_SCALE_FACTOR);

	ModelComponent* modelcomp = GetDenseComponent<ModelComponent>(entity);
	modelcomp->ModelHandle = gfx::g_ModelBank.LoadModel("Robot1Shape.robo");
	modelcomp->Min = gfx::g_ModelBank.FetchModel(modelcomp->ModelHandle).Min;
	modelcomp->Max = gfx::g_ModelBank.FetchModel(modelcomp->ModelHandle).Max;

	const float entityCollisionRadius	= modelcomp->Max.z;

	DecalComponent* decal = GetDenseComponent<DecalComponent>( entity );
	decal->Decal.Texture = g_DecalManager.GetTextureAtlas()->GetHandle("circle.png");
	decal->Decal.Tint = glm::vec4(1, 1, 0, 1);
	decal->Scale = (modelcomp->Max.z - modelcomp->Min.z) * UNIT_SCALE_FACTOR;
	glm::mat4 w =  glm::translate( vec3( posX, 0, posZ ) ) * glm::rotate(3.14f * 0.5f ,vec3(1,0,0)) * glm::scale( vec3( decal->Scale ) );
	decal->Decal.World = w;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData( entity );
	if ( ownerID == g_PlayerData.GetPlayerID( ) )
		collisionEntity->SetGroupID( PICKING_TYPE_CONTROLLABLE );
	else
		collisionEntity->SetGroupID( ( g_Alliances.IsAllied( g_PlayerData.GetPlayerID( ), ownerID )) ? PICKING_TYPE_FRIEND : PICKING_TYPE_ENEMY );
	g_CollisionDetection.AddCollisionVolumeSphere( collisionEntity, glm::vec3( 0.0f ), entityCollisionRadius );
	GetDenseComponent<CollisionComponent>( entity )->CollisionEntity = collisionEntity;

	SelectionComponent* selectionComponent = GetDenseComponent<SelectionComponent>(entity);
	selectionComponent->Radius		= entityCollisionRadius;
	selectionComponent->Selected	= false;
	selectionComponent->MouseOvered	= false;

	OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( entity );
	ownerComponent->OwnerID = ownerID;

	ColourComponent* colourComponent = GetDenseComponent<ColourComponent>( entity );
	colourComponent->Colour = g_GameData.GetPlayerColour( ownerID );

	// SFXEmitter - add all the triggers
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(entity);

	g_SSSFXEmitter.AddSFXTrigger(sfxEmitter, "nom0", "../../../asset/audio/collection/weapon/weapon_cutter.sfxc", SFXTriggerType::WHILE_EATING, false, 1.0f, 50.0f, 150.0f);
	g_SSSFXEmitter.AddSFXTrigger(sfxEmitter, "nom1", "../../../asset/audio/collection/robot/robot_eating.sfxc", SFXTriggerType::WHILE_EATING, false, 0.5f, 50.0f, 150.0f);

	TerrainFollowComponent* terrainFollowComponent = GetDenseComponent<TerrainFollowComponent>(entity);
	terrainFollowComponent->Offset = -modelcomp->Min.y;

	StealthComponent* stealthComponent = GetDenseComponent<StealthComponent>( entity );
	stealthComponent->CanStealth	= false;
	stealthComponent->InStealth		= false;

	return entity;
}

Entity EntityFactory::CreateSquadUnitWithModules( float posX, float posZ, int squadID, int ownerID )
{
	Entity newUnit = CreateSquadUnit( posX, posZ, squadID, ownerID );

	Entity squadBuddyID = g_SSAI.m_Teams[ownerID].Squads[squadID]->GetAgents()[0]->GetEntityID();
	ParentComponent* parent = GetDenseComponent<ParentComponent>( squadBuddyID );
	for ( int slot = 0; slot < MODULE_SLOT_SIZE; ++slot )
	{
		if ( parent->Children[slot] != ENTITY_INVALID )
		{
			UpgradeComponent* upgrade = GetDenseComponent<UpgradeComponent>( parent->Children[slot] );
			EntityFactory::CreateSquadUnitModule( newUnit, ownerID, upgrade->UpgradeDataID );
		}
	}

	return newUnit;
}

Entity EntityFactory::CreateSquadUnitCopy( Entity originalEntity )
{
	if ( !HasComponent<AgentComponent>( originalEntity ) )
	{
		return ENTITY_INVALID;	// TODOOE: Write warning message here.
	}
	Agent*	originalAgent	= GetDenseComponent<AgentComponent>( originalEntity )->Agent;
	Tile*	originalTile	= originalAgent->GetTile();

	Entity newUnit = CreateSquadUnit( static_cast<float>(originalTile->X), static_cast<float>(originalTile->Y), originalAgent->GetSquadID(), originalAgent->GetTeam() );
	ParentComponent* parent = GetDenseComponent<ParentComponent>( originalEntity );
	for ( int slot = 0; slot < MODULE_SLOT_SIZE; ++slot )
	{
		if ( parent->Children[slot] != ENTITY_INVALID )
		{
			UpgradeComponent* upgrade = GetDenseComponent<UpgradeComponent>( parent->Children[slot] );
			EntityFactory::CreateSquadUnitModule( newUnit, originalAgent->GetTeam(), upgrade->UpgradeDataID );
		}
	}

	return newUnit;
}

Entity EntityFactory::CreateSquadUnitModule( Entity parent, int ownerID, int upgradeName)
{
	Entity entity	= g_EntityManager.CreateEntity();
	int slot		= g_SSUpgrades.GetUpgradeSlot( upgradeName );
	int upgradeType = g_SSUpgrades.GetUpgradeType( upgradeName );


	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ChildComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<PlacementComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ModelComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ColourComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<CollisionComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<UpgradeComponent>());
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<SFXEmitterComponent>());

	UpgradeData& upgradeData = g_SSUpgrades.GetUpgrade( upgradeName );
	if ( upgradeData.IsAnimated )
	{
		g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<AnimationComponent>() );
		AnimationComponent* animationComponent = GetDenseComponent<AnimationComponent>( entity );
		for ( auto& pair : upgradeData.AnimationNames )
		{
			gfx::AnimationHandle animHandle = gfx::g_AnimationBank.LoadAnimation( pair.first.c_str() );
			animationComponent->HandleMap[pair.second] = animHandle;
			animationComponent->Playing = true;
			animationComponent->CurrentAnimation = gfx::AnimationType::Idle;
			animationComponent->CurrentTime = g_Randomizer.UserRand( 0, 1000 ) / 100.0f;
		}
	}

	switch (upgradeType)
	{
		case UPGRADE_TYPE_WEAPON:
		{
			SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(entity);
			g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<WeaponComponent>());
			WeaponComponent* wc = GetDenseComponent<WeaponComponent>(entity);
			wc->LoadComponentStats( upgradeName, g_SSUpgrades.GetUpgrade( upgradeName ) );
			Agent* agent = GetAgentPointer(parent);

			if (agent != nullptr)
				agent->AddWeapon(wc);
	
			///Get sfxpath from weapon component and add a trigger to activate when the weapon is used
			g_SSSFXEmitter.AddSFXTrigger(sfxEmitter, "weaponSFX",wc->SfxPath, SFXTriggerType::WHILE_ATTACKING, false, 0.0f, 0.0f, 250.0f);
		}
		break;

		default:
		{

		}
		break;

	}

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData(entity);
	collisionEntity->SetGroupID(PICKING_TYPE_UNIT_PROP);
	g_CollisionDetection.AddCollisionVolumeSphere(collisionEntity, glm::vec3(0.0f), 1);
	GetDenseComponent<CollisionComponent>(entity)->CollisionEntity = collisionEntity;

	UpgradeComponent* upgradeComponent = GetDenseComponent<UpgradeComponent>(entity);
	upgradeComponent->UpgradeDataID = upgradeName;
	upgradeComponent->Changed		= true;

	ChildComponent* childComponent = GetDenseComponent<ChildComponent>( entity );
	childComponent->Parent = parent;
	ParentComponent* parentComponent = GetDenseComponent<ParentComponent>(parent);
	PlacementComponent* pc = GetDenseComponent<PlacementComponent>(entity);

	// TODOMH: Scaling should be removed (or set to 1). Will offset stille be needed ? // DP
	switch ( slot )
	{
		case MODULE_SLOT_TOP:
			childComponent->Offset = glm::vec3(0, 0.0f, 0.0f);
			parentComponent->Children[MODULE_SLOT_TOP] = entity;
			pc->Scale = glm::vec3(UNIT_SCALE_FACTOR);
		break;

		case MODULE_SLOT_BACK:
			childComponent->Offset = glm::vec3(0, 0, 0);
			parentComponent->Children[MODULE_SLOT_BACK] = entity;
			pc->Scale = glm::vec3(UNIT_SCALE_FACTOR);
		break;

		case MODULE_SLOT_SIDES:
			childComponent->Offset = glm::vec3(0, 0, 0);
			parentComponent->Children[MODULE_SLOT_SIDES] = entity;
			pc->Scale = glm::vec3(UNIT_SCALE_FACTOR);
		break;

		case MODULE_SLOT_FRONT:
			childComponent->Offset = glm::vec3(0, 0, 0);
			parentComponent->Children[MODULE_SLOT_FRONT] = entity;
			pc->Scale = glm::vec3(UNIT_SCALE_FACTOR);
		break;
	}

	ModelComponent* modelcomp = GetDenseComponent<ModelComponent>(entity);
	modelcomp->ModelHandle = gfx::g_ModelBank.LoadModel( g_SSUpgrades.GetUpgrade( upgradeName ).ModelName.c_str( ) );
	modelcomp->Min = gfx::g_ModelBank.FetchModel(modelcomp->ModelHandle).Min;
	modelcomp->Max = gfx::g_ModelBank.FetchModel(modelcomp->ModelHandle).Max;

	ColourComponent* colourComponent = GetDenseComponent<ColourComponent>( entity );
	colourComponent->Colour = g_GameData.GetPlayerColour( ownerID );


	// Animation
	//AnimationComponent* animationComponent = GetDenseComponent<AnimationComponent>( entity );
	//gfx::AnimationHandle animHandle = gfx::g_AnimationBank.LoadAnimation( "IdleCycle.roboanim" ); 
	//animationComponent->HandleMap[gfx::AnimationType::Idle] = animHandle;
	//animationComponent->CurrentAnimation = gfx::AnimationType::Idle;
	//animationComponent->CurrentTime = g_Randomizer.UserRand( 0, 1000 ) / 100.0f;

	return entity;
}

Entity EntityFactory::CreateResource( float posX, float posZ, const glm::vec3& scale, const glm::quat& orientation, const rString& modelFileName, int spawnedBy )
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<PlacementComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ModelComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ResourceComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<CollisionComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<HealthComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<SFXEmitterComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<SelectionComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<DoodadComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<TerrainFollowComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<DecalComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ColourComponent>() );

	ModelComponent* mc = GetDenseComponent<ModelComponent>( entity );
	mc->ModelHandle = gfx::g_ModelBank.LoadModel( modelFileName.c_str() );
	mc->Min = gfx::g_ModelBank.FetchModel(mc->ModelHandle).Min;
	mc->Max = gfx::g_ModelBank.FetchModel(mc->ModelHandle).Max;

	const float hitboxRadius = glm::max( mc->Max.x,
		glm::max( -mc->Min.x, 
		glm::max( mc->Max.y, 
		glm::max( -mc->Min.y,
		glm::max( mc->Max.z, -mc->Min.z ) ) ) ) );

	HealthComponent* hc = GetDenseComponent<HealthComponent>( entity );
	hc->Health = 200;
	hc->MaxHealth = 200;

	//TODOHJ: its probably the resource site that should have a decal keep it like this for now
	DecalComponent* decal = GetDenseComponent<DecalComponent>( entity );
	decal->Decal.Texture = g_DecalManager.GetTextureAtlas()->GetHandle("circle.png");
	decal->Decal.Tint = glm::vec4(1, 1, 0, 1);
	decal->Scale = UNIT_SCALE_FACTOR;
	glm::mat4 w =  glm::translate(vec3(posX, 0, posZ)) * glm::rotate(3.14f * 0.5f ,vec3(1,0,0)) * glm::scale( vec3( decal->Scale ) );
	decal->Decal.World = w;

	GetDenseComponent<ResourceComponent>( entity )->EntityID = entity;
	GetDenseComponent<ResourceComponent>( entity )->SpawnedBy = spawnedBy;
	GetDenseComponent<ResourceComponent>( entity )->TilePointer = Terrain::GetInstance( )->GetTile( (int) posX, (int) posZ );

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>(entity);
	placement->Position = glm::vec3( posX, 0.0f, posZ );
	placement->Scale = scale * UNIT_SCALE_FACTOR;
	placement->Orientation = orientation;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData( entity );
	collisionEntity->SetGroupID( PICKING_TYPE_RESOURCE );
	g_CollisionDetection.AddCollisionVolumeSphere( collisionEntity, glm::vec3(0.0f), hitboxRadius );
	GetDenseComponent<CollisionComponent>( entity )->CollisionEntity = collisionEntity;

	SelectionComponent* selectionComponent = GetDenseComponent<SelectionComponent>( entity );
	selectionComponent->Radius		= hitboxRadius;
	selectionComponent->Selected	= false;
	selectionComponent->MouseOvered	= false;

	TerrainFollowComponent* terrainFollowComponent = GetDenseComponent<TerrainFollowComponent>( entity );
	terrainFollowComponent->Offset = -mc->Min.y;

	const unsigned int sensorOffset = 1;
	g_SSAI.AddToSensor( entity, ( int ) posX, ( int ) posZ, static_cast<unsigned int>( ceilf( ( scale.x + scale.z ) / 2 ) ) + sensorOffset );

	return entity;
}

Entity EntityFactory::CreateProp(float posX, float posZ, const glm::vec3& scale, const glm::quat& orientation, bool blockPath, const rString& modelPath, const int radius, const bool isTree)
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<PlacementComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ModelComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<DoodadComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<CollisionComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<TerrainFollowComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<SelectionComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ColourComponent>() );

	ModelComponent* mc = GetDenseComponent<ModelComponent>(entity);
	mc->ModelHandle = gfx::g_ModelBank.LoadModel(modelPath.c_str());
	mc->Min = gfx::g_ModelBank.FetchModel(mc->ModelHandle).Min;
	mc->Max = gfx::g_ModelBank.FetchModel(mc->ModelHandle).Max;
	 
	const float hitboxRadius = glm::max( mc->Max.x,
		glm::max( -mc->Min.x, 
		glm::max( mc->Max.y, 
		glm::max( -mc->Min.y,
		glm::max( mc->Max.z, -mc->Min.z ) ) ) ) );

	SelectionComponent* selectionComponent = GetDenseComponent<SelectionComponent>(entity);
	selectionComponent->Radius = hitboxRadius;
	selectionComponent->Selected = false;
	selectionComponent->MouseOvered = false;

	DoodadComponent* dc = GetDenseComponent<DoodadComponent>(entity);
	dc->BlockPath = blockPath;
	dc->IsTree = isTree;

	if (isTree)
	{
		dc->SwaySpeedDivisor = static_cast<float>(g_Randomizer.UserRand(1200, 2800));
		dc->AngleVector = glm::vec3(g_Randomizer.UserRand(0, 100)*0.0001, 1, g_Randomizer.UserRand(0, 100)*0.0001);
	}

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>(entity);
	placement->Position = glm::vec3( posX, 0.0f, posZ );
	placement->Scale = scale;
	placement->Orientation = orientation;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData(entity);
	collisionEntity->SetGroupID(PICKING_TYPE_PROP);
	g_CollisionDetection.AddCollisionVolumeSphere(collisionEntity, glm::vec3(0.0f), hitboxRadius);
	GetDenseComponent<CollisionComponent>(entity)->CollisionEntity = collisionEntity;

	TerrainFollowComponent* terrainFollowComponent = GetDenseComponent<TerrainFollowComponent>(entity);
	terrainFollowComponent->Offset = -mc->Min.y;

	const unsigned int sensorOffset = 1;
	g_SSAI.AddToSensor( entity, ( int )posX, ( int )posZ, static_cast< unsigned int >( ceilf( ( scale.x + scale.z ) / 2 ) ) + sensorOffset );

	return entity;
}

Entity EntityFactory::CreateAnimatedProp( float posX , float posZ , const glm::vec3& scale , const glm::quat& orientation , bool blockPath , const rString& bindMeshFileName , const rString& animFileName , const int radius )
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<PlacementComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<ModelComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<DoodadComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<CollisionComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<TerrainFollowComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<SelectionComponent>() );

	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<AnimationComponent>() );
	AnimationComponent* animationComponent = GetDenseComponent<AnimationComponent>( entity );

	gfx::AnimationHandle animHandle = gfx::g_AnimationBank.LoadAnimation( animFileName.c_str() );
	animationComponent->HandleMap[ gfx::AnimationType::Idle ] = animHandle;
	animationComponent->Playing = true;
	animationComponent->CurrentAnimation = gfx::AnimationType::Idle;
	animationComponent->CurrentTime = g_Randomizer.UserRand( 0 , 1000 ) / 100.0f;
	//animationComponent->FramesPerSec = 4;

	ModelComponent* mc = GetDenseComponent<ModelComponent>( entity );
	mc->ModelHandle = gfx::g_ModelBank.LoadModel( bindMeshFileName.c_str() );
	mc->Min = gfx::g_ModelBank.FetchModel( mc->ModelHandle ).Min;
	mc->Max = gfx::g_ModelBank.FetchModel( mc->ModelHandle ).Max;

	const float hitboxRadius = glm::max( mc->Max.x ,
		glm::max( -mc->Min.x ,
		glm::max( mc->Max.y ,
		glm::max( -mc->Min.y ,
		glm::max( mc->Max.z , -mc->Min.z ) ) ) ) );

	SelectionComponent* selectionComponent = GetDenseComponent<SelectionComponent>( entity );
	selectionComponent->Radius = hitboxRadius;
	selectionComponent->Selected = false;
	selectionComponent->MouseOvered = false;

	DoodadComponent* dc = GetDenseComponent<DoodadComponent>( entity );
	dc->BlockPath = blockPath;

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>( entity );
	placement->Position = glm::vec3( posX , 0.0f , posZ );
	placement->Scale = scale;
	placement->Orientation = orientation;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData( entity );
	collisionEntity->SetGroupID( PICKING_TYPE_PROP );
	g_CollisionDetection.AddCollisionVolumeSphere( collisionEntity , glm::vec3( 0.0f ) , hitboxRadius );
	GetDenseComponent<CollisionComponent>( entity )->CollisionEntity = collisionEntity;

	TerrainFollowComponent* terrainFollowComponent = GetDenseComponent<TerrainFollowComponent>( entity );
	terrainFollowComponent->Offset = -mc->Min.y;

	const unsigned int sensorOffset = 1;
	g_SSAI.AddToSensor( entity , ( int )posX , ( int )posZ , static_cast< unsigned int >( ceilf( ( scale.x + scale.z ) / 2 ) ) + sensorOffset );

	return entity;
}

Entity EntityFactory::CreateCombinedProp( float posX , float posY , float posZ , const glm::vec3& scale , const glm::quat& orientation , const rString& modelFileName )
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<PlacementComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<ModelComponent>() );
	g_EntityManager.AddComponent( entity , GetDenseComponentTypeIndex<CollisionComponent>() );

	ModelComponent* mc = GetDenseComponent<ModelComponent>( entity );
	mc->ModelHandle = gfx::g_ModelBank.LoadModel( modelFileName.c_str() );
	mc->Min = gfx::g_ModelBank.FetchModel( mc->ModelHandle ).Min;
	mc->Max = gfx::g_ModelBank.FetchModel( mc->ModelHandle ).Max;

	const float hitboxRadius = glm::max( mc->Max.x ,
		glm::max( -mc->Min.x ,
		glm::max( mc->Max.y ,
		glm::max( -mc->Min.y ,
		glm::max( mc->Max.z , -mc->Min.z ) ) ) ) );

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>( entity );
	placement->Position = glm::vec3( posX , posY , posZ );
	placement->Scale = scale;
	placement->Orientation = orientation;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData( entity );
	collisionEntity->SetGroupID( PICKING_TYPE_PROP );
	g_CollisionDetection.AddCollisionVolumeSphere( collisionEntity , glm::vec3( 0.0f ) , hitboxRadius );
	GetDenseComponent<CollisionComponent>( entity )->CollisionEntity = collisionEntity;

	return entity;
}

Entity EntityFactory::CreateTerrain()
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<PlacementComponent>());
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<CollisionComponent>());
	
	PlacementComponent* placement = GetDenseComponent<PlacementComponent>(entity);
	placement->Position = glm::vec3( 0.0f );

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData( entity );
	collisionEntity->SetGroupID( PICKING_TYPE_TERRAIN );
	g_CollisionDetection.AddCollisionVolumeHeightMap( collisionEntity, std::bind( &gfx::Terrain::GetHeightAtWorldCoord, &gfx::g_GFXTerrain, std::placeholders::_1, std::placeholders::_2 ) );
	GetDenseComponent<CollisionComponent>(entity)->CollisionEntity = collisionEntity;

	return entity;
}

Entity EntityFactory::CreateControlPoint( float posX, float posZ, const glm::vec3& scale, const glm::quat& orientation, short ownerID, const rString& modelFileName )
{
	Entity entity = g_EntityManager.CreateEntity( );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<PlacementComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ModelComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<CollisionComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<SelectionComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<DoodadComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<OwnerComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ColourComponent>( ) );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ControlPointComponent>());
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<VisionComponent>( ));
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<TerrainFollowComponent>());
	
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ParentComponent>());

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>( entity );
	placement->Position = glm::vec3(posX, 0.0f, posZ);
	placement->Scale	= scale;
	placement->Orientation = orientation;

	ModelComponent* modelComponent = GetDenseComponent<ModelComponent>( entity );
	modelComponent->ModelHandle = gfx::g_ModelBank.LoadModel( modelFileName.c_str( ) );
	modelComponent->Min = gfx::g_ModelBank.FetchModel(modelComponent->ModelHandle).Min;
	modelComponent->Max = gfx::g_ModelBank.FetchModel(modelComponent->ModelHandle).Max;

	//modelComponent->Min.x = -CONTROL_POINT_RADIUS;
	//modelComponent->Min.z = -CONTROL_POINT_RADIUS;
	//modelComponent->Max.x = CONTROL_POINT_RADIUS;
	//modelComponent->Max.z = CONTROL_POINT_RADIUS;

	const float hitboxRadius = glm::max( modelComponent->Max.x,
		glm::max( -modelComponent->Min.x, 
		glm::max( modelComponent->Max.y, 
		glm::max( -modelComponent->Min.y,
		glm::max( modelComponent->Max.z, -modelComponent->Min.z ) ) ) ) );

	glm::vec3 obbDirections[3] = {
		glm::vec3( 0.5f * ( modelComponent->Max.x - modelComponent->Min.x ), 0.0f, 0.0f ),
		glm::vec3( 0.0f, 0.5f * ( modelComponent->Max.y - modelComponent->Min.y ), 0.0f ),
		glm::vec3( 0.0f, 0.0f, 0.5f * ( modelComponent->Max.x - modelComponent->Min.x ) )	// Using x instead of z to stick close to the models pillar (Flag on top messes it up).
	};
	glm::vec3 obbPosition = modelComponent->Min + glm::vec3( obbDirections[0].x, obbDirections[1].y, obbDirections[2].z );

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity( );
	collisionEntity->SetUserData( entity );
	collisionEntity->SetGroupID( PICKING_TYPE_CONTROL_POINT );
	g_CollisionDetection.AddCollisionVolumeOBB( collisionEntity, obbPosition, obbDirections );
	GetDenseComponent<CollisionComponent>( entity )->CollisionEntity = collisionEntity;

	SelectionComponent* selectionComponent = GetDenseComponent<SelectionComponent>( entity );
	selectionComponent->Radius		= hitboxRadius;
	selectionComponent->Selected	= false;
	selectionComponent->MouseOvered = false;

	DoodadComponent* doodadComponent = GetDenseComponent<DoodadComponent>( entity );
	doodadComponent->BlockPath = true;

	OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( entity );
	ownerComponent->OwnerID = ownerID;

	ColourComponent* colourComponent = GetDenseComponent<ColourComponent>( entity );
	colourComponent->Colour = g_GameData.GetPlayerColour( ownerID );

	TerrainFollowComponent* terrainFollowComponent = GetDenseComponent<TerrainFollowComponent>(entity);
	terrainFollowComponent->Offset = -modelComponent->Min.y;

	const unsigned int sensorOffset = 2;
	g_SSAI.AddToSensor( entity, ( int ) posX, ( int ) posZ, static_cast<unsigned int>( ceilf( ( scale.x + scale.z ) / 2 ) ) + sensorOffset );
	g_SSControlPoint.RegisterControlPoint( entity );

	//TURRET ON CONTROL POINTS
	Entity childEntity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<ChildComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<PlacementComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<ModelComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<ColourComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<CollisionComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<UpgradeComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<SFXEmitterComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<WeaponComponent>() );
	g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<DecalComponent>() );

	if ( g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::Editor )
	{
		SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>( childEntity );
		g_EntityManager.AddComponent( childEntity, GetDenseComponentTypeIndex<WeaponComponent>() );
		WeaponComponent* wc = GetDenseComponent<WeaponComponent>( childEntity );
		wc->LoadComponentStats( childEntity, g_SSUpgrades.GetUpgrade( WEAPON_PIERCING_LASER ) );
	
		///Get sfxpath from weapon component and add a trigger to activate when the weapon is used
		g_SSSFXEmitter.AddSFXTrigger( sfxEmitter, "cpWeaponSFX", wc->SfxPath, SFXTriggerType::WHILE_ATTACKING, false, 0.0f, 0.0f, 250.0f );
	}

	UpgradeComponent* upgradeComponent = GetDenseComponent<UpgradeComponent>( childEntity );
	upgradeComponent->UpgradeDataID = WEAPON_PIERCING_LASER;
	upgradeComponent->Changed = true;

	ChildComponent* childComponent = GetDenseComponent<ChildComponent>( childEntity );
	childComponent->Parent = entity;
	ParentComponent* parentComponent = GetDenseComponent<ParentComponent>( entity );
	PlacementComponent* pc = GetDenseComponent<PlacementComponent>( entity );

	pc->Position = glm::vec3( posX, 0.0f, posZ );
	pc->Scale = scale;

	childComponent->Offset = glm::vec3( 0, 0.0f, 0.0f );
	parentComponent->Children[MODULE_SLOT_TOP] = childEntity;

	ModelComponent* modelcomp = GetDenseComponent<ModelComponent>( childEntity );
	modelcomp->ModelHandle = gfx::g_ModelBank.LoadModel( "ResearchTopShape.robo" );
	modelcomp->Min = gfx::g_ModelBank.FetchModel( modelcomp->ModelHandle ).Min;
	modelcomp->Max = gfx::g_ModelBank.FetchModel( modelcomp->ModelHandle ).Max;

	DecalComponent* decal = GetDenseComponent<DecalComponent>( childEntity );
	decal->Decal.Texture = g_DecalManager.GetTextureAtlas()->GetHandle( "circle.png" );
	decal->Decal.Tint = glm::vec4( 1, 1, 1, 1 );
	decal->Scale = CONTROL_POINT_RADIUS;
	glm::mat4 w = glm::translate( placement->Position ) * glm::rotate( 3.14f * 0.5f, vec3( 1, 0, 0 ) ) * glm::scale( vec3( decal->Scale ) );
	decal->Decal.World = w;
	decal->AlwaysDraw = true;

	ICollisionEntity* childCollisionEntity = g_CollisionDetection.CreateEntity();
	childCollisionEntity->SetUserData( childEntity );
	childCollisionEntity->SetGroupID( PICKING_TYPE_PROP );
	g_CollisionDetection.AddCollisionVolumeSphere( childCollisionEntity, glm::vec3( 0.0f ), CONTROL_POINT_RADIUS / scale.x ); //adjust hitbox so we still render decal when we have the control point outside window
	GetDenseComponent<CollisionComponent>( childEntity )->CollisionEntity = childCollisionEntity;

	colourComponent = GetDenseComponent<ColourComponent>( childEntity );
	colourComponent->Colour = g_GameData.GetPlayerColour( ownerID );

	return entity;
}

Entity EntityFactory::CreateSoundEmitterObject(const glm::vec3& pos, rString name, const float distanceMin, const float distanceMax, const bool looping, const float timeInterval, const bool editor)
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<PlacementComponent>());
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<SFXEmitterComponent>());

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>(entity);
	placement->Position = pos;
	placement->Scale = glm::vec3(1.0f);
	placement->Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	if(editor)
	{
		g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<ColourComponent>() );
		g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<ModelComponent>());
		g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<CollisionComponent>());

		ModelComponent* modelCompoent = GetDenseComponent<ModelComponent>(entity);
		modelCompoent->IsVisible = true;
		modelCompoent->ModelHandle = gfx::g_ModelBank.LoadModel("projectileCone.robo");
		modelCompoent->Min = gfx::g_ModelBank.FetchModel(modelCompoent->ModelHandle).Min;
		modelCompoent->Max = gfx::g_ModelBank.FetchModel(modelCompoent->ModelHandle).Max;

		ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
		collisionEntity->SetUserData(entity);
		collisionEntity->SetGroupID(PICKING_TYPE_SFX_EMITTER);
		g_CollisionDetection.AddCollisionVolumeSphere(collisionEntity, glm::vec3(0.0f), 1);
		GetDenseComponent<CollisionComponent>(entity)->CollisionEntity = collisionEntity;
	}
	
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(entity);
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<WeaponComponent>());
	g_SSSFXEmitter.AddSFXTrigger(sfxEmitter, "defaultSFX", name, SFXTriggerType::AT_CREATION, looping, timeInterval, distanceMin, distanceMax);

	return entity;
}

Entity EntityFactory::CreateParticleEmitterObject(const glm::vec3& pos, rString name, const short particleType, glm::vec4 color)
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<PlacementComponent>());
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<ParticleEmitterComponent>());

	PlacementComponent* placement = GetDenseComponent<PlacementComponent>(entity);
	placement->Position = pos;
	placement->Scale = glm::vec3(1.0f);
	placement->Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<ColourComponent>() );
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<ModelComponent>());
	g_EntityManager.AddComponent(entity, GetDenseComponentTypeIndex<CollisionComponent>());

	ModelComponent* modelCompoent = GetDenseComponent<ModelComponent>(entity);
	modelCompoent->IsVisible = true;
	modelCompoent->ModelHandle = gfx::g_ModelBank.LoadModel("projectileCone.robo");
	modelCompoent->Min = gfx::g_ModelBank.FetchModel(modelCompoent->ModelHandle).Min;
	modelCompoent->Max = gfx::g_ModelBank.FetchModel(modelCompoent->ModelHandle).Max;

	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData(entity);
	collisionEntity->SetGroupID(PICKING_TYPE_PARTICLE_EMITTER);
	g_CollisionDetection.AddCollisionVolumeSphere(collisionEntity, glm::vec3(0.0f), 1);
	GetDenseComponent<CollisionComponent>(entity)->CollisionEntity = collisionEntity;

	ParticleEmitterComponent* emitter = GetDenseComponent<ParticleEmitterComponent>(entity);
	emitter->Pos = pos;
	emitter->Name = name;
	emitter->ParticleType = static_cast<ParticleEmitterType>(particleType);
	emitter->Col = color;
		
	return entity;
}

Entity EntityFactory::CreatePlacementGhostObject()
{
	Entity entity = g_EntityManager.CreateEntity();

	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<PlacementComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ModelComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<FollowMouseComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<ColourComponent>() );
	g_EntityManager.AddComponent( entity, GetDenseComponentTypeIndex<CollisionComponent>() );

	// Model component
	GetDenseComponent<ModelComponent>( entity )->IsVisible = false;

	// Collision component
	ICollisionEntity* collisionEntity = g_CollisionDetection.CreateEntity();
	collisionEntity->SetUserData( entity );
	collisionEntity->SetGroupID( PICKING_TYPE_GHOST_PLACEMENT_OBJECT );
	g_CollisionDetection.AddCollisionVolumeSphere( collisionEntity, glm::vec3( 0.0f ), 1 );
	GetDenseComponent<CollisionComponent>( entity )->CollisionEntity = collisionEntity;

	return entity;
}

void EntityFactory::ScriptInitializeBindings()
{
	g_Script.Register( "GE_CreateSquadUnit",		std::bind( EntityFactory::ScriptCreateSquadUnit,		std::placeholders::_1 ) );
	g_Script.Register( "GE_CreateSquadUnitModule",	std::bind( EntityFactory::ScriptCreateSquadUnitModule,	std::placeholders::_1 ) );
	g_Script.Register( "GE_CreateTerrain",			std::bind( EntityFactory::ScriptCreateTerrain,			std::placeholders::_1 ) );
	g_Script.Register( "GE_CreateResource",			std::bind( EntityFactory::ScriptCreateResource,			std::placeholders::_1 ) );
	g_Script.Register( "GE_CreateProp",				std::bind( EntityFactory::ScriptCreateProp,				std::placeholders::_1 ) );
	g_Script.Register( "GE_CreateControlPoint",		std::bind( EntityFactory::ScriptCreateControlPoint,		std::placeholders::_1 ) );
}

int EntityFactory::ScriptCreateSquadUnit( IScriptEngine* scriptEngine )
{
	int		team	= scriptEngine->PopInt();
	int		squadID	= scriptEngine->PopInt();
	float	posZ	= scriptEngine->PopFloat();
	float	posX	= scriptEngine->PopFloat();

	CreateSquadUnitWithModules( posX, posZ, squadID, team );

	return 0;	// Number of return values.
}

int EntityFactory::ScriptCreateSquadUnitModule( IScriptEngine* scriptEngine )
{
	int ownerID		= scriptEngine->PopInt();
	int upgradeName	= scriptEngine->PopInt();
	Entity parent	= scriptEngine->PopInt();

	CreateSquadUnitModule( parent, upgradeName, ownerID ); // TODOOE: FIX? //DP

	return 0;	// Number of return values.
}

int EntityFactory::ScriptCreateTerrain( IScriptEngine* scriptEngine )
{
	CreateTerrain();

	return 0;	// Number of return values.
}

int EntityFactory::ScriptCreateResource( IScriptEngine* scriptEngine )
{
	rString		modelPath	    = scriptEngine->PopString();
	glm::quat	orientation;
				orientation.z	= scriptEngine->PopFloat();
				orientation.y	= scriptEngine->PopFloat();
				orientation.x	= scriptEngine->PopFloat();
				orientation.w	= scriptEngine->PopFloat();
	glm::vec3	scale;
				scale.z			= scriptEngine->PopFloat();
				scale.y			= scriptEngine->PopFloat();
				scale.x			= scriptEngine->PopFloat();
	float		posZ			= scriptEngine->PopFloat();
	float		posX			= scriptEngine->PopFloat();

	CreateResource( posX , posZ , scale , orientation , modelPath );

	return 0;	// Number of return values.
}

int EntityFactory::ScriptCreateProp( IScriptEngine* scriptEngine )
{
	int			radius			= scriptEngine->PopInt();
	rString		modelPath		= scriptEngine->PopString();
	bool		blockPath		= scriptEngine->PopBool();
	glm::quat	orientation;
				orientation.z	= scriptEngine->PopFloat();
				orientation.y	= scriptEngine->PopFloat();
				orientation.x	= scriptEngine->PopFloat();
				orientation.w	= scriptEngine->PopFloat();
	glm::vec3	scale;
				scale.z			= scriptEngine->PopFloat();
				scale.y			= scriptEngine->PopFloat();
				scale.x			= scriptEngine->PopFloat();
	float		posZ			= scriptEngine->PopFloat();
	float		posX			= scriptEngine->PopFloat();

	CreateProp( posX, posZ, scale, orientation, blockPath, modelPath, radius ,false);

	return 0;	// Number of return values.
}

int EntityFactory::ScriptCreateControlPoint( IScriptEngine* scriptEngine )
{
	rString		modelPath		= scriptEngine->PopString();
	short		ownerID			= scriptEngine->PopInt();
	glm::quat	orientation;
				orientation.z	= scriptEngine->PopFloat();
				orientation.y	= scriptEngine->PopFloat();
				orientation.x	= scriptEngine->PopFloat();
				orientation.w	= scriptEngine->PopFloat();
	glm::vec3	scale;
				scale.z			= scriptEngine->PopFloat();
				scale.y			= scriptEngine->PopFloat();
				scale.x			= scriptEngine->PopFloat();
	float		posZ			= scriptEngine->PopFloat();
	float		posX			= scriptEngine->PopFloat();

	CreateControlPoint( posX, posZ, scale, orientation, ownerID, modelPath );

	return 0;	// Number of return values.
}