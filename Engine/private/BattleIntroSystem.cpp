#include "Enginepch.h"

Handle BattleIntroSystem::Create(EntityID owner, Handle animHandle, Handle tfHandle, const AnimProfile& profile)
{
    Handle handle      = CreateComp(owner);
    auto& state        = *Get(handle);
    state.entity       = owner;
    state.animHandle   = animHandle;
    state.tfHandle     = tfHandle;
    state.profile      = profile;
    state.stage        = BattleIntroStage::IntroStart;
   
    const auto& dataSys = registry.Get<CharacterDataSystem>();
    const auto* actionSpec = dataSys.GetActionSpec(owner);
    if (actionSpec && actionSpec->introChain.has_value() && !actionSpec->introChain->stages.empty())
    {
        state.introStageIdx = 0;
        const AnimStageSpec& first = actionSpec->introChain->stages[0];
        PlayKey(state, first.clipKey, ANIMTYPE::ONCE, first.fadeDur);
    }
    return handle;
}

void BattleIntroSystem::Update(float dt)
{
    auto& animSys  = registry.Get<AnimatorSystem>();
    auto& tfSys    = registry.Get<TransformSystem>();
    auto& faceSrv  = registry.Get<FacingForceService>();
    auto* session  = registry.TryGet<BattleSessionSystem>();

    ForEachAliveEx([&](Handle handle, EntityID owner, BattleIntroState& state)
        {
            state.elasped += dt;

            if (!session || !session->HasActiveSession())  return;

            _float3 targetWorld{};
            const bool hasTarget = session->TryGetIntroTargetPos(owner, targetWorld);

            _float2 faceDirXZ{};
            const bool hasFace = session->TryGetIntroFaceXZ(owner, faceDirXZ);

            switch (state.stage)
            {
            case BattleIntroStage::IntroStart:
            {
                if (hasFace) faceSrv.PushSnapXZ(owner, _float2{0.f, -1.f});

                const auto& dataSys    = registry.Get<CharacterDataSystem>();
                const auto& actionSpec = dataSys.GetActionSpec(owner);

                if (IsFinished(state))
                {
                    bool hasAdvanced = false;
                    if (actionSpec && actionSpec->introChain.has_value())
                    {
                        const auto& stages = actionSpec->introChain->stages;
                        if (state.introStageIdx >= 0 && state.introStageIdx + 1 < static_cast<int>(stages.size()))
                        {
                            state.introStageIdx++;
                            const AnimStageSpec& nextStage = stages[state.introStageIdx];
                            PlayKey(state, nextStage.clipKey, ANIMTYPE::ONCE, nextStage.fadeDur);
                            hasAdvanced = true;
                        }
                    }

                    if (!hasAdvanced)
                        NextStage(state, AnimKey::Battle_RunStart, BattleIntroStage::RunStart, ANIMTYPE::ONCE, 0.06f);
                }
                break;
            }
            // ---------------------------------------------------------------------------------------
            
            case BattleIntroStage::RunStart:
            {
                if (hasTarget)
                {
                    const _float3 curPos = tfSys.GetPos(state.tfHandle);
                    const float dx = targetWorld.x - curPos.x;
                    const float dz = targetWorld.z - curPos.z;
                    const float len = sqrtf(dx * dx + dz * dz);
                    if (len > 1e-6f)
                        faceSrv.PushSmoothXZ(owner, _float2{ dx / len, dz / len });
                }

                if (IsFinished(state))
                {
                    PlayKey(state, AnimKey::Battle_RunLoop, ANIMTYPE::LOOP, 0.05f);
                    state.stage = BattleIntroStage::RunLoop;
                }
                break;
            }

            case BattleIntroStage::RunLoop:
            {
                if (hasTarget)
                {
                    const _float3 curPos = tfSys.GetPos(state.tfHandle);
                    const float dx = targetWorld.x - curPos.x;
                    const float dz = targetWorld.z - curPos.z;
                    const float distSq = dx * dx + dz * dz;
                    const bool arrived = (distSq < 10.f * 10.f);

                    if (!arrived)
                    {
                        const float len = sqrtf(distSq);
                        if (len > 1e-3f)
                        {
                            const float nx = dx / len;
                            const float nz = dz / len;
                            const float moveSpeed = 500.f; 
                            _float3 newPos{ curPos.x + nx * moveSpeed * dt, curPos.y, curPos.z + nz * moveSpeed * dt };
                            tfSys.SetPos(state.tfHandle, newPos);
                            faceSrv.PushSmoothXZ(owner, _float2{ nx, nz });
                        }
                    }
                    else
                    {
                        if (!state.readyIdle)
                        {
                            state.readyIdle = true;
                            session->ReportIntroReady(owner);
                        }
                        PlayKey(state, AnimKey::Battle_RunEnd, ANIMTYPE::ONCE, 0.06f);
                        state.stage = BattleIntroStage::RunEnd;
                    }
                }
                break;
            }

            case BattleIntroStage::RunEnd:
            {
                if (session->TryGetIntroFaceXZ(owner, faceDirXZ))
                    faceSrv.PushSnapXZ(owner, faceDirXZ);   
                else 
                {
                    const _float3 curPos = tfSys.GetPos(state.tfHandle);
                    const _float3 center = session->GetCenter();
                    const float dx = center.x - curPos.x;
                    const float dz = center.z - curPos.z;
                    const float len = sqrtf(dx * dx + dz * dz);
                    if (len > 1e-6f) faceSrv.PushSnapXZ(owner, _float2{ dx / len, dz / len }); 
                }
                if (IsFinished(state))
                {
                    PlayKey(state, AnimKey::Battle_Idle, ANIMTYPE::LOOP, 0.06f);
                    state.stage = BattleIntroStage::BattleIdle;
                }
                break;
            }

            default: break;
            }
        });
}

