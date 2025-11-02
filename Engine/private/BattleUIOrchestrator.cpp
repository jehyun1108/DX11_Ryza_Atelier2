#include "Enginepch.h"

void BattleUIOrchestrator::Enter()
{
	for (auto& [key, spec] : uiRegistry.GetArchetypes())
	{
		if (spec.context == UIContext::Battle && spec.startEnabled)
			uiRegistry.Ensure(key);
	}
}

void BattleUIOrchestrator::Tick(float dt)
{
	uiAnimSys.Tick(dt);
	uiSys.Tick(dt);
}

void BattleUIOrchestrator::Exit()
{
	for (auto& [key, inst] : uiRegistry.GetInstances())
		uiRegistry.SetEnabled(key, false);
}