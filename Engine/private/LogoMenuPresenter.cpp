#include "Enginepch.h"
#include "LogoMenuPresenter.h"
#include "ScreenFadeSystem.h"
#include "WorldMapPresenter.h"
#include "SoundSystem.h"

void LogoMenuPresenter::OnBoot()
{
    uiRegistry = &registry.Get<UIRegistry>();
    uiAnimSys  = &registry.Get<UIAnimSystem>();
    input      = &registry.Get<InputService>();
    director   = &registry.Get<GameModeDirectorSystem>();
    uiSys      = &registry.Get<UISystem>();
    fadeSys    = &registry.Get<ScreenFadeSystem>();
    soundSys   = &registry.Get<SoundSystem>();

    InitButtons();
    phase = LogoPhase::PressAny;
    pressAnyTime = 0.f;
}

void LogoMenuPresenter::Enter()
{
    phase = LogoPhase::PressAny;
    pressAnyTime = 0.f;
    hoverBarTime = 0.f;

    uiRegistry->Ensure(pressAnyKey);
    uiRegistry->SetEnabled(pressAnyKey, true);
    uiAnimSys->SetOpacity(pressAnyKey, 0.3f);

    for (auto& btn : buttons)
    {
        uiRegistry->Ensure(btn.keyNormal);
        uiRegistry->Ensure(btn.keyHover);
        uiRegistry->Ensure(titleBar);
        uiRegistry->Ensure(logobg);

        uiRegistry->SetEnabled(btn.keyNormal, false);
        uiRegistry->SetEnabled(btn.keyHover, false);

        btn.state = UIWidgetState::None;

        uiAnimSys->SetScale(btn.keyNormal, 1.f, 1.f);
        uiAnimSys->SetScale(btn.keyHover, 1.f, 1.f);
        uiAnimSys->SetOpacity(btn.keyNormal, 1.f);
        uiAnimSys->SetOpacity(btn.keyHover, 1.f);
    }
    uiRegistry->Ensure(hoverBarKey);
    uiRegistry->SetEnabled(hoverBarKey, false);
}

void LogoMenuPresenter::Tick(float dt)
{
    if (phase == LogoPhase::PressAny)
    {
        const bool anyClicked = input->KeyDown(KEY::SPACE);

        pressAnyTime += dt;
        const float s = (sinf(pressAnyTime * 3.f) + 1.f) * 0.5f; 
        const float alpha = 0.1f + 0.9f * s;
        uiAnimSys->SetOpacity(pressAnyKey, alpha);

        if (anyClicked)
        {
            uiRegistry->SetEnabled(pressAnyKey, false);

            for (auto& btn : buttons)
            {
                uiRegistry->SetEnabled(btn.keyNormal, true);
                uiRegistry->SetEnabled(btn.keyHover, false);

                uiAnimSys->SetScale(btn.keyNormal, 1.f, 1.f);
                uiAnimSys->SetScale(btn.keyHover, 1.f, 1.f);
                uiAnimSys->SetOpacity(btn.keyNormal, 1.f);
                uiAnimSys->SetOpacity(btn.keyHover, 1.f);

                btn.state = UIWidgetState::None;
            }
            phase = LogoPhase::Menu;
            hoverBarTime = 0.f;
        }
    }
    else if (phase == LogoPhase::Menu)
    {
        TickButtons(dt);
    }
    else if (phase == LogoPhase::FadingToField)
    {
        if (fadeSys->IsFullyBlack())
        {
            onCommand(LogoMenuCommand::NewGame);
        }
    }
}

void LogoMenuPresenter::Exit()
{
    fadeSys->FadeOut();

    for (auto& btn : buttons)
    {
        uiRegistry->SetEnabled(btn.keyNormal, false);
        uiRegistry->SetEnabled(btn.keyHover, false);
    }
    uiRegistry->SetEnabled(hoverBarKey, false);
    uiRegistry->SetEnabled(pressAnyKey, false);
    uiRegistry->SetEnabled(titleBar, false);
    uiRegistry->SetEnabled(logobg, false);

    registry.Get<WorldMapPresenter>().Enter();
    //registry.Get<WorldMapPresenter>().TeleportToSpot(6);
    registry.Get<WorldMapPresenter>().Exit();
}

void LogoMenuPresenter::InitButtons()
{
    buttons.clear();
    buttons.push_back({ LogoButtonID::NewGame,  L"new_game_0",  L"new_game_1" });
    buttons.push_back({ LogoButtonID::LoadGame, L"load_game_0", L"load_game_1" });
    buttons.push_back({ LogoButtonID::Setting,  L"setting_0",   L"setting_1" });
    buttons.push_back({ LogoButtonID::Exit,     L"exit_game_0", L"exit_game_1" });
}

void LogoMenuPresenter::TickButtons(float dt)
{
    const bool isDown = input->KeyPressing(KEY::LBUTTON);
    const bool isEdgeUp = input->KeyReleased(KEY::LBUTTON);

    LogoButton* hoveredBtn = nullptr;

    for (auto& btn : buttons)
    {
        UIWidgetState prev = btn.state;
        UIWidgetState next = UIWidgetState::None;

        const bool inside = HitTest(btn);

        if (!inside)
            next = UIWidgetState::None;
        else
        {
            if (isDown)
                next = UIWidgetState::Pressed;
            else
                next = UIWidgetState::Hovered;
        }

        if (prev != next)
        {
            switch (next)
            {
            case UIWidgetState::None:
                uiRegistry->SetEnabled(btn.keyNormal, true);
                uiRegistry->SetEnabled(btn.keyHover, false);
                uiAnimSys->ScaleTo(btn.keyNormal, 1.f, 1.f, 0.08f);
                break;

            case UIWidgetState::Hovered:
                uiRegistry->SetEnabled(btn.keyNormal, false);
                uiRegistry->SetEnabled(btn.keyHover, true);
                uiAnimSys->ScaleTo(btn.keyHover, 1.08f, 1.08f, 0.08f);
                break;

            case UIWidgetState::Pressed:
                uiRegistry->SetEnabled(btn.keyNormal, false);
                uiRegistry->SetEnabled(btn.keyHover, true);
                uiAnimSys->ScaleTo(btn.keyHover, 0.95f, 0.95f, 0.05f);
                break;
            default:
                break;
            }
        }

        if (prev == UIWidgetState::Pressed && next == UIWidgetState::Hovered && isEdgeUp && inside)
            HandleClick(btn.id);

        if (next == UIWidgetState::Hovered)
            hoveredBtn = &btn;

        btn.state = next;
    }

    if (hoveredBtn)
    {
        UIInstance& btnInst = uiRegistry->Ensure(hoveredBtn->keyHover);
        UIInstance& barInst = uiRegistry->Ensure(hoverBarKey);

        uiRegistry->SetLocalPos(hoverBarKey, btnInst.localX, btnInst.localY);
        uiRegistry->SetEnabled(hoverBarKey, true);

        hoverBarTime += dt;
        const float s = (sinf(hoverBarTime * 4.f) + 1.f) * 0.5f; 
        const float alpha = 0.5f + 0.5f * s;                       
        uiAnimSys->SetOpacity(hoverBarKey, alpha);
    }
    else
    {
        hoverBarTime = 0.f;
        uiRegistry->SetEnabled(hoverBarKey, false);
    }
}

void LogoMenuPresenter::HandleClick(LogoButtonID id)
{
    switch (id)
    {
    case LogoButtonID::NewGame:  
        soundSys->Play(L"Press_Start", 0.15f);
        fadeSys->FadeIn(0.5f);
        phase = LogoPhase::FadingToField;
        break;

    case LogoButtonID::LoadGame: 
        onCommand(LogoMenuCommand::LoadGame); 
        break;

    case LogoButtonID::Setting:  
        onCommand(LogoMenuCommand::OpenSetting);
        break;

    case LogoButtonID::Exit:  
        onCommand(LogoMenuCommand::ExitGame); 
        break;
    }
}

bool LogoMenuPresenter::HitTest(const LogoButton& btn) const
{
    const auto& instances = uiRegistry->GetInstances();
    auto it = instances.find(btn.keyNormal);
    assert(it != instances.end());

    const UIInstance& inst = it->second;
    assert(inst.spec);

    const UIArchetypeSpec& spec = *inst.spec;

    const auto& viewport = GAME.GetViewport();
    const float screenW = static_cast<float>(viewport.Width);
    const float screenH = static_cast<float>(viewport.Height);

    const wstring texKey = inst.overrideKey ? *inst.overrideKey : spec.texKey;
    const auto   sizeWH = uiRegistry->GetOrCacheTexSize(texKey);
    const float  srcW = sizeWH.first;
    const float  srcH = sizeWH.second;

    float drawW = srcW;
    float drawH = srcH;
    switch (spec.sizeMode)
    {
    case UISizeMode::Original:
        break;
    case UISizeMode::Fixed:
        drawW = spec.fixedWidth;
        drawH = spec.fixedHeight;
        break;
    case UISizeMode::Ratio:
        drawW = srcW * spec.ratioX;
        drawH = srcH * spec.ratioY;
        break;
    }

    // Anchor
    auto anchorNorm = uiSys->ToNorm(spec.anchor);
    const float anchorX = anchorNorm.first * screenW;
    const float anchorY = anchorNorm.second * screenH;

    const float scaledW = drawW * inst.animScaleX;
    const float scaledH = drawH * inst.animScaleY;

    auto pivotNorm = uiSys->ToNorm(spec.pivot);
    const float pivotX = pivotNorm.first * scaledW;
    const float pivotY = pivotNorm.second * scaledH;

    const float x = anchorX + inst.localX - pivotX + inst.animOffsetX;
    const float y = anchorY + inst.localY - pivotY + inst.animOffsetY;

    const float left = x;
    const float right = x + scaledW;
    const float top = y;
    const float bottom = y + scaledH;

    const _float2& mp = input->GetMousePos();
    const float mx = mp.x;
    const float my = mp.y;

    return (mx >= left && mx <= right && my >= top && my <= bottom);
}