const wstring& BattleIntroSystem::ResolveClip(const AnimProfile& profile, AnimKey key) const
{
    return registry.Get<AnimDataSystem>().GetClipName(profile.character, profile.context, key);
}

bool BattleIntroSystem::IsFinished(const BattleIntroState& state) const
{
    auto& animSys = registry.Get<AnimatorSystem>();
    float dur     = animSys.GetClipDuration(state.animHandle, state.curClipName);
    return (dur > 0.f && state.elasped >= dur);
}

void BattleIntroSystem::PlayKey(BattleIntroState& state, AnimKey key, ANIMTYPE type, float fadeDur)
{
    auto& animSys       = registry.Get<AnimatorSystem>();
    const wstring& clip = ResolveClip(state.profile, key);
    state.curClipName   = clip;
    state.elasped       = 0.f;
    if (fadeDur > 0.f) animSys.PlayFade(state.animHandle, 0, clip, fadeDur, 1.f, type);
    else               animSys.Play(state.animHandle, 0, clip, type);
}

void BattleIntroSystem::NextStage(BattleIntroState& state, AnimKey nextKey, BattleIntroStage nextStage, ANIMTYPE type, float fadeDur)
{
    PlayKey(state, nextKey, type, fadeDur);
    state.stage = nextStage;
}

bool BattleIntroSystem::TryQueryTargetPos(EntityID entity, _float3& outTargetWorld) const
{
    auto* session = registry.TryGet<BattleSessionSystem>();
    if (!session || !session->HasActiveSession())
        return false;

    return session->TryGetIntroTargetPos(entity, outTargetWorld);
}

_float3 BattleIntroSystem::QueryCenterWorldOrDefault() const
{
    auto* session = registry.TryGet<BattleSessionSystem>();
    if (session && session->HasActiveSession() && session->TryGetState())
        return session->TryGetState()->layout.centerWorld;

    return _float3{ 0.f, 0.f, 0.f };
}