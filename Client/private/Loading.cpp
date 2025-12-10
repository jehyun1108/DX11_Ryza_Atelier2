#include "pch.h"
#include "Loading.h"
#include "Central.h"
#include "UILoader.h"
#include "CharacterUILoader.h"
#include "SoundLoader.h"
#include "ModelLoader.h"
#include "MapLoader.h"
#include "CamLoader.h"

#include "NavMeshSystem.h"
#include "WorldSerializer.h"
#include "LogoMenuPresenter.h"
#include "FieldMinimapPresenter.h"
#include "WorldMapPresenter.h"
#include "ScreenFadeSystem.h"
#include "LoadingPresenter.h"
#include "EffectLoader.h"
#include "DressingRoomPresenter.h"

#include "CharacterSpawner.h"
#include "CamSpawner.h"
#include "EnemySpawner.h"
#include "SkyboxSpawner.h"
#include "LightSpawner.h"
#include "SoundSystem.h"

unique_ptr<Loading> Loading::Create()
{
	auto instance = make_unique<Loading>();
	if (FAILED(instance->Init()))
		return nullptr;
	return instance;
}

HRESULT Loading::Init()
{
	uiSys->SetActiveContext(UIContext::Loading);

	loader.Start([this](atomic<bool>& stopRequested) 
		{
			LoadResources();
			SpawnEntities();
		});

	auto& logoMenu = registry.Get<LogoMenuPresenter>();
	logoMenu.SetCommandCallback(
		[&](LogoMenuCommand cmd)
		{
			switch (cmd)
			{
			case LogoMenuCommand::NewGame:
				director->RequestSwitch(GameMode::Field);
				director->Start();
				break;

			case LogoMenuCommand::LoadGame: 
				break;

			case LogoMenuCommand::OpenSetting:
				break;

			case LogoMenuCommand::ExitGame:
				PostQuitMessage(0);
				break;
			}
		} );
	return S_OK;
}

void Loading::Update(float dt)
{
	loadingPresenter->Tick(dt);

	if (!loader.IsDone()) return;

	static bool joined = false;
	static bool switchedToMenu = false;
	static bool fadeOutStarted = false;

	if (!joined)
	{
		loader.StopAndJoin();
		joined = true;
		loadingPresenter->OnLoadingComplete();
	}

	if (loadingPresenter->IsFadeOutFinished() && !fadeOutStarted)
	{
		fadeSys->FadeOut(0.8f);
		fadeOutStarted = true;
	}

	if (!switchedToMenu)
	{
		director->RequestSwitch(GameMode::Menu);
		switchedToMenu = true;
	}

	if (tfSys->GetPos(playerHandle.tf).y <= -500.f)
		tfSys->SetPos(playerHandle.tf, _float3{ 0.f, 100.f, 0.f });
}

void Loading::Render()
{

}

void Loading::LoadResources()
{
	CamSpawner    camSpawner(registry);
	SkyboxSpawner skySpawner(registry);
	LightSpawner  lightSpawner(registry);

	auto freeCam = camSpawner.SpawnFreeCam(_float3{ 0.f, 400.f, -200.f });
	registry.Get<CamRegistry>().SetDebugCam(freeCam.cam, freeCam.tf);
	auto light   = lightSpawner.SpawnDirectionalLight();

	UILoader::InitFonts(registry);

	UILoader::RegisterUIResources(assets);
	UIArchetypeLoader::RegisterUIResources(uiRegistry, uiSys);
	SoundLoader::Load(soundRegistry);

	CharacterUILoader::RegisterCharacterUI(dataSys);
	ModelLoader::RegisterModelResources(assets);
	MapLoader::LoadResources(worldSys, navSys);
	EffectLoader::LoadEffect(registry);
	CamLoader::LoadCamResources(registry);

	auto nightSky = skySpawner.SpawnNightSky();
	//auto bottleSky = skySpawner.SpawnSkybox(L"bottlesky", L"bottlesky");
	registry.Get<WorldMapPresenter>().Init();
}

void Loading::SpawnEntities()
{
// ======================== Character ========================================================
	CharacterSpawner characterSpawner(registry);
	CamSpawner       camSpawner(registry);
	EnemySpawner     enemySpawner(registry);

	//auto ryza = characterSpawner.SpawnRyza({});
	//auto ryza = characterSpawner.SpawnRyza({ 15000.f, 8500.f, -13000.f });
	auto ryza = characterSpawner.SpawnRyza(_float3{32050.f, 30.f, 4500.f});
	auto klaudia  = characterSpawner.SpawnKlaudia(_float3{ 42200.f, 120.f, 1880.f });
	auto patricia = characterSpawner.SpawnPatricia(_float3{ 42000.f, 100.f, 1300.f });

	playerHandle = ryza;
	playerID = playerHandle.entity;
// ============================ Camera =======================================
	auto orbitCam = camSpawner.SpawnFieldOrbitCam(ryza.tf);
	auto fieldMiniCam = camSpawner.SpawnFieldMiniCam();

	auto* fieldMini = &registry.Get<FieldMinimapPresenter>();
	fieldMini->SetPlayer(ryza.entity, ryza.tf);
	fieldMini->SetFieldCam(fieldMiniCam.tf, fieldMiniCam.cam);

	fieldCtrlSys->Create(ryza.entity, orbitCam.tf);
	input->SetActiveEntity(playerID);
// ===========================================================================

// ==================================== Enemy ============================================
	//const vector<_float3> initPos = 
	//	{
	//		_float3{    0.f,  -30.f,   0.f  },
	//		//_float3{ -400.f,  0.f, -300.f },
	//		//_float3{  400.f,  0.f, -300.f }
	//	};
	//auto angels = enemySpawner.SpawnAngels(initPos);
	
	//auto angel = enemySpawner.SpawnAngel({});
	auto angel = enemySpawner.SpawnAngel({ 15000.f, 4100.f, -16000.f });
	auto angel2 = enemySpawner.SpawnAngel({ 15300.f, 4100.f, -16300.f });
	auto angel3 = enemySpawner.SpawnAngel({ 14600.f, 4100.f, -16300.f });
// ========================================================================================
	registry.Get<DressingRoomPresenter>().BuildInitData();
}