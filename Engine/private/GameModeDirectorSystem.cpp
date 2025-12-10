#include "Enginepch.h"
#include "LogoOrchestraSystem.h"
#include "ScreenFadeSystem.h"
#include "ScreenDistortionSystem.h"
#include "SoundSystem.h"

GameModeDirectorSystem::GameModeDirectorSystem(SystemRegistry& registry) 
	:registry(registry),curMode(GameMode::None) {}

void GameModeDirectorSystem::OnBoot()
{
	fieldSys   = &registry.Get<FieldOrchestraSystem>();
	battleSys  = &registry.Get<BattleOrchestraSystem>();
	logoSys    = &registry.Get<LogoOrchestraSystem>();
	uiSys      = &registry.Get<UISystem>();
	fadeSys    = &registry.Get<ScreenFadeSystem>();
	uiAnimSys  = &registry.Get<UIAnimSystem>();
	distortSys = &registry.Get<ScreenDistortionSystem>();
	soundSys   = &registry.Get<SoundSystem>();
}

void GameModeDirectorSystem::Update(float dt)
{
	fadeSys->Tick(dt);
	distortSys->Tick(dt);

	switch (curMode)
	{
	case GameMode::Field:  fieldSys->Update(dt);   break;
	case GameMode::Battle: battleSys->Update(dt);  break;
	case GameMode::Menu:   logoSys->Tick(dt);      break;
	}

	uiAnimSys->Tick(dt);
	uiSys->Tick(dt);
}

void GameModeDirectorSystem::RequestSwitch(GameMode nextMode)
{
	if (nextMode == curMode) return;

	switch (curMode)
	{
	case GameMode::Field:  fieldSys->Exit();   break;
	case GameMode::Battle: battleSys->Exit();  break;
	case GameMode::Menu:   logoSys->Exit();    break;
	}

	curMode = nextMode;

	switch (curMode)
	{
	case GameMode::Field:
		uiSys->SetActiveContext(UIContext::Field);
		fieldSys->Enter();
		soundSys->PlayBgm(L"Central_BGM");
		break;

	case GameMode::Battle:
		uiSys->SetActiveContext(UIContext::Battle);
		battleSys->Enter();
		soundSys->PlayBgm(L"Battle_BGM_3");
		soundSys->PlayAfter(L"022_battle_start", 0.25f, 0.f);
		soundSys->PlayAfter(L"patricia_29", 2.5f);
		soundSys->PlayAfter(L"ryza_49", 5.5f);
		soundSys->PlayAfter(L"ryza_48", 0.f);
		break;

	case GameMode::Menu:
		uiSys->SetActiveContext(UIContext::Logo);
		logoSys->Enter();
		soundSys->PlayBgm(L"Intro_BGM");
		break;
	}
}

bool GameModeDirectorSystem::BeginBattle(const BattleStartParams& startParams)
{
	if (!battleSys->BeginBattle(startParams)) return false;
	RequestSwitch(GameMode::Battle);
	return true;
}