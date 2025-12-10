#include "EnginePch.h"
#include "ScreenFadeSystem.h"

void ScreenFadeSystem::OnBoot()
{
	uiRegistry = &registry.Get<UIRegistry>();
}

void ScreenFadeSystem::Init()
{
    fadeBlack = &uiRegistry->Ensure(L"black");
    fadeWhite = &uiRegistry->Ensure(L"white");

    mode = FadeMode::Black;
    activeInst = fadeBlack;

    curAlpha    = 1.f;
    startAlpha  = 1.f;
    targetAlpha = 1.f;
    timer       = 0.f;
    dur         = 0.f;
    active      = false;

    fadeBlack->animAlpha   = curAlpha;
    fadeBlack->selfEnabled = true;
    fadeWhite->animAlpha   = 0.f;
    fadeWhite->selfEnabled = false;
}

void ScreenFadeSystem::SetMode(FadeMode newMode)
{
    mode = newMode;
    activeInst = (mode == FadeMode::Black) ? fadeBlack : fadeWhite;

    if (mode == FadeMode::Black)
    {
        fadeWhite->selfEnabled = false;
        fadeWhite->animAlpha   = 0.f;
    }
    else
    {
        fadeBlack->selfEnabled = false;
        fadeBlack->animAlpha   = 0.f;
    }
    activeInst->animAlpha = curAlpha;
    activeInst->selfEnabled = (curAlpha > 0.f);
}

void ScreenFadeSystem::Tick(float dt)
{
    if (!active) return;

    timer += dt;
    float t = (dur > 0.f) ? min(timer / dur, 1.f) : 1.f;

    curAlpha = lerp(startAlpha, targetAlpha, t);

    if (t >= 1.f)
    {
        curAlpha = targetAlpha;
        active = false;
    }

    activeInst->animAlpha = curAlpha;
    activeInst->selfEnabled = (curAlpha > 0.f);
}

void ScreenFadeSystem::FadeIn(float _dur)
{
    SetMode(FadeMode::Black);
    BeginFade(1.f, _dur);
}

void ScreenFadeSystem::FadeOut(float _dur)
{
    SetMode(FadeMode::Black);
    BeginFade(0.f, _dur);
}

void ScreenFadeSystem::FadeInWhite(float _dur)
{
    SetMode(FadeMode::White);
    BeginFade(1.f, _dur);
}

void ScreenFadeSystem::FadeOutWhite(float _dur)
{
    SetMode(FadeMode::White);
    BeginFade(0.f, _dur);
}

void ScreenFadeSystem::BeginFade(float target, float _dur)
{
    startAlpha  = curAlpha;
    targetAlpha = target;
    dur         = _dur;
    timer       = 0.f;
    active      = true;
    activeInst->selfEnabled = true;
}