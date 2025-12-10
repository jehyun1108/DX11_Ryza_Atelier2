#include "Enginepch.h"

namespace
{
    constexpr float kIntroBackOffset = 350.f;  
    constexpr float kArriveRadius    = 10.f;   
    constexpr float kIntroRunSpeed   = 500.f;  
    constexpr float kFadeShort       = 0.06f;  
}
static inline _float3 ComputeIntroSpawnPos(const _float3& target, const _float2& faceDirXZ, float backOffset)
{
    return _float3{ target.x - faceDirXZ.x * backOffset, target.y, target.z - faceDirXZ.y * backOffset };
}
// =================================================================================================================

void BattleIntroSystem::OnBoot()
{
    animator     = &registry.Get<AnimatorSystem>();
    tfSys        = &registry.Get<TransformSystem>();
    faceSrv      = &registry.Get<FacingForceService>();
    sessionSys   = &registry.Get<BattleSessionSystem>();
    formationSys = &registry.Get<BattleFormationSystem>();
    animDataSys  = &registry.Get<AnimDataSystem>();
    actionReg    = &registry.Get<ActionAnimRegistry>();
}

Handle BattleIntroSystem::Create(EntityID owner, Handle animHandle, Handle tfHandle, const AnimProfile& profile)
{
    Handle handle = CreateComp(owner);
    auto& state = *Get(handle);
    state.entity = owner;
    state.animHandle = animHandle;
    state.tfHandle = tfHandle;
    state.profile = profile;
    state.stage = BattleIntroStage::IntroStart;

    auto [team, slot] = GetTeamSlot(owner);
    _float3 targetWorld = GetFormationTarget(owner);
    _float2 faceDirXZ = GetFormationFace(owner);

    const _float3 spawnPos = ComputeIntroSpawnPos(targetWorld, faceDirXZ, kIntroBackOffset);
    tfSys->SetPos(state.tfHandle, spawnPos);
    faceSrv->PushSnapXZ(owner, faceDirXZ);

    const ActionAnimSpec& spec = actionReg->Get(profile.character);
    const bool hasIntro = spec.specials.contains(SpecialAnimTag::Intro);

    if (hasIntro)
    {
        const AnimChainSpec& chain = actionReg->GetSpecial(profile.character, SpecialAnimTag::Intro);
        state.introChain = &chain;
        state.chainStageIdx = !chain.stages.empty() ? 0 : -1;

        if (state.chainStageIdx >= 0)
        {
            const AnimStageSpec& first = chain.stages[0];
            PlayKey(state, first.clipKey, ANIMTYPE::ONCE, max(0.f, first.fadeDur));
        }
        else
        {
            PlayKey(state, AnimKey::Battle_RunStart, ANIMTYPE::ONCE, kFadeShort);
            state.stage = BattleIntroStage::RunStart;
        }
    }
    else
    {
        // 인트로 미정의: 기존 fallback
        PlayKey(state, AnimKey::Battle_RunStart, ANIMTYPE::ONCE, kFadeShort);
        state.stage = BattleIntroStage::RunStart;
    }

    return handle;
}

