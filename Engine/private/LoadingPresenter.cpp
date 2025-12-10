#include "Enginepch.h"
#include "LoadingPresenter.h"

void LoadingPresenter::OnBoot()
{
	uiRegistry = &registry.Get<UIRegistry>();
	uiAnimSys  = &registry.Get<UIAnimSystem>();
}

void LoadingPresenter::Enter()
{
    state = State::FadingIn;
    stateTime = 0.f;
    loadingFinished = false;

    InitDotsIfNeeded();

    uiRegistry->Ensure(bgKey);
    uiRegistry->Ensure(textKey);

    uiRegistry->SetEnabled(bgKey, true);
    uiRegistry->SetEnabled(textKey, true);

    uiAnimSys->SetOpacity(bgKey, 0.f);
    uiAnimSys->SetOpacity(textKey, 0.f);

    uiAnimSys->PlayFadeOnce(bgKey, 0.f, 1.f, fadeInDur);
    uiAnimSys->PlayFadeOnce(textKey, 0.f, 1.f, fadeInDur);

    visibleDotCount = 1;
    dotTimer = 0.f;
    ApplyDotVisibility();
}

void LoadingPresenter::Tick(float dt)
{
    if (state == State::Hidden)
        return;

    stateTime += dt;

    switch (state)
    {
    case State::FadingIn:
        if (stateTime >= fadeInDur)
        {
            state = State::Showing;
            stateTime = 0.f;
        }
        break;

    case State::Showing:
        UpdateDots(dt);

        if (loadingFinished)
        {
            state = State::FadingOut;
            stateTime = 0.f;

            uiAnimSys->PlayFadeOnce(bgKey, 1.f, 0.f, fadeOutDur);
            uiAnimSys->PlayFadeOnce(textKey, 1.f, 0.f, fadeOutDur);

            for (const wstring& dotKey : dotKeys)
                uiAnimSys->PlayFadeOnce(dotKey, 1.f, 0.f, fadeOutDur);
        }
        break;

    case State::FadingOut:
        UpdateDots(dt);

        if (stateTime >= fadeOutDur)
        {
            uiRegistry->SetEnabled(bgKey, false);
            uiRegistry->SetEnabled(textKey, false);

            for (const wstring& dotKey : dotKeys)
                uiRegistry->SetEnabled(dotKey, false);

            state = State::Hidden;
            stateTime = 0.f;
        }
        break;

    default:
        break;
    }
}

void LoadingPresenter::Exit()
{
    uiRegistry->SetEnabled(bgKey, false);
    uiRegistry->SetEnabled(textKey, false);

    for (const wstring& dotKey : dotKeys)
        uiRegistry->SetEnabled(dotKey, false);

    state = State::Hidden;
    stateTime = 0.f;
    loadingFinished = false;
}

void LoadingPresenter::OnLoadingComplete()
{
    loadingFinished = true;
}

void LoadingPresenter::InitDotsIfNeeded()
{
	if (dotsInitialized) return;

    UIInstance& baseDot = uiRegistry->Ensure(dotBaseKey);
    baseDot.selfEnabled = false;

    const float dotSpacing = 24.f;

    for (int i = 0; i < 3; ++i)
    {
        const wstring& instKey = dotKeys[static_cast<size_t>(i)];

        UIInstance& dot = uiRegistry->EnsureClone(dotBaseKey, instKey);
        dot.selfEnabled = false;
        dot.localX = baseDot.localX + dotSpacing * static_cast<float>(i);
        dot.localY = baseDot.localY;

        uiAnimSys->SetOpacity(instKey, 1.f);
    }

    dotsInitialized = true;
}

void LoadingPresenter::UpdateDots(float dt)
{
    dotTimer += dt;
    if (dotTimer < dotInterval)
        return;

    dotTimer -= dotInterval;

    visibleDotCount += 1;
    if (visibleDotCount > 3)
        visibleDotCount = 1;

    ApplyDotVisibility();
}

void LoadingPresenter::ApplyDotVisibility()
{
    for (int i = 0; i < 3; ++i)
    {
        const wstring& key = dotKeys[static_cast<size_t>(i)];

        const bool enable = (i < visibleDotCount);
        uiRegistry->SetEnabled(key, enable);
    }
}