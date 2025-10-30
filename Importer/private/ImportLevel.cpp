#include "pch.h"
#include "ImportLevel.h"
#include "ImportPanel.h"

unique_ptr<ImportLevel> ImportLevel::Create()
{
	auto instance = make_unique<ImportLevel>();
	if (FAILED(instance->Init()))
		return nullptr;
	return instance;
}

HRESULT ImportLevel::Init()
{
#ifdef USE_IMGUI
	game.AddPanel<ImportPanel>("Importer");
#endif
	assets.RegisterModel(L"patricia", { L"../bin/Resources/Models/Patricia/Patricia.model", true });
	assets.RegisterModel(L"patricia_weapon", { L"../bin/Resources/Models/Patricia_weapon/Patricia_weapon.model", true });

	assets.RegisterModel(L"ryza", { L"../bin/Resources/Models/Ryza/Ryza.model", true });
	assets.RegisterModel(L"ryza_cap", { L"../bin/Resources/Models/Ryza_cap/Ryza_cap.model", true });
	assets.RegisterModel(L"ryza_weapon_4", { L"../bin/Resources/Models/Ryza_weapon_4/ryza_weapon_4.model", true });
	
	assets.RegisterModel(L"klaudia", { L"../bin/Resources/Models/klaudia/klaudia.model", true });
	assets.RegisterModel(L"klaudia_weapon_2", { L"../bin/Resources/Models/Klaudia_weapon_2/Klaudia_weapon_2.model", true });

	assets.RegisterModel(L"bottlesky", { L"../bin/Resources/Models/Skybox/BottleSky/BottleSky.model", true });
	assets.RegisterModel(L"nightsky", { L"../bin/Resources/Models/Skybox/NightSky/NightSky.model", true });

	assets.RegisterModel(L"angel", { L"../bin/Resources/Models/Angel/Angel.model", true });
// --------------------------------------------------------------------------------------------------
	playerHandle = SpawnRyza();
	playerID = playerHandle.entity;

	auto klaudia = SpawnKlaudia();
	auto patricia = SpawnPatricia();

	DrawSkyBox(true);
	
	auto cam = spawner.NewEntity()
		.WithTf(TransformDesc{ .pos = _float3{ 0.f, 400.f, -200.f } })
		.WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
		.WithCam(XMConvertToRadians(90.f), float(WinX) / WinY, 0.1f, 10000.f, true)
		.WithFreeCam(1000.f)
		//.WithThirdCam(playerHandle.tf, _vec{ 0.f, 200.f, -200.f }, OffsetSpace::WorldSpace, FollowPolicy::PosOnly)
		.WithDirectionalLight()
		.WithTag("freecam")
		.Build();

	auto grid = spawner.NewEntity()
		.WithTf()
		.WithGrid()
		.WithLayer(LayerUtil::LayerBit(LAYER::MAPOBJ))
		.WithTag("grid")
		.Build();

	auto angel = spawner.NewEntity()
		.WithTf().WithEuler(0.f, 180.f, 0.f).WithScale(3.f, 3.f, 3.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::MONSTER))
		.WithTag("angel")
		.WithModel(L"angel")
		.WithColliderFromModel()
		.Build();

	inputSerivce.SetActiveEntity(playerID);
	inputSerivce.SetContext(InputContext::Field);
	inputSerivce.SetFocus(FocusState::None);
	inputSerivce.ReleaseLock(LockTag::MenuLock);
	inputSerivce.ReleaseLock(LockTag::CutScene);
	inputSerivce.SetManualTime(0.f);

	return S_OK;
}

void ImportLevel::Update(float dt)
{
	UpdateGrid();
}

void ImportLevel::Render()
{

}