void BattleIntroSystem::Update(float dt)
{
    ForEachAliveEx([&](Handle handle, EntityID owner, BattleIntroState& state)
        {
            state.elapsed += dt;

            auto [team, slot]   = GetTeamSlot(owner);
            _float3 targetWorld = GetFormationTarget(owner);
            _float2 faceDirXZ   = GetFormationFace(owner);

            switch (state.stage)
            {
            case BattleIntroStage::IntroStart:
            {
                faceSrv->PushSnapXZ(owner, faceDirXZ);

                if (IsCurClipFinished(state))
                {
                    const bool advanced = state.introChain ? PlayNextIntroChain(state) : false;
                    if (!advanced)
                        NextStage(state, AnimKey::Battle_RunStart, BattleIntroStage::RunStart, ANIMTYPE::ONCE, 0.06f);
                }
                break;
            }
            // ---------------------------------------------------------------------------------------
            
            case BattleIntroStage::RunStart:
            {
                const _float3 cur = tfSys->GetPos(state.tfHandle);
                const float dx = targetWorld.x - cur.x, dz = targetWorld.z - cur.z;
                const float len = sqrtf(dx * dx + dz * dz);
                if (len > 1e-6f) faceSrv->PushSmoothXZ(owner, _float2{ dx / len, dz / len });

                if (IsCurClipFinished(state))
                {
                    PlayKey(state, AnimKey::Battle_RunLoop, ANIMTYPE::LOOP, 0.05f);
                    state.stage = BattleIntroStage::RunLoop;
                }
                break;
            }

            case BattleIntroStage::RunLoop:
            {
                const _float3 cur = tfSys->GetPos(state.tfHandle);
                const float dx = targetWorld.x - cur.x, dz = targetWorld.z - cur.z;
                const float distSq = dx * dx + dz * dz;
                const bool  arrived = (distSq < kArriveRadius * kArriveRadius);

                if (!arrived)
                {
                    const float len = sqrtf(distSq);
                    if (len > 1e-3f)
                    {
                        const float nx = dx / len, nz = dz / len;
                        _float3 next{ cur.x + nx * kIntroRunSpeed * dt, cur.y, cur.z + nz * kIntroRunSpeed * dt };
                        tfSys->SetPos(state.tfHandle, next);
                        faceSrv->PushSmoothXZ(owner, _float2{ nx, nz });
                    }
                }
                else
                {
                    if (!state.readyIdle)
                    {
                        state.readyIdle = true;

                        const auto [team, slot] = GetTeamSlot(owner);
                        if (team == BattleTeam::Ally)         
                            sessionSys->ReportIntroReady(owner);
                    }

                    PlayKey(state, AnimKey::Battle_RunEnd, ANIMTYPE::ONCE, kFadeShort);
                    state.stage = BattleIntroStage::RunEnd;
                }
                break;
            }

            case BattleIntroStage::RunEnd:
            {
                faceSrv->PushSnapXZ(owner, faceDirXZ);

                if (IsCurClipFinished(state))
                {
                    PlayKey(state, AnimKey::Battle_Idle, ANIMTYPE::LOOP, 0.06f);
                    state.stage = BattleIntroStage::BattleIdle;
                }
                break;
            }

            case BattleIntroStage::BattleIdle:
            {
                //if (!state.engagedToBattle && sessionSys->GetPhase() == BattlePhase::Active)
                //{
                //    auto [battleTeam, battleSlot] = GetTeamSlot(owner);
                //    _float3 battleTarget = formationSys->GetTargetPos(battleTeam, battleSlot);
                //    _float2 battleFace = formationSys->GetFaceDir(battleTeam, battleSlot);

                //    tfSys->SetPos(state.tfHandle, battleTarget);
                //    faceSrv->PushSnapXZ(owner, battleFace);

                //    state.engagedToBattle = true;
                //}
                //break;
            }

            default: break;
            }
        });
}

const wstring& BattleIntroSystem::ResolveClip(const AnimProfile& profile, AnimKey key) const
{
    return animDataSys->GetClipName(profile.character, profile.context, key);
}

bool BattleIntroSystem::IsCurClipFinished(const BattleIntroState& state) const
{
    const Handle handle = state.animHandle;

    if (animator->IsCrossFading(handle))
        return false;

    const _uint activeLayer = 0;

    if (!animator->IsPlaying(handle, activeLayer))
        return true;

    if (!state.curClipName.empty() && !animator->IsPlayingClip(handle, activeLayer, state.curClipName)) return false;

    const float remainSec = animator->GetRemainingTime(handle, activeLayer);
    return (remainSec <= 0.01f);
}

void BattleIntroSystem::PlayKey(BattleIntroState& state, AnimKey key, ANIMTYPE type, float fadeDur)
{
    const wstring& clip = ResolveClip(state.profile, key);
    state.curClipName   = clip;
    state.elapsed       = 0.f;

    const float fadeSeconds = max(0.f, fadeDur);
    if (fadeSeconds > 0.f)
        animator->CrossFade(state.animHandle, 0, 1, clip, fadeSeconds, type);
    else
        animator->Play(state.animHandle, 0, clip, type);
}

void BattleIntroSystem::NextStage(BattleIntroState& state, AnimKey nextKey, BattleIntroStage nextStage, ANIMTYPE type, float fadeDur)
{
    PlayKey(state, nextKey, type, fadeDur);
    state.stage = nextStage;
}

bool BattleIntroSystem::PlayNextIntroChain(BattleIntroState& state)
{
    const auto& stages = state.introChain->stages;
    ++state.chainStageIdx;
    if (state.chainStageIdx < static_cast<int>(stages.size()))
    {
        const AnimStageSpec& nextStage = stages[state.chainStageIdx];
        PlayKey(state, nextStage.clipKey, ANIMTYPE::ONCE, max(0.f, nextStage.fadeDur));
        return true;
    }

    NextStage(state, AnimKey::Battle_RunStart, BattleIntroStage::RunStart, ANIMTYPE::ONCE, 0.06f);
    return false;
}

pair<BattleTeam, int> BattleIntroSystem::GetTeamSlot(EntityID entity) const
{
    BattleTeam team = sessionSys->GetTeam(entity);
    int slot = sessionSys->GetSlotIdx(entity);
    return { team, slot };
}

_float3 BattleIntroSystem::GetFormationTarget(EntityID entity) const
{
    auto [team, slot] = GetTeamSlot(entity);
    return formationSys->GetTargetPos(team, slot);
}

_float2 BattleIntroSystem::GetFormationFace(EntityID entity) const
{
    auto [team, slot] = GetTeamSlot(entity);
    return formationSys->GetFaceDir(team, slot);
}

_float3 BattleIntroSystem::GetCenterWorld() const
{
    return formationSys->GetCenter();
}