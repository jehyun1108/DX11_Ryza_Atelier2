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
// --------------------------------------------------------------------------------------------------
	
	auto patricia = spawner.NewEntity()
		.WithTf()
		//.WithScale(0.1f, 0.1f, 0.1f)
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"Patricia")
		//.WithFace(L"PC24A_Face_Eye_UP", L"PC24A_Face_CloseEye_down")
		//.WithMouth(L"PC24A_Face_pronunciation")
		.WithTag("patricia")
		.WithPickingFromModel()
		.WithColliderFromModel(ColliderType::AABB)
		.Build();

	auto freeCam = spawner.NewEntity()
		.WithTf(TransformDesc{ .pos = _float3{ 0.f, 30.f, -50.f } })
		.WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
		.WithCam(XMConvertToRadians(90.f), float(WinX) / WinY, 0.1f, 1000.f, true)
		.WithFreeCam(100.f)
		//.WithThirdCam(patricia.tf, _vec{ 0.f, 40.f, -40.f })
		.WithDirectionalLight()
		.WithTag("freecam")
		//.WithPicking()
		.Build();

	//auto patricia_weapon = spawner.NewEntity()
	//	.WithTf()
	//	.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
	//	.WithModel(L"patricia_weapon")
	//	.WithSocket("patricia", "bone_71", _float3(3.f, 3.f, 0.f), _float3(-13.f, 86.f, -1.4f))
	//	.WithTag("patricia_weapon")
	//	//.WithPicking()
	//	.Build();

	auto grid = spawner.NewEntity()
		.WithTf()
		.WithGrid()
		.WithLayer(LayerUtil::LayerBit(LAYER::MAPOBJ))
		.WithTag("grid")
		.WithPicking()
		.Build();

	return S_OK;
}

void ImportLevel::Update(float dt)
{
	SelectionContext ctx = selectSys.GetContext();
	ctx.layerMask = 0xFFFFFFFFu;

	EntityID gridID = tagSys.Get("grid");
	Handle gridHandle{};
	gridSys.GetByOwner(gridID, &gridHandle);

	const auto& gridParam = gridSys.GetParams(gridHandle);

	ctx.dragPlane.enabled = true;
	ctx.dragPlane.point   = { 0, gridParam.origin.y, 0 };
	ctx.dragPlane.normal  = { 0, 1, 0 };

	ctx.snap.enabled      = game.KeyPressing(KEY::LSHIFT);
	ctx.snap.stepX        = ctx.snap.stepZ = gridParam.cellSize;
	ctx.snap.stepY        = 0.f; // 평면 드래그
	ctx.snap.stepZ        = gridParam.cellSize;
	ctx.snap.origin       = gridParam.origin;

	selectSys.SetContext(ctx);

	const EntityID hovered = selectSys.GetHovered();

	if (hovered != 0)
	{
		if (auto pick = pickSys.GetByOwner(hovered))
		{
			_float3 corners[8];
			pick->worldBox.GetCorners(corners);

			_float3 mn = corners[0];
			_float3 mx = corners[0];

			for (int i = 0; i < 8; ++i)
			{
				mn.x = min(mn.x, corners[i].x);
				mn.y = min(mn.y, corners[i].y);
				mn.z = min(mn.z, corners[i].z);
				mx.x = max(mx.x, corners[i].x);
				mx.y = max(mx.y, corners[i].y);
				mx.z = max(mx.z, corners[i].z);
			}

			// Grid -> Hightlight
			gridSys.SetHoverForAABB(gridHandle, mn, mx, true);
		}
		else
			gridSys.SetHover(gridHandle, {}, false);
	}
	else
		gridSys.SetHover(gridHandle, {}, false);
}

void ImportLevel::Render()
{

}