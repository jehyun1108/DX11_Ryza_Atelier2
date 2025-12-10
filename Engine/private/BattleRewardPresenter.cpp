#include "Enginepch.h"
#include "BattleRewardPresenter.h"
#include "ScreenFadeSystem.h"
#include "SoundSystem.h"
#include "UISystem.h"

void BattleRewardPresenter::OnBoot()
{
    animator   = &registry.Get<AnimatorSystem>();
    animSys    = &registry.Get<AnimDataSystem>();
    dataSys    = &registry.Get<CharacterDataSystem>();
    fadeSys    = &registry.Get<ScreenFadeSystem>();
    camReg     = &registry.Get<CamRegistry>(); 
    soundSys   = &registry.Get<SoundSystem>();
    uiSys      = &registry.Get<UISystem>();
    uiRegistry = &registry.Get<UIRegistry>();
    uiAnimSys  = &registry.Get<UIAnimSystem>();

    rewardFade = camReg->GetFadeProfile(CamRole::Reward);
}

void BattleRewardPresenter::BeginVictory(const vector<EntityID>& order)
{
    steps.clear();
    steps.reserve(order.size());
    uiSys->SetActiveContext(UIContext::None);
    soundSys->PlayBgm(L"Battle_Reward_BGM", 0.15f);

    for (EntityID entity : order)
    {
        if (entity == 0u) continue;

        Step s{};
        s.entity = entity;
        s.phase = 0;
        s.timer = 0.f;
        s.durA = ResolveClipDur(entity, AnimKey::Battle_Ceremony_1A);
        s.durB = ResolveClipDur(entity, AnimKey::Battle_Ceremony_1B);
        s.gapAfter = 0.25f;
        steps.push_back(s);
    }

    curIndex = steps.empty() ? -1 : 0;
    active = !steps.empty();
    finished = steps.empty();
    resultUiShown = false;

    slowDur = 1.0f;
    slowTimeRemaining = slowDur;
    slowFactor = 0.2f;

    if (!active)
    {
        state = RewardState::Idle;
        return;
    }

    state = RewardState::FadeInToStep;

    if (rewardFade.useFade && fadeSys)
    {
        if (rewardFade.mode == FadeMode::White)
            fadeSys->FadeInWhite(rewardFade.inDur);
        else
            fadeSys->FadeIn(rewardFade.inDur);
    }
    else
    {
        StartStep(curIndex);
        state = RewardState::PlayingStep;
    }
}

void BattleRewardPresenter::Tick(float dt)
{
    if (!active || finished)
        return;

    switch (state)
    {
    case RewardState::FadeInToStep:
    {
        bool ready = true;
        if (rewardFade.useFade)
        {
            if (rewardFade.mode == FadeMode::White)
                ready = fadeSys->IsFullyWhite(); 
            else
                ready = fadeSys->IsFullyBlack();   
        }

        if (ready)
        {
            StartStep(curIndex);

            if (rewardFade.useFade)
            {
                if (rewardFade.mode == FadeMode::White)
                    fadeSys->FadeOutWhite(rewardFade.outDur);
                else
                    fadeSys->FadeOut(rewardFade.outDur);
            }

            state = RewardState::PlayingStep;
        }
    }
    break;

    case RewardState::PlayingStep:
        UpdateCeremony(dt);
        break;

    case RewardState::Idle:
    default:
        break;
    }
}

float BattleRewardPresenter::CalcBattleDt(float dt)
{
    if (slowTimeRemaining <= 0.f)
        return dt;

    slowTimeRemaining -= dt;
    if (slowTimeRemaining <= 0.f)
        return dt;

    const float t = 1.f - (slowTimeRemaining / slowDur);
    const float k = lerp(slowFactor, 1.f, t);
    return dt * k;
}

