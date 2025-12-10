#include "Enginepch.h"
#include "FieldMinimapPresenter.h"
#include "WorldMapPresenter.h"
#include "ScreenFadeSystem.h"
#include "DressingRoomPresenter.h"

void FieldUIOrchestrator::OnBoot()
{
	uiRegistry        = &registry.Get<UIRegistry>();
	uiSys             = &registry.Get<UISystem>();
	uiAnimSys         = &registry.Get<UIAnimSystem>();
	fieldMini         = &registry.Get<FieldMinimapPresenter>();
	worldPresenter    = &registry.Get<WorldMapPresenter>();
	input             = &registry.Get<InputService>();
	fadeSys           = &registry.Get<ScreenFadeSystem>();
    dressingPresenter = &registry.Get<DressingRoomPresenter>();
}

void FieldUIOrchestrator::Enter()
{
	const auto& archetypes = uiRegistry->GetArchetypes();
	assert(!archetypes.empty() && "UI archetypes must be registered BEFORE FieldUIOrchestrator::Enter()");

	for (auto& [key, spec] : archetypes)
	{
		if (spec.context == UIContext::Field && spec.startEnabled)
			uiRegistry->Ensure(key);
	}

	fieldMini->Enter();
	worldMapOpen = false;
	worldPhase   = WorldMapPhase::Idle;

    dressingOpen = false;
    dressingPhase = DressingPhase::Hidden;
    dressingPresenter->Exit();
}

