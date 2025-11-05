#include "Enginepch.h"

void BattleUIOrchestrator::Enter()
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	timelinePresenter = make_unique<BattleTimelinePresenter>(registry, bus, uiRegistry, uiAnimSys, timelineSys);

	timelinePresenter->Enter();
	timelinePresenter->SetAbsoluteLayout(700.f, 900.f, 1200.f, 1000.f, 540.f);

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

	if (timelinePresenter)
		timelinePresenter->Tick(dt);
}

void BattleUIOrchestrator::Exit()
{
	if (timelinePresenter)
		timelinePresenter->Exit();

	for (auto& [key, inst] : uiRegistry.GetInstances())
		uiRegistry.SetEnabled(key, false);
}