#include "Enginepch.h"

void BattleUIOrchestrator::OnBoot()
{
	eventBus   = &registry.Get<BattleEventBus>();
	uiRegistry = &registry.Get<UIRegistry>();
	uiSys      = &registry.Get<UISystem>();
	uiAnimSys  = &registry.Get<UIAnimSystem>();
	presenter  = &registry.Get<BattleTimelinePresenter>();

	assert(eventBus && uiRegistry && uiSys && uiAnimSys && presenter);
}

void BattleUIOrchestrator::Enter()
{
	presenter->Enter();
	presenter->SetAbsoluteLayout(700.f, 900.f, 1200.f, 1000.f, 540.f);

	for (auto& [key, spec] : uiRegistry->GetArchetypes())
	{
		if (spec.context == UIContext::Battle && spec.startEnabled)
			uiRegistry->Ensure(key);
	}
}

void BattleUIOrchestrator::Tick(float dt)
{
	uiAnimSys->Tick(dt);
	uiSys->Tick(dt);

	presenter->Tick(dt);
}

void BattleUIOrchestrator::Exit()
{
	presenter->Exit();

	for (auto& [key, inst] : uiRegistry->GetInstances())
		uiRegistry->SetEnabled(key, false);
}