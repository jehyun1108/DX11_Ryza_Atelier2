#include "pch.h"
#include "MainApp.h"

#include "MapToolPanel.h"
#include "MapToolLevel.h"

HRESULT MainApp::Init()
{
	EngineDesc desc{};
	desc.hWnd    = g_hWnd;
	desc.winMode = WINMODE::WIN;
	desc.WinX    = WinX;
	desc.WinY    = WinY;
	desc.levelCount = ENUM(LEVEL::END);

	if (FAILED(game.InitEngine(desc)))
		return E_FAIL;
	
	game.ChangeLevel(ENUM(LEVEL::MAPTOOL), MapToolLevel::Create());

	return S_OK;
}

void MainApp::Update(_float dt)
{
	game.UpdateEngine(dt);
}

HRESULT MainApp::Render()
{
	_float4 clearColor = _float4(0.f, 0.f, 1.f, 1.f);

	game.BeginDraw(clearColor);
	game.Draw();
	game.GuiRender();
	game.EndDraw();

	return S_OK;
}

unique_ptr<MainApp> MainApp::Create()
{
	auto inst = make_unique<MainApp>();

	if (FAILED(inst->Init()))
		return nullptr;

	return inst;
}