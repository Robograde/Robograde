/**************************************************
2015 Robograde Development Team
***************************************************/

#include "SSGFXLoader.h"
#include <gfx/MaterialBank.h>
#include <gfx/ModelBank.h>
#include "../../CompileFlags.h"
#include <utility/ConfigManager.h>
SSGFXLoader& SSGFXLoader::GetInstance()
{
	static SSGFXLoader instance;
	return instance;
}

void SSGFXLoader::Startup()
{
		gfx::g_MaterialBank.LoadMaterials( "materials.matrob" );
#ifdef LOW_RES_TEXTURES
		gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/albedo_LowRes.json", gfx::Texture_Atlas_Type::Diffuse);
		gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/normal_LowRes.json", gfx::Texture_Atlas_Type::Normal);
		gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/roughness_LowRes.json", gfx::Texture_Atlas_Type::Roughness);
		gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/metal_LowRes.json", gfx::Texture_Atlas_Type::Metal);
		gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/glow_LowRes.json", gfx::Texture_Atlas_Type::Glow);
#else
		CallbackConfig* gfxConfig = g_ConfigManager.GetConfig( "graphics.cfg" );
		int quality = gfxConfig->GetInt("TextureQuality", 1, "Which quality the textures should have low medium high");

		if(quality == 0)
		{
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/albedo_low.json", gfx::Texture_Atlas_Type::Diffuse);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/normal_low.json", gfx::Texture_Atlas_Type::Normal);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/roughness.json", gfx::Texture_Atlas_Type::Roughness);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/metal.json", gfx::Texture_Atlas_Type::Metal);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/glow.json", gfx::Texture_Atlas_Type::Glow);
		}
		else if(quality == 1)
		{
				gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/albedo_medium.json", gfx::Texture_Atlas_Type::Diffuse);
				gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/normal_medium.json", gfx::Texture_Atlas_Type::Normal);
				gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/roughness.json", gfx::Texture_Atlas_Type::Roughness);
				gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/metal.json", gfx::Texture_Atlas_Type::Metal);
				gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/glow.json", gfx::Texture_Atlas_Type::Glow);
		}
		else if(quality == 2)
		{
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/albedo_high.json", gfx::Texture_Atlas_Type::Diffuse);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/normal_high.json", gfx::Texture_Atlas_Type::Normal);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/roughness.json", gfx::Texture_Atlas_Type::Roughness);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/metal.json", gfx::Texture_Atlas_Type::Metal);
			gfx::g_MaterialBank.LoadTextureAtlass("../../../asset/texture/glow.json", gfx::Texture_Atlas_Type::Glow);
		}

#endif
	gfx::g_ModelBank.LoadModel( "pod1Shape.robo" );
	gfx::g_ModelBank.LoadModel( "laserShape.robo" );
	gfx::g_ModelBank.LoadModel( "projectileCone.robo" );
	gfx::g_ModelBank.LoadModel( "Radar1Shape.robo" );
	gfx::g_ModelBank.LoadModel( "robotClawShape.robo" );
	gfx::g_ModelBank.LoadModel( "laserRobotShape.robo" );
	gfx::g_ModelBank.LoadModel( "MachineGun1Shape.robo" );
	gfx::g_ModelBank.LoadModel( "plasmaWeaponShape.robo" );
	gfx::g_ModelBank.LoadModel( "shieldShape.robo" );
	gfx::g_ModelBank.LoadModel( "tankTurretShape.robo" );
	gfx::g_ModelBank.LoadModel( "ArtilleryModelShape.robo" );
	gfx::g_ModelBank.LoadModel( "artilleyShellShape.robo" );
	gfx::g_ModelBank.LoadModel( "CutterShape.robo" ); 
	gfx::g_ModelBank.LoadModel( "Fortifikation1Shape.robo" );

	gfx::g_ModelBank.LoadModel( "LegsShape.robo" );
	gfx::g_ModelBank.LoadModel( "trackShape.robo" );

	gfx::g_ModelBank.LoadModel( "Radar1Shape.robo" );
	gfx::g_ModelBank.LoadModel( "ImprovedSensor.robo" );
	gfx::g_ModelBank.LoadModel( "ResourceRadarShape1.robo" );

	gfx::g_ModelBank.LoadModel( "RockShape1.robo" );
	gfx::g_ModelBank.LoadModel( "RockShape2.robo" );
	gfx::g_ModelBank.LoadModel( "RockShape3.robo" );
	gfx::g_ModelBank.LoadModel( "RockShape4.robo" );
	gfx::g_ModelBank.LoadModel( "RockShape5.robo" );
	gfx::g_ModelBank.LoadModel( "RockShape6.robo" );

	gfx::g_ModelBank.LoadModel( "OldTVShape.robo" );
	gfx::g_ModelBank.LoadModel( "CardboardBoxShape.robo" );
	gfx::g_ModelBank.LoadModel( "dirtyWheelsShape.robo" );
	gfx::g_ModelBank.LoadModel( "oldBicycleShape.robo" );
	gfx::g_ModelBank.LoadModel( "OldChairShape.robo" );
	gfx::g_ModelBank.LoadModel( "ResearchBottomShape.robo" );

	gfx::g_ModelBank.BuildBuffers();
	gfx::g_MaterialBank.BuildMaterialBuffer();
	Subsystem::Startup();
}

void SSGFXLoader::Shutdown()
{
	gfx::g_MaterialBank.ClearMaterials();
	gfx::g_ModelBank.Clear();
	Subsystem::Shutdown();
}
