#include "Enginepch.h"
#include "BattleTimelinePresenter.h"
#include "PlayerInputPresenter.h"
#include "BattleHUDPresenter.h"
#include "BattleFatalDrivePresenter.h"
#include "BattleDamagePresenter.h"
#include "BattleTargetHUDPresenter.h"
#include "BattleBoardPresenter.h"

void BattleUIOrchestrator::OnBoot()
{
	eventBus           = &registry.Get<BattleEventBus>();
	uiRegistry         = &registry.Get<UIRegistry>();
	uiSys              = &registry.Get<UISystem>();
	uiAnimSys          = &registry.Get<UIAnimSystem>();
	timelinePresenter  = &registry.Get<BattleTimelinePresenter>();
	inputPresenter     = &registry.Get<PlayerInputPresenter>();
	HUDPresenter       = &registry.Get<BattleHUDPresenter>();
	fatalPresenter     = &registry.Get<BattleFatalDrivePresenter>();
	dmgPresenter       = &registry.Get<BattleDamagePresenter>();
	targetHUDPresenter = &registry.Get<BattleTargetHUDPresenter>();
	boardPresenter     = &registry.Get<BattleBoardPresenter>();
}

void BattleUIOrchestrator::Enter()
{
	timelinePresenter->Enter();
	timelinePresenter->SetAbsoluteLayout(700.f, 900.f, 1200.f, 1000.f, 550.f);
	inputPresenter->Enter();
	HUDPresenter->Enter();
	fatalPresenter->Enter();
	targetHUDPresenter->Enter();
	boardPresenter->Enter();
	dmgPresenter->Enter();

	for (auto& [key, spec] : uiRegistry->GetArchetypes())
	{
		if (spec.context == UIContext::Battle && spec.startEnabled)
			uiRegistry->Ensure(key);
	}

	HUDPresenter->Tick(0.f);
	inputPresenter->Tick(0.f);
	timelinePresenter->Tick(0.f);
	fatalPresenter->Tick(0.f);
	dmgPresenter->Tick(0.f);
	targetHUDPresenter->Tick(0.f);

	uiAnimSys->Tick(0.f);
	uiSys->Tick(0.f);
}

void BattleUIOrchestrator::Tick(float dt)
{
	HUDPresenter->Tick(dt);
	inputPresenter->Tick(dt);
	timelinePresenter->Tick(dt);
	fatalPresenter->Tick(dt);
	dmgPresenter->Tick(dt);
	targetHUDPresenter->Tick(dt);
	boardPresenter->Tick(dt);
}

void BattleUIOrchestrator::Exit()
{
	timelinePresenter->Exit();
	inputPresenter->Exit();
	HUDPresenter->Exit();
	fatalPresenter->Exit();
	targetHUDPresenter->Exit();
	boardPresenter->Exit();

	for (auto& [key, inst] : uiRegistry->GetInstances())
		uiRegistry->SetEnabled(key, false);
}