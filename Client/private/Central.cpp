#include "pch.h"
#include "Central.h"
#include "UILoader.h"
#include "CharacterUILoader.h"

unique_ptr<Central> Central::Create()
{
	auto instance = make_unique<Central>();
	if (FAILED(instance->Init()))
		return nullptr;
	return instance;
}

HRESULT Central::Init()
{
	auto ryza = SpawnRyza();
	playerHandle = ryza;
	playerID = playerHandle.entity;

	auto klaudia  = SpawnKlaudia();
	auto patricia = SpawnPatricia();
	auto angel    = SpawnAngel();

	DrawSkyBox(true);

	auto cam = spawner.NewEntity()
		.WithTf(TransformDesc{ .pos = _float3{ 0.f, 400.f, -200.f } })
		.WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
		.WithCam(XMConvertToRadians(90.f), float(WinX) / WinY, 0.1f, 10000.f, true)
		.WithFreeCam(500.f)
		//.WithThirdCam(playerHandle.tf, _vec{ 0.f, 200.f, -200.f }, OffsetSpace::WorldSpace, FollowPolicy::PosOnly)
		.WithDirectionalLight()
		.WithTag("freecam")
		.Build();

	//auto orbitCam = spawner.NewEntity()
	//	.WithTf()
	//	.WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
	//	.WithCam(XMConvertToRadians(90.f), float(WinX) / WinY, 0.1f, 10000.f, true)
	//	.WithOrbitCam(ryza.tf, 0.f, 15.f, 350.f)
	//	.WithDirectionalLight()
	//	.WithTag("orbitcam")
	//	.Build();
	//fieldCtrlHandle = fieldCtrlSys.Create(ryza.entity, orbitCam.tf);

	auto grid = spawner.NewEntity()
		.WithTf()
		.WithGrid()
		.WithLayer(LayerUtil::LayerBit(LAYER::MAPOBJ))
		.WithTag("grid")
		.Build();

	registry.Get<GameModeDirectorSystem>().Start();

	return S_OK;
}

void Central::Update(float dt)
{

}

void Central::Render()
{

}

EntityHandles Central::SpawnPatricia()
{
	auto patricia = spawner.NewEntity()
		.WithTf().WithPos(200.f, 0.f, 1100.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"patricia")
		//.WithFace(L"PC24A_Face_Eye_UP", L"PC24A_Face_CloseEye_down")
		//.WithMouth(L"PC24A_Face_pronunciation")
		.WithTag("patricia")
		.WithPlayerMovement()
		.Build();
	
	charaDataSys.BindEntity(patricia.entity, CharacterID::Patricia);

	auto patricia_weapon = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithTag("patricia_weapon")
		.WithModel(L"patricia_weapon")
		.WithSocket("patricia", "bone_71", _float3(3.f, 3.f, 0.f), _float3(-13.f, 86.f, -1.4f))
		.Build();
	
	return patricia;
}

EntityHandles Central::SpawnKlaudia()
{
	auto klaudia = spawner.NewEntity()
		.WithTf().WithPos(-200.f, 0.f, 1100.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"klaudia")
		.WithTag("klaudia")
		//.WithFace(L"PC21A_Face_Eye_UP", L"PC21A_Face_CloseEye_down")
		//.WithMouth(L"PC21A_Face_pronunciation")
		.WithPlayerMovement()
		.Build();

	charaDataSys.BindEntity(klaudia.entity, CharacterID::Klaudia);

	auto weapon = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"klaudia_weapon_2")
		.WithTag("kluadia_weapon_2")
		.WithSocket("klaudia", "bone_62", _float3(2.f, 2.5f, 0.f), _float3(-103.f, 45.f, 30.f))
		.Build();

	return klaudia;
}

EntityHandles Central::SpawnRyza()
{
	auto ryza = spawner.NewEntity()
		.WithTf().WithPos(0.f, 0.f, 1000.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"ryza")
		//.WithFace(L"", L"")
		//.WithMouth(L"PC21A_Face_pronunciation")
		.WithTag("ryza")
		.WithPlayerMovement()
		.Build();

	charaDataSys.BindEntity(ryza.entity, CharacterID::Ryza);

	auto ryza_weapon = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"ryza_weapon_4")
		.WithTag("ryza_weapon_4")
		.WithSocket("ryza", "bone_63", _float3(1.f, 3.f, 0.f), _float3(0.f, 90.f, 0.f))
		.Build();

	auto ryza_cap = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"ryza_cap")
		.WithTag("ryza_cap")
		.WithSocket("ryza", "bone_961", _float3(-4.f, -153.f, 0.f), _float3(0, -90.f, 0.f))
		.Build();

	return ryza;
}

EntityHandles Central::SpawnAngel()
{
	const vector<_float3> initPos = { _float3{}, _float3{ -400.f, 0.f, -300.f }, _float3{ 400.f, 0.f, -300.f } };
	for (int i = 0; i < 3; ++i)
	{
		auto angel = spawner.NewEntity()
			.WithTf(TransformDesc{ .pos = initPos[i] }).WithEuler(0.f, 180.f, 0.f).WithScale(3.f, 3.f, 3.f)
			.WithLayer(LayerUtil::LayerBit(LAYER::MONSTER))
			.WithModel(L"angel")
			.Build();
		charaDataSys.BindEntity(angel.entity, CharacterID::Angel);
	}
	return {};
}

void Central::DrawSkyBox(bool isNight)
{
	if (isNight)
	{
		auto nightSky = spawner.NewEntity()
			.WithTf()
			.WithLayer(LayerUtil::LayerBit(LAYER::SKYBOX))
			.WithTag("nightsky")
			.WithSkybox(L"nightsky")
			.Build();
	}
	else
	{
		auto bottleSky = spawner.NewEntity()
			.WithTf()
			.WithLayer(LayerUtil::LayerBit(LAYER::SKYBOX))
			.WithTag("bottlesky")
			.WithSkybox(L"bottlesky")
			.Build();
	}
}