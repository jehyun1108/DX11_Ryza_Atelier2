#include "pch.h"
#include "MainApp.h"

#include "Logo.h"
#include "Central.h"
#include "loading.h"
#include "UILoader.h"
#include "LoadingPresenter.h"
#include "ScreenFadeSystem.h"

HRESULT MainApp::Init()
{
	EngineDesc desc{};
	desc.hWnd    = g_hWnd;
	desc.winMode = WINMODE::WIN;
	desc.WinX    = WinX;
	desc.WinY    = WinY;
	desc.levelCount = ENUM(LEVEL::END);

	if (FAILED(game.InitEngine(desc))) return E_FAIL;
	LoadLoadingResources();
	game.ChangeLevel(ENUM(LEVEL::LOADING), Loading::Create());

	return S_OK;
}

void MainApp::Update(_float dt)
{
	//RECT rect{0.f, 0.f, WinX, WinY};
	//ClipCursor(&rect);
	game.UpdateEngine(dt);
}

HRESULT MainApp::Render()
{
	game.Draw();
	game.GuiRender();
	game.EndDraw();
	return S_OK;
}

void MainApp::LoadLoadingResources()
{
	auto& registry = game.GetRegistry();
	auto& assets = registry.Get<AssetSystem>();
	auto& uiRegistry = registry.Get<UIRegistry>();

	UILoader::RegisterOverlayUI(&assets);
	UIArchetypeLoader::RegisterOverlayUI(&uiRegistry);
	UILoader::RegisterLoadingUI(&assets);
	UIArchetypeLoader::RegisterLoadingUI(&uiRegistry);


	auto& loadingPresenter = registry.Get<LoadingPresenter>();
	loadingPresenter.Enter();
	
	auto& fadeSys = registry.Get<ScreenFadeSystem>();
	fadeSys.Init();
	//fadeSys.FadeToBlack(0.5f);
}


unique_ptr<MainApp> MainApp::Create()
{
	auto inst = make_unique<MainApp>();
	if (FAILED(inst->Init()))
		return nullptr;
	return inst;
}