EntityHandles ImportLevel::SpawnPatricia()
{
	auto patricia = spawner.NewEntity()
		.WithTf().WithPos(150.f, 0.f, 1100.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"patricia")
		//.WithFace(L"PC24A_Face_Eye_UP", L"PC24A_Face_CloseEye_down")
		//.WithMouth(L"PC24A_Face_pronunciation")
		.WithTag("patricia")
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithSelectable(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithPlayerMovement()
		.Build();

	auto patricia_weapon = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithTag("patricia_weapon")
		.WithModel(L"patricia_weapon")
		.WithSocket("patricia", "bone_71", _float3(3.f, 3.f, 0.f), _float3(-13.f, 86.f, -1.4f))
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithSelectable(LayerUtil::LayerBit(LAYER::PLAYER))
		.Build();
	
	return patricia;
}

EntityHandles ImportLevel::SpawnKlaudia()
{
	auto klaudia = spawner.NewEntity()
		.WithTf().WithPos(-150.f, 0.f, 1100.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"klaudia")
		.WithTag("klaudia")
		//.WithFace(L"", L"")
		//.WithMouth(L"PC21A_Face_pronunciation")
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable()
		.WithSelectable()
		.WithPlayerMovement()
		.Build();

	auto weapon = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"klaudia_weapon_2")
		.WithTag("kluadia_weapon_2")
		.WithSocket("klaudia", "bone_62", _float3(2.f,2.5f,0.f), _float3(-103.f,45.f,30.f))
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithSelectable(LayerUtil::LayerBit(LAYER::PLAYER))
		.Build();

	return klaudia;
}

EntityHandles ImportLevel::SpawnRyza()
{
	auto ryza = spawner.NewEntity()
		.WithTf().WithPos(0.f, 0.f, 1000.f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"ryza")
		//.WithFace(L"", L"")
		//.WithMouth(L"PC21A_Face_pronunciation")
		.WithTag("ryza")
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable()
		.WithSelectable()
		.WithPlayerMovement()
		.Build();

	auto ryza_weapon = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"ryza_weapon_4")
		.WithTag("ryza_weapon_4")
		.WithSocket("ryza", "bone_63",_float3(1.f,3.f,0.f), _float3(0.f,90.f,0.f))
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithSelectable(LayerUtil::LayerBit(LAYER::SOCKET))
		.Build();

	auto ryza_cap = spawner.NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"ryza_cap")
		.WithTag("ryza_cap")
		.WithSocket("ryza", "bone_961",_float3(-4.f,-153.f,0.f), _float3(0, -90.f,0.f))
		.WithColliderFromModel(ColliderType::AABB)
		.WithMeshCollider()
		.WithPickable(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithSelectable(LayerUtil::LayerBit(LAYER::SOCKET))
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

	return ryza;
}

void ImportLevel::SetUp()
{

}

void ImportLevel::UpdateGrid()
{
	SelectionContext ctx = selectSys.GetContext();
	ctx.layerMask = 0xFFFFFFFFu;

	EntityID gridID = tagSys.Get("grid");
	Handle gridHandle{};
	gridSys.GetByOwner(gridID, &gridHandle);

	const auto& gridParam = gridSys.GetParams(gridHandle);

	ctx.dragPlane.enabled = true;
	ctx.dragPlane.point = { 0, gridParam.origin.y, 0 };
	ctx.dragPlane.normal = { 0, 1, 0 };

	ctx.snap.enabled = game.KeyPressing(KEY::LSHIFT);
	ctx.snap.stepX  = ctx.snap.stepZ = gridParam.cellSize;
	ctx.snap.stepY  = 0.f; 
	ctx.snap.stepZ  = gridParam.cellSize;
	ctx.snap.origin = gridParam.origin;

	selectSys.SetContext(ctx);

	const EntityID hovered = selectSys.GetHovered();
	
	if (hovered != 0)
	{
		_float3 hitMin{}, hitMax{};

		_float3 rayOrigin{}, rayDir{};
		if (ctx.fromScreen)
		{
			auto& camSys = registry.Get<CameraSystem>();
			_vec vOrigin{}, vDir{};
			camSys.CreateRayFromScreen(ctx.cam, ctx.screenPos, ctx.viewport, vOrigin, vDir);
			XMStoreFloat3(&rayOrigin, vOrigin);
			XMStoreFloat3(&rayDir, vDir);
		}
		else
		{
			rayOrigin = ctx.worldRayOrigin;
			XMStoreFloat3(&rayDir, XMVector3Normalize(XMLoadFloat3(&ctx.worldRayDir)));
		}

		PickingHit hit{};
		if (pickSys.Pick(PickingRequest{ rayOrigin, rayDir, false, {}, {}, {}, ctx.layerMask }, hit) && hit.hit)
		{
			if (gridSys.ComputeCellBoundsFromPoint(gridHandle, hit.point, hitMin, hitMax))
				gridSys.SetHoverForAABB(gridHandle, hitMin, hitMax, true);
			else
				gridSys.SetHover(gridHandle, {}, false);
		}
		else
			gridSys.SetHover(gridHandle, {}, false);
	}
	else
		gridSys.SetHover(gridHandle, {}, false);
}

void ImportLevel::DrawSkyBox(bool isNight)
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
