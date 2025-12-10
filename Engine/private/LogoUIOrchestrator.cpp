#include "Enginepch.h"
#include "LogoUIOrchestrator.h"
#include "LogoMenuPresenter.h"
#include "SoundSystem.h"

void LogoUIOrchestrator::OnBoot()
{
    uiRegistry    = &registry.Get<UIRegistry>();
    uiSys         = &registry.Get<UISystem>();
    uiAnimSys     = &registry.Get<UIAnimSystem>();
    input         = &registry.Get<InputService>();
    director      = &registry.Get<GameModeDirectorSystem>();
    menuPresenter = &registry.Get<LogoMenuPresenter>();
}

void LogoUIOrchestrator::Enter()
{
    const auto& archetypes = uiRegistry->GetArchetypes();
    assert(!archetypes.empty() && "UI archetypes must be registered before LogoUIOrchestrator::Enter");

    for (auto& [key, spec] : archetypes)
    {
        if (spec.context == UIContext::Loading && spec.startEnabled)
            uiRegistry->Ensure(key);
    }

    menuPresenter->Enter();

    menuPresenter->Tick(0.f);
    uiAnimSys->Tick(0.f);
    uiSys->Tick(0.f);
}

void LogoUIOrchestrator::Tick(float dt)
{
    menuPresenter->Tick(dt);
}

void LogoUIOrchestrator::Exit()
{
    menuPresenter->Exit();

    for (auto& [key, inst] : uiRegistry->GetInstances())
    {
        const UIArchetypeSpec* spec = inst.spec;
        if (spec && spec->context == UIContext::Loading) 
            uiRegistry->SetEnabled(key, false);
    }
    //ShowCursor(false);
}