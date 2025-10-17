#include "pch.h"

unique_ptr<MapToolLevel> MapToolLevel::Create()
{
	auto instance = make_unique<MapToolLevel>();
	if (FAILED(instance->Init()))
		return nullptr;

	return instance;
}

HRESULT MapToolLevel::Init()
{
	{
		ObjDesc gridDesc;
		gridDesc.levelID = ENUM(LEVEL::MAPTOOL);
		gridDesc.layerType = LAYER::MAPOBJ;
		gridDesc.name = L"Grid";
		grid = game.AddObj<Grid>(gridDesc);

		if (grid)
		{
			auto params = grid->GetParams();
		
			params.cellSize = 10.0f;
			params.origin = { 0.f, 0.f, 0.f };
			params.CellCountX = 100;
			params.CellCountZ = 100;
			params.majorEvery = 5;
			params.showMajor = true;
			params.showMinor = true;
			params.showHover = true;
			grid->SetParams(params);
		}
	}

#ifdef USE_IMGUI
	auto panel = game.AddPanel<MapToolPanel>();
	panel->SetVisibleRule([this]() {return game.GetCurLevelID() == ENUM(LEVEL::MAPTOOL); });
	panel->SetGrid(grid);
#endif

	return S_OK;
}

void MapToolLevel::Update(float dt)
{

}

void MapToolLevel::Render()
{
	if (grid)
	{
		auto cam = game.GetMainCam();
		if (!cam) return;

		auto shader = game.GetShader(L"PC");
		shader->Bind();

		game.BindGrid();

		game.SetBlendState(BLENDSTATE::Opaque);
		game.SetDepthState(DEPTHSTATE::DEFAULT);
		grid->DrawGrid();

		game.SetBlendState(BLENDSTATE::ALPHABLEND);
		game.SetDepthState(DEPTHSTATE::NO_DEPTHWRITE);
		game.BindGrid();

		game.SetRasterizerState(RASTERIZER::CULLNONE);
		grid->DrawHover();
		game.SetRasterizerState(RASTERIZER::CULLBACK);

		game.SetDepthState(DEPTHSTATE::DEFAULT);
		game.SetBlendState(BLENDSTATE::Opaque);
	}
}