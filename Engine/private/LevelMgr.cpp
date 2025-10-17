#include "Enginepch.h"

void LevelMgr::Update(float dt)
{
	if (curLevel)
		curLevel->Update(dt);
}

void LevelMgr::Render()
{
	if (curLevel)
		curLevel->Render();
}

void LevelMgr::ChangeLevel(_uint _levelID, unique_ptr<Level> newLevel)
{
	if (curLevel)
		gameInstance.ClearResources(levelID);

	curLevel = move(newLevel);
	levelID = _levelID;
}