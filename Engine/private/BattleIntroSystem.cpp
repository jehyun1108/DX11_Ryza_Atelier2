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
   
    const auto& actionReg = registry.Get<ActionAnimRegistry>();
    state.introChain = actionReg.TryGetSpecial(profile.character, SpecialAnimTag::Intro);
    state.chainStageIdx = (state.introChain && !state.introChain->stages.empty()) ? 0 : -1;

    if (state.chainStageIdx >= 0)
    {
        const AnimStageSpec& first = state.introChain->stages[0];
        PlayKey(state, first.clipKey, ANIMTYPE::ONCE, max(0.f, first.fadeDur));
    }
    else
    {
        PlayKey(state, AnimKey::Battle_RunStart, ANIMTYPE::ONCE, 0.06f);
        state.stage = BattleIntroStage::RunStart;
    }
    return handle;
}

void BattleIntroSystem::Update(float dt)
{
    auto& animSys   = registry.Get<AnimatorSystem>();
    auto& tfSys     = registry.Get<TransformSystem>();
    auto& faceSrv   = registry.Get<FacingForceService>();
    auto* session   = registry.TryGet<BattleSessionSystem>();
    auto* formation = registry.TryGet<BattleFormationSystem>();

    if (!session || !session->HasActiveSession() || !formation) return;

    ForEachAliveEx([&](Handle handle, EntityID owner, BattleIntroState& state)
        {
            state.elapsed += dt;
            
            _float3 targetWorld{};
            _float2 faceDirXZ{};
            const bool hasTarget = TryQueryFormationTarget(owner, targetWorld);
            const bool hasFace   = TryQueryFormationFace(owner, faceDirXZ);

            switch (state.stage)
            {
            case BattleIntroStage::IntroStart:
            {
                if (hasFace) 
                    faceSrv.PushSnapXZ(owner, _float2{0.f, -1.f});
                else
                {
                    const _float3 cur = tfSys.GetPos(state.tfHandle);
                    const _float3 cen = QueryCenterWorld();
                    const float   dx  = cen.x - cur.x, dz = cen.z - cur.z;
                    const float   len = sqrtf(dx * dx + dz * dz);
                    if (len > 1e-6f) 
                        faceSrv.PushSnapXZ(owner, _float2{ dx / len, dz / len });
                }

                if (IsCurClipFinished(state))
                {
                    const bool advanced = TryPlayNextIntroChain(state);
                    if (!advanced)
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

                if (IsCurClipFinished(state))
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
                if (hasFace) 
                    faceSrv.PushSnapXZ(owner, faceDirXZ);
                else
                {
                    const _float3 curPos = tfSys.GetPos(state.tfHandle);
                    const _float3 center = QueryCenterWorld();
                    const float   dx     = center.x - curPos.x, dz = center.z - curPos.z;
                    const float   len    = sqrtf(dx * dx + dz * dz);
                    if (len > 1e-6f) faceSrv.PushSnapXZ(owner, _float2{ dx / len, dz / len });
                }

                if (IsCurClipFinished(state))
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

bool BattleIntroSystem::IsCurClipFinished(const BattleIntroState& state) const
{
    auto& animSys = registry.Get<AnimatorSystem>();
    const Handle handle = state.animHandle;

    if (animSys.IsCrossFading(handle))
        return false;

    const _uint activeLayer = 0;

    if (!animSys.IsPlaying(handle, activeLayer))
        return true;

    if (!state.curClipName.empty() && !animSys.IsPlayingClip(handle, activeLayer, state.curClipName)) return false;

    const float remainSec = animSys.GetRemainingTime(handle, activeLayer);
    return (remainSec <= 0.01f);
}

void BattleIntroSystem::PlayKey(BattleIntroState& state, AnimKey key, ANIMTYPE type, float fadeDur)
{
    auto& animSys       = registry.Get<AnimatorSystem>();
    const wstring& clip = ResolveClip(state.profile, key);
    state.curClipName   = clip;
    state.elapsed       = 0.f;

    const float fadeSeconds = max(0.f, fadeDur);
    if (fadeSeconds > 0.f)
        animSys.CrossFade(state.animHandle, 0, 1, clip, fadeSeconds, type);
    else
        animSys.Play(state.animHandle, 0, clip, type);
}

void BattleIntroSystem::NextStage(BattleIntroState& state, AnimKey nextKey, BattleIntroStage nextStage, ANIMTYPE type, float fadeDur)
{
    PlayKey(state, nextKey, type, fadeDur);
    state.stage = nextStage;
}

bool BattleIntroSystem::TryPlayNextIntroChain(BattleIntroState& state)
{
    if (!state.introChain) return false;

    const auto& stages = state.introChain->stages;
    if (state.chainStageIdx < 0 || state.chainStageIdx >= static_cast<int>(stages.size())) return false;

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

bool BattleIntroSystem::ResolveTeamSlot(EntityID entity, BattleTeam& outTeam, int& outSlot) const
{
    auto* session = registry.TryGet<BattleSessionSystem>();
    if (!session || !session->HasActiveSession()) return false;
    if (!session->TryGetTeam(entity, outTeam))    return false;
    if (!session->TryGetSlotIdx(entity, outSlot)) return false;
    return true;
}

bool BattleIntroSystem::TryQueryFormationTarget(EntityID entity, _float3& outPos) const
{
    BattleTeam team; int slot;
    if (!ResolveTeamSlot(entity, team, slot)) return false;
    auto* formation = registry.TryGet<BattleFormationSystem>();
    if (!formation) return false;
    return formation->TryGetTargetPos(team, slot, outPos);
}

bool BattleIntroSystem::TryQueryFormationFace(EntityID entity, _float2& outDirXZ) const
{
    BattleTeam team; int slot;
    if (!ResolveTeamSlot(entity, team, slot)) return false;
    auto* formation = registry.TryGet<BattleFormationSystem>();
    if (!formation) return false;
    return formation->TryGetFaceDir(team, slot, outDirXZ);
}

_float3 BattleIntroSystem::QueryCenterWorld() const
{
    auto* formation = registry.TryGet<BattleFormationSystem>();
    if (formation) return formation->GetCenter();
    return _float3{ 0.f, 0.f, 0.f };
}