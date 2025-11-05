#include "Enginepch.h"

GameModeDirectorSystem::GameModeDirectorSystem(SystemRegistry& registry)
	:registry(registry),curMode(GameMode::Field), playerLeader(1){}

void GameModeDirectorSystem::OnBoot()
{
	fieldSys  = &registry.Get<FieldOrchestraSystem>();
	battleSys = &registry.Get<BattleOrchestraSystem>();

	assert(fieldSys && battleSys);
}

void GameModeDirectorSystem::Update(float dt)
{
	switch (curMode)
	{
	case GameMode::Field:  fieldSys->Update(dt);   break;
	case GameMode::Battle: battleSys->Update(dt);  break;
	case GameMode::Menu:                          break;
	}
}

void GameModeDirectorSystem::RequestSwitch(GameMode nextMode)
{
	if (nextMode == curMode) return;

	switch (curMode)
	{
	case GameMode::Field:  fieldSys->Exit();	  break;
	case GameMode::Battle: battleSys->Exit();  break;
	case GameMode::Menu:                      break;
	}

	curMode = nextMode;

	switch (curMode)
	{
	case GameMode::Field:  fieldSys->Enter();  break;
	case GameMode::Battle: battleSys->Enter(); break;
	case GameMode::Menu:                      break;
	}
}

bool GameModeDirectorSystem::BeginBattle(const BattleStartParams& startParams)
{
	if (!battleSys->BeginBattle(startParams)) return false;
	RequestSwitch(GameMode::Battle);
	return true;
}