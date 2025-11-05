#include "pch.h"
#include "Loading.h"
#include "Central.h"
#include "UILoader.h"
#include "CharacterUILoader.h"

unique_ptr<Loading> Loading::Create()
{
	auto instance = make_unique<Loading>();
	if (FAILED(instance->Init()))
		return nullptr;
	return instance;
}

HRESULT Loading::Init()
{
	loader.Start();
	return S_OK;
}

void Loading::Update(float dt)
{
	if (!loader.IsDone()) return;

	loader.StopAndJoin();

	LoadResources();

	GAME.ChangeLevel(ENUM(LEVEL::CENTRAL), Central::Create());
}

void Loading::Render()
{

}

void Loading::LoadResources()
{
	UILoader::RegisterUIResources(assets);
	UIArchetypeLoader::RegisterUIArchetypes(uiRegistry);
	CharacterUILoader::RegisterCharacterUIMappings(dataSys);

	assets->RegisterModel(L"patricia", { L"../bin/Resources/Models/Patricia/Patricia.model", true });
	assets->RegisterModel(L"patricia_weapon", { L"../bin/Resources/Models/Patricia_weapon/Patricia_weapon.model", true });

	assets->RegisterModel(L"ryza", { L"../bin/Resources/Models/Ryza/Ryza.model", true });
	assets->RegisterModel(L"ryza_cap", { L"../bin/Resources/Models/Ryza_cap/Ryza_cap.model", true });
	assets->RegisterModel(L"ryza_weapon_4", { L"../bin/Resources/Models/Ryza_weapon_4/ryza_weapon_4.model", true });

	assets->RegisterModel(L"klaudia", { L"../bin/Resources/Models/klaudia/klaudia.model", true });
	assets->RegisterModel(L"klaudia_weapon_2", { L"../bin/Resources/Models/Klaudia_weapon_2/Klaudia_weapon_2.model", true });

	assets->RegisterModel(L"bottlesky", { L"../bin/Resources/Models/Skybox/BottleSky/BottleSky.model", true });
	assets->RegisterModel(L"nightsky", { L"../bin/Resources/Models/Skybox/NightSky/NightSky.model", true });

	assets->RegisterModel(L"angel", { L"../bin/Resources/Models/Angel/Angel.model", true });
}