void FieldUIOrchestrator::Tick(float dt)
{
    // WorldMap Toggle
    if (input->KeyDown(KEY::M) && !fadeSys->IsBusy())
    {
        if (!worldMapOpen)
        {
            fadeSys->FadeIn(1.f);
            worldPhase = WorldMapPhase::FadingOpen;
        }
        else
        {
            fadeSys->FadeIn(1.f);
            worldPhase = WorldMapPhase::FadingClose;
        }
    }
    // DressingRoom Toggle
    if (input->KeyDown(KEY::I) && !fadeSys->IsBusy() && worldPhase == WorldMapPhase::Idle)
    {
        if (!dressingOpen)
        {
            fadeSys->FadeIn(1.f);
            dressingPhase = DressingPhase::FadingOpen;
        }
        else
        {
            fadeSys->FadeIn(1.f);
            dressingPhase = DressingPhase::FadingClose;
        }
    }
    // WorldMap Phase
    switch (worldPhase)
    {
    case WorldMapPhase::FadingOpen:
        if (fadeSys->IsFullyBlack())
        {
            fieldMini->Exit();
            worldPresenter->Enter();
            worldMapOpen = true;

            fadeSys->FadeOut(1.f);    
            worldPhase = WorldMapPhase::Showing;
        }
        break;

    case WorldMapPhase::FadingClose:
        if (fadeSys->IsFullyBlack())
        {
            worldPresenter->Exit();
            fieldMini->Enter();
            worldMapOpen = false;
            fadeSys->FadeOut(1.f);
            worldPhase = WorldMapPhase::Idle;
        }
        break;

    default:
        break;
    }
    // Dressing Room Phase
    switch (dressingPhase)
    {
    case DressingPhase::FadingOpen:
        if (fadeSys->IsFullyBlack())
        {
            dressingPresenter->Enter();
            dressingOpen = true;

            fadeSys->FadeOut(1.f);
            dressingPhase = DressingPhase::Showing;
        }
        break;

    case DressingPhase::FadingClose:
        if (fadeSys->IsFullyBlack())
        {
            dressingPresenter->Exit();
            dressingOpen = false;

            fadeSys->FadeOut(1.f);
            dressingPhase = DressingPhase::Hidden;
        }
        break;

    default:
        break;
    }

    // Tick
    if (dressingOpen)
        dressingPresenter->Tick(dt);
    else if (worldMapOpen)
        worldPresenter->Tick(dt);
    else
        fieldMini->Tick(dt);

    if (!dressingOpen && worldMapOpen && worldPhase == WorldMapPhase::Showing && worldPresenter->HasTeleport() && !fadeSys->IsBusy())
    {
        fadeSys->FadeIn(1.f);
        worldPhase = WorldMapPhase::FadingClose;
    }

    switch (mapTitlePhase)
    {
    case MapTitlePhase::Hidden:
        break;

    case MapTitlePhase::Waiting:
        mapTitleTimer += dt;
        if (mapTitleTimer >= mapTitleDelay)
        {
            mapTitleTimer = 0.f;
            mapTitlePhase = MapTitlePhase::Showing;

            UIInstance& bar = uiRegistry->Ensure(mapTitleBarKey);
            UIInstance& spinner = uiRegistry->Ensure(mapTitleSpinnerKey);
            UIInstance& text = uiRegistry->Ensure(mapTitleTextKey);

            bar.selfEnabled = true;
            spinner.selfEnabled = true;
            text.selfEnabled = true;

            uiRegistry->SetEnabled(mapTitleBarKey, true);
            uiRegistry->SetEnabled(mapTitleSpinnerKey, true);
            uiRegistry->SetEnabled(mapTitleTextKey, true);

            uiAnimSys->SetOpacity(mapTitleBarKey, 0.f);
            uiAnimSys->SetOpacity(mapTitleSpinnerKey, 0.f);
            uiAnimSys->SetOpacity(mapTitleTextKey, 0.f);

            constexpr float fadeDur = 0.6f;

            uiAnimSys->SetScale(mapTitleSpinnerKey, 1.3f, 1.3f);

            uiAnimSys->PlayFadeOnce(mapTitleBarKey, 0.f, 1.f, fadeDur, UIEasing::Linear);
            uiAnimSys->PlayFadeOnce(mapTitleTextKey, 0.f, 1.f, fadeDur, UIEasing::Linear);

            uiAnimSys->PlayFadeOnce(mapTitleSpinnerKey, 0.f, 1.f, fadeDur, UIEasing::Linear);
            uiAnimSys->ScaleTo(mapTitleSpinnerKey, 1.f, 1.f, fadeDur);

            uiAnimSys->Spin(mapTitleSpinnerKey, 120.f);
            uiSys->SetText(mapTitleTextKey, mapTitlePendingName);
        }
        break;

    case MapTitlePhase::Showing:
        mapTitleTimer += dt;
        if (mapTitleTimer >= mapTitleDur)
        {
            mapTitleTimer = 0.6f;
            mapTitlePhase = MapTitlePhase::Hidden;

            constexpr float fadeOutDur = 0.4f;

            uiAnimSys->PlayFadeOnce(mapTitleBarKey, 1.f, 0.f, fadeOutDur);
            uiAnimSys->PlayFadeOnce(mapTitleTextKey, 1.f, 0.f, fadeOutDur);
            uiAnimSys->PlayFadeOnce(mapTitleSpinnerKey, 1.f, 0.f, fadeOutDur);

            uiAnimSys->StopSpin(mapTitleSpinnerKey);
        }
        break;
    }
}

void FieldUIOrchestrator::Exit()
{
	fieldMini->Exit();
	worldPresenter->Exit();
    dressingPresenter->Exit();

	for (auto& [key, inst] : uiRegistry->GetInstances())
	{
		const UIArchetypeSpec* spec = inst.spec;
		if (spec && spec->context == UIContext::Field)
			uiRegistry->SetEnabled(key, false);
	}
}

void FieldUIOrchestrator::ShowMapTitle(const wstring& name)
{
    mapTitlePendingName = name;

    if (mapTitlePhase == MapTitlePhase::Showing)
    {
        uiRegistry->SetEnabled(mapTitleBarKey, false);
        uiRegistry->SetEnabled(mapTitleSpinnerKey, false);
        uiRegistry->SetEnabled(mapTitleTextKey, false);
        uiAnimSys->StopSpin(mapTitleSpinnerKey);
    }

    mapTitlePhase = MapTitlePhase::Waiting;
    mapTitleTimer = 0.f;
}