void BattleRewardPresenter::StartStep(int idx)
{
    Step& s = steps[idx];
    s.phase = 0;
    s.timer = 0.f;
    PlayClip(s.entity, AnimKey::Battle_Ceremony_1A, 0.15f);

    CharacterID ch = dataSys->GetCharacterID(s.entity);
    camReg->PlayRewardCam(ch, s.entity);

    switch (ch)
    {
    case CharacterID::Ryza:
        soundSys->Play(L"ryza_41");
        break;
    case CharacterID::Patricia:
        soundSys->Play(L"patricia_35");
        break;
    case CharacterID::Klaudia:
        soundSys->Play(L"klaudia_37");
        break;
    default:
        break;
    }
}

void BattleRewardPresenter::UpdateCeremony(float dt)
{
    if (curIndex < 0 || curIndex >= (int)steps.size())
    {
        active = false;
        finished = true;
        state = RewardState::Idle;
        return;
    }

    Step& s = steps[curIndex];
    s.timer += dt;

    if (s.phase == 0)
    {
        if (s.timer >= s.durA)
        {
            s.phase = 1;
            s.timer = 0.f;
            PlayClip(s.entity, AnimKey::Battle_Ceremony_1B, 0.1f);
        }
    }
    else if (s.phase == 1)
    {
        // --- 마지막 캐릭터의 B 동작이 거의 끝났을 때 살짝 먼저 띄우고 싶으면 (예: 80% 시점) ---
        bool isLastStep = (curIndex == (int)steps.size() - 1);
        if (isLastStep && !resultUiShown)
        {
            float t01 = (s.durB > 0.f) ? (s.timer / s.durB) : 1.f;
            if (t01 >= 0.8f)        // ★ "거의 끝날 때쯤" 느낌
            {
                ShowResultUI();     // 내부에 resultUiShown 체크 있으니 중복 호출 문제 없음
            }
        }

        // --- 원래 페이즈 전환 로직 ---
        if (s.timer >= s.durB)
        {
            s.phase = 2;
            s.timer = 0.f;

            ++curIndex;
            if (curIndex >= (int)steps.size())
            {
                // 모든 캐릭터 세레머니 끝난 시점
                if (!resultUiShown)
                    ShowResultUI();          // 혹시 0.8 에 못 걸렸으면 여기서라도 한 번 더 보장

                active = false;
                finished = true;
                state = RewardState::Idle;
            }
            else
            {
                state = RewardState::FadeInToStep;

                if (rewardFade.useFade && fadeSys)
                {
                    if (rewardFade.mode == FadeMode::White)
                        fadeSys->FadeInWhite(rewardFade.inDur);
                    else
                        fadeSys->FadeIn(rewardFade.inDur);
                }
            }
        }
    }
}

void BattleRewardPresenter::PlayClip(EntityID entity, AnimKey key, float crossFadeDur)
{
    CharacterID ch = dataSys->GetCharacterID(entity);
    const wstring& clipName = animSys->GetClipName(ch, AnimContext::Battle, key);
    ClipTuning     tuning = animSys->GetClipTuning(ch, AnimContext::Battle, key);
    animator->CrossFade(animator->Get(entity), 0, 1, clipName, crossFadeDur, ANIMTYPE::ONCE, tuning.startNormalized, tuning.endNormalized );
}

float BattleRewardPresenter::ResolveClipDur(EntityID entity, AnimKey key) const
{
    CharacterID ch = dataSys->GetCharacterID(entity);
    const wstring& clipName = animSys->GetClipName(ch, AnimContext::Battle, key);
    return animator->GetClipDuration(animator->Get(entity), clipName);
}

void BattleRewardPresenter::ShowResultUI()
{
    if (resultUiShown) return;
    resultUiShown = true;

    auto fadeIn = [&](const wstring& key, float dur)
        {
            uiRegistry->Ensure(key);
            uiRegistry->SetEnabled(key, true);
            uiAnimSys->SetOpacity(key, 0.f);
            uiAnimSys->PlayFadeOnce(key, 0.f, 1.f, dur);
        };
    fadeIn(L"battle_result", 0.45f);
    fadeIn(L"battle_result_under", 0.45f);

    fadeIn(L"ryza_reward", 0.40f);
    fadeIn(L"klaudia_reward", 0.40f);
    fadeIn(L"patricia_reward", 0.40f);
    fadeIn(L"serri_reward", 0.40f);
}