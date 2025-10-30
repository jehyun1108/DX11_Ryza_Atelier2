#include "Enginepch.h"

GameModeDirectorSystem::GameModeDirectorSystem(SystemRegistry& registry, EntityID fieldLeaderEntity)
	:registry(registry), fieldSys(registry), battleSys(registry), curMode(GameMode::Field), playerLeader(fieldLeaderEntity)
{
	fieldSys.Enter();
}

void GameModeDirectorSystem::Update(float dt)
{
	switch (curMode)
	{
	case GameMode::Field:  fieldSys.Update(dt);   break;
	case GameMode::Battle: battleSys.Update(dt);  break;
	case GameMode::Menu:                          break;
	}
}

void GameModeDirectorSystem::RequestSwitch(GameMode nextMode)
{
	if (nextMode == curMode) return;

	switch (curMode)
	{
	case GameMode::Field:  fieldSys.Exit();	 break;
	case GameMode::Battle: battleSys.Exit(); break;
	case GameMode::Menu:                     break;
	}

	curMode = nextMode;

	switch (curMode)
	{
	case GameMode::Field:  fieldSys.Enter();  break;
	case GameMode::Battle: battleSys.Enter(); break;
	case GameMode::Menu:                      break;
	}
}

bool GameModeDirectorSystem::BeginBattle(const BattleStartParams& startParams)
{
	if (!battleSys.BeginBattle(startParams)) return false;
	RequestSwitch(GameMode::Battle);
	return true;
}