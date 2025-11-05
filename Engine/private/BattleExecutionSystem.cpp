#include "Enginepch.h"
// -----------------------------------------------------------------------------------------------------------------
namespace
{
    constexpr float kMoveDur = 0.6f;
    constexpr float kDist    = 150.f;
}

static inline bool IsExecuteTag(SpecialAnimTag t)
{
    switch (t)
    {
    case SpecialAnimTag::BasicAttack:
    case SpecialAnimTag::SkillA:
    case SpecialAnimTag::SkillB:
    case SpecialAnimTag::SkillC:
    case SpecialAnimTag::SkillD:
    case SpecialAnimTag::ItemRush: return true;
    default: return false;
    }
}

static inline void QueuePulse(ExecutionUnitRunTime& rt, const _float2& dirXZ, float distanceMeters, float durationSec)
{
    if (distanceMeters <= 0.f || durationSec <= 0.f) return;
    rt.pulse.dirXZ      = dirXZ;
    rt.pulse.remainDist = distanceMeters;
    rt.pulse.speed      = distanceMeters / durationSec; 
    rt.pulse.active     = true;
}

static inline bool ContainsTag(const ActionAnimSpec& spec, SpecialAnimTag tag)
{
    return spec.specials.find(tag) != spec.specials.end();
}

static inline _float2 DirXZ(const _float3& from, const _float3& to)
{
    const float dx = to.x - from.x;
    const float dz = to.z - from.z;
    const float len = sqrtf(dx * dx + dz * dz);
    return (len > 1e-5f) ? _float2{ dx / len, dz / len } : _float2{ 0.f, 0.f };
}

static inline float DistXZ(const _float3& a, const _float3& b)
{
    const float dx = b.x - a.x;
    const float dz = b.z - a.z;
    return sqrtf(dx * dx + dz * dz);
}
static inline float Clampf(float v, float lo, float hi) { return max(lo, min(v, hi)); }
// ------------------------------------------------------------------------------------------------------------
void BattleExecutionSystem::OnBoot()
{
    actionReg   = &registry.Get<ActionAnimRegistry>();
    dataSys     = &registry.Get<CharacterDataSystem>();
    timelineSys = &registry.Get<BattleTimelineSystem>();
    animDataSys = &registry.Get<AnimDataSystem>();
    animator    = &registry.Get<AnimatorSystem>();
    tfSys       = &registry.Get<TransformSystem>();
    faceSrv     = &registry.Get<FacingForceService>();

    assert(actionReg && dataSys && timelineSys && animDataSys && animator && tfSys && faceSrv);
}

const AnimChainSpec* BattleExecutionSystem::TryGetChain(CharacterID ch, AnimContext cx, SpecialAnimTag tag) const
{
    const ActionAnimSpec* spec = actionReg->TryGet(ch);
    if (!spec) return nullptr;
    auto it = spec->specials.find(tag);
    return (it == spec->specials.end()) ? nullptr : &it->second;
}

bool BattleExecutionSystem::BeginAction(EntityID entity, const TimelineActionIntent& intent)
{
    if (entity == invalidEntity) return false;
  
    ExecutionUnitRunTime& rt = runtimeByEntity[entity];
    rt.character = dataSys->GetCharacterID(entity);
    rt.context = AnimContext::Battle;
    rt.activeIntent = intent;
    rt.cursor = {};
    rt.plannedTags.clear();
    rt.plannedIdx = -1;
    rt.phase = ExecutionUnitRunTime::Phase::None;

    switch (intent.battleCmd)
    {
    case BattleCommand::AttackBasic:
    case BattleCommand::Skill:
        BuildPlanForAttack(entity, rt, intent);
        break;
    case BattleCommand::Defend:
        BuildPlanForDefend(entity, rt);
        break;
    case BattleCommand::Escape:
        BuildPlanForEscape(entity, rt);
        break;
    default:
        timelineSys->NotifyActionFinished(entity, rt.activeIntent);
        runtimeByEntity.erase(entity);
        return false;
    }

    if (rt.plannedTags.empty())
    {
        timelineSys->NotifyActionFinished(entity, rt.activeIntent);
        runtimeByEntity.erase(entity);
        return false;
    }

    rt.plannedIdx = 0;
    rt.cursor = { 0, 0, true };
    bool played = false;

    if (const AnimChainSpec* first = TryGetChain(rt.character, rt.context, rt.plannedTags[0]))
        played = PlayStage(entity, rt, *first);

    if (!played)
    {
        if (!AdvanceChainOrFinish(entity, rt))
            return false;
    }

    return true;
}

void BattleExecutionSystem::Tick(float dt)
{
    if (runtimeByEntity.empty()) return;

    vector<EntityID> entitiesToRemove;
    entitiesToRemove.reserve(runtimeByEntity.size());

    for (auto& pair : runtimeByEntity)
    {
        const EntityID selfEntity = pair.first;
        ExecutionUnitRunTime& rt = pair.second;

        // 공격 준비/실행 시에는 타깃을 향해 지속적으로 정렬 + 접근/후퇴 유지
        if (rt.phase == ExecutionUnitRunTime::Phase::AttackStart || rt.phase == ExecutionUnitRunTime::Phase::Execute || rt.phase == ExecutionUnitRunTime::Phase::AttackFinished)
            MaintainFacing(selfEntity, rt);

        ConsumePulse(selfEntity, rt, dt);

        if (!rt.cursor.isActive)
        {
            entitiesToRemove.push_back(selfEntity);
            continue;
        }

        if (!AdvanceIfStageFinished(selfEntity, rt, dt)) continue;

        if (!rt.cursor.isActive)
            entitiesToRemove.push_back(selfEntity);
    }

    for (EntityID id : entitiesToRemove)
        runtimeByEntity.erase(id);
}

void BattleExecutionSystem::BuildPlanForAttack(EntityID entity, ExecutionUnitRunTime& rt, const TimelineActionIntent& intent)
{
    const ActionAnimSpec* spec = actionReg->TryGet(rt.character);

    if (!spec)
    {
        timelineSys->NotifyActionFinished(entity, rt.activeIntent);
        runtimeByEntity.erase(entity);
        return;
    }

    const bool hasStart  = ContainsTag(*spec, SpecialAnimTag::AttackStarted);
    const bool hasFinish = ContainsTag(*spec, SpecialAnimTag::AttackFinished);

    SpecialAnimTag       executeTag = intent.specialTag.has_value() ? intent.specialTag.value() : SpecialAnimTag::BasicAttack;
    const AnimChainSpec* execChain  = TryGetChain(rt.character, rt.context, executeTag);

    if (!execChain || execChain->stages.empty())
    {
        timelineSys->NotifyActionFinished(entity, rt.activeIntent);
        runtimeByEntity.erase(entity);
        return;
    }

    rt.plannedTags.clear();
    if (hasStart)  rt.plannedTags.push_back(SpecialAnimTag::AttackStarted);
    rt.plannedTags.push_back(executeTag);
    if (hasFinish) rt.plannedTags.push_back(SpecialAnimTag::AttackFinished);

    rt.phase = hasStart ? ExecutionUnitRunTime::Phase::AttackStart : ExecutionUnitRunTime::Phase::Execute;
}

void BattleExecutionSystem::BuildPlanForDefend(EntityID entity, ExecutionUnitRunTime& rt)
{
    const ActionAnimSpec* spec = actionReg->TryGet(rt.character);
    if (!spec) { timelineSys->NotifyActionFinished(entity, rt.activeIntent); runtimeByEntity.erase(entity); return; }

    const bool hasStart = ContainsTag(*spec, SpecialAnimTag::DefendStart);
    const bool hasHold  = ContainsTag(*spec, SpecialAnimTag::Defending);
    const bool hasEnd   = ContainsTag(*spec, SpecialAnimTag::DefendEnd);
    const bool hasTag   = rt.activeIntent.specialTag.has_value();
    
    const SpecialAnimTag tag = hasTag ? rt.activeIntent.specialTag.value() : SpecialAnimTag::DefendStart;
    rt.plannedTags.clear();

    if (!hasTag)
    {
        if (hasStart) rt.plannedTags.push_back(SpecialAnimTag::DefendStart);
        if (hasHold)  rt.plannedTags.push_back(SpecialAnimTag::Defending);
    }
    else
    {
        switch (tag)
        {
        case SpecialAnimTag::DefendStart:
            if (hasStart) rt.plannedTags.push_back(SpecialAnimTag::DefendStart);
            if (hasHold)  rt.plannedTags.push_back(SpecialAnimTag::Defending);
            break;
        case SpecialAnimTag::Defending:
            if (hasHold)  rt.plannedTags.push_back(SpecialAnimTag::Defending);
            break;
        case SpecialAnimTag::DefendEnd:
            if (hasEnd)   rt.plannedTags.push_back(SpecialAnimTag::DefendEnd);
            break;
        default:
            timelineSys->NotifyActionFinished(entity, rt.activeIntent);
            runtimeByEntity.erase(entity);
            return;
        }
    }

    if (rt.plannedTags.empty())
    {
        timelineSys->NotifyActionFinished(entity, rt.activeIntent);
        runtimeByEntity.erase(entity);
        return;
    }

    if (!rt.plannedTags.empty())
    {
        const auto first = rt.plannedTags.front();
        if      (first == SpecialAnimTag::DefendStart) rt.phase = ExecutionUnitRunTime::Phase::DefendStart;
        else if (first == SpecialAnimTag::Defending)   rt.phase = ExecutionUnitRunTime::Phase::Defending;
        else if (first == SpecialAnimTag::DefendEnd)   rt.phase = ExecutionUnitRunTime::Phase::DefendEnd;
        else                                           rt.phase = ExecutionUnitRunTime::Phase::Defending;
    }
}

void BattleExecutionSystem::BuildPlanForEscape(EntityID entity, ExecutionUnitRunTime& rt)
{
    timelineSys->NotifyActionFinished(entity, rt.activeIntent);
    runtimeByEntity.erase(entity);
}

bool BattleExecutionSystem::AdvanceIfStageFinished(EntityID entity, ExecutionUnitRunTime& rt, float dt)
{
    if (!rt.cursor.isActive) return false;
    if (rt.plannedIdx < 0 || rt.plannedIdx >= (int)rt.plannedTags.size()) return false;

    const SpecialAnimTag curTag = rt.plannedTags[(size_t)rt.plannedIdx];
    const AnimChainSpec* chain = TryGetChain(rt.character, rt.context, curTag);
    if (!chain || chain->stages.empty())
        return AdvanceChainOrFinish(entity, rt);

    if (!IsCurStageFinished(entity, rt, *chain))  return false;

    ++rt.cursor.curStageIdx;
    if (rt.cursor.curStageIdx >= (int)chain->stages.size())
        return AdvanceChainOrFinish(entity, rt);

    return PlayStage(entity, rt, *chain);
}

bool BattleExecutionSystem::AdvanceChainOrFinish(EntityID entity, ExecutionUnitRunTime& rt)
{
    ++rt.plannedIdx;
    rt.cursor.curStageIdx = 0;

    if (rt.plannedIdx >= (int)rt.plannedTags.size())
    {
        FinishAndIdle(entity, rt);
        return false;
    }

    const SpecialAnimTag nextTag = rt.plannedTags[(size_t)rt.plannedIdx];
    const AnimChainSpec* next    = TryGetChain(rt.character, rt.context, nextTag);

    switch (nextTag)
    {
    case SpecialAnimTag::AttackStarted:   rt.phase = ExecutionUnitRunTime::Phase::AttackStart;     break;
    case SpecialAnimTag::AttackFinished:  rt.phase = ExecutionUnitRunTime::Phase::AttackFinished;  break;
    case SpecialAnimTag::BasicAttack:
    case SpecialAnimTag::SkillA:
    case SpecialAnimTag::SkillB:
    case SpecialAnimTag::SkillC:
    case SpecialAnimTag::SkillD:
    case SpecialAnimTag::ItemRush:        rt.phase = ExecutionUnitRunTime::Phase::Execute;       break;
    default:                              rt.phase = ExecutionUnitRunTime::Phase::Defending;     break;
    }

    if (!next || next->stages.empty())
        return AdvanceChainOrFinish(entity, rt);

    rt.cursor.isActive = true;
    return PlayStage(entity, rt, *next);
}

bool BattleExecutionSystem::PlayStage(EntityID entity, ExecutionUnitRunTime& rt, const AnimChainSpec& chain)
{
    if (rt.cursor.curStageIdx < 0 || rt.cursor.curStageIdx >= (int)chain.stages.size())
    {
        FinishAndIdle(entity, rt);
        return false;
    }

    SpecialAnimTag curTag = SpecialAnimTag::BasicAttack;
    if (rt.plannedIdx >= 0 && rt.plannedIdx < (int)rt.plannedTags.size())
        curTag = rt.plannedTags[(size_t)rt.plannedIdx];

    const AnimStageSpec& stageSpec = chain.stages[(size_t)rt.cursor.curStageIdx];
    const wstring& clipName = animDataSys->GetClipName(rt.character, rt.context, stageSpec.clipKey);
    if (clipName.empty())
    {
        FinishAndIdle(entity, rt);
        return false;
    }

    ClipTuning tuning = animDataSys->GetClipTuning(rt.character, rt.context, stageSpec.clipKey);
    if (stageSpec.startNormalizedOverride) tuning.startNormalized = *stageSpec.startNormalizedOverride;
    if (stageSpec.endNormalizedOverride)   tuning.endNormalized = *stageSpec.endNormalizedOverride;

    Handle animHandle = ResolveAnimHandle(entity);
    if (!animHandle.IsValid())
    {
        rt.cursor.isActive = false;
        registry.Get<BattleTimelineSystem>().NotifyActionFinished(entity, rt.activeIntent);
        return false;
    }

    const float fadeSeconds = max(0.f, stageSpec.fadeDur);
    animator->SetPlaybackSpeed(animHandle, 0, tuning.playbackSpeed);

    const bool loopThisStage = (curTag == SpecialAnimTag::Defending);

    if (fadeSeconds > 0.f)
        animator->CrossFade(animHandle, 0, 1, clipName, fadeSeconds, loopThisStage ? ANIMTYPE::LOOP : ANIMTYPE::ONCE, tuning.startNormalized, tuning.endNormalized);
    else
        animator->PlaySection(animHandle, 0, clipName, loopThisStage ? ANIMTYPE::LOOP : ANIMTYPE::ONCE, tuning.startNormalized, tuning.endNormalized);
    
// ---------------------------------------------------------------------------------------------------------------------------------------------------------
    if (rt.activeIntent.targetEntity != invalidEntity && rt.cursor.curStageIdx == 0)
    {
        Handle selfTf = ResolveTfHandle(entity);
        Handle tgtTf = ResolveTfHandle(rt.activeIntent.targetEntity);
        if (selfTf.IsValid() && tgtTf.IsValid())
        {
            const _float3 a = tfSys->GetPos(selfTf);
            const _float3 b = tfSys->GetPos(tgtTf);
            const float dx = b.x - a.x, dz = b.z - a.z;
            const float len = sqrtf(dx * dx + dz * dz);
            if (len > 0.01f)
            {
                const _float2 dir{ dx / len, dz / len };
                const SpecialAnimTag curTag = rt.plannedTags[(size_t)rt.plannedIdx];

                auto queue_fixed = [&](const _float2& d) { QueuePulse(rt, d, kDist, kMoveDur); };

                if (curTag == SpecialAnimTag::AttackStarted)
                    queue_fixed(dir);
                else if (curTag == SpecialAnimTag::AttackFinished)
                    queue_fixed(_float2{ -dir.x, -dir.y });
            }
        }
    }
    return true;
}

bool BattleExecutionSystem::IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& rt, const AnimChainSpec& chain) const
{
    if (!rt.cursor.isActive) return true;
    if (rt.cursor.curStageIdx < 0 || rt.cursor.curStageIdx >= (int)chain.stages.size()) return true;

    SpecialAnimTag curTag = SpecialAnimTag::BasicAttack;
    if (rt.plannedIdx >= 0 && rt.plannedIdx < (int)rt.plannedTags.size())
        curTag = rt.plannedTags[(size_t)rt.plannedIdx];
    if (curTag == SpecialAnimTag::Defending)
        return false;

    const AnimStageSpec& stageSpec = chain.stages[(size_t)rt.cursor.curStageIdx];
    const wstring& clipName = animDataSys->GetClipName(rt.character, rt.context, stageSpec.clipKey);
    if (clipName.empty()) return true;

    Handle animHand = ResolveAnimHandle(entity);
    if (!animHand.IsValid()) return true;
    if (!animator->IsPlaying(animHand, 0)) return true;
    if (animator->IsCrossFading(animHand)) return false;

    const float remainingSeconds = animator->GetRemainingTime(animHand, 0);
    const float transitionWindow = max(stageSpec.fadeDur, stageSpec.minOverlapDur);
    return (remainingSeconds <= transitionWindow);
}

Handle BattleExecutionSystem::ResolveAnimHandle(EntityID entity) const
{
    Handle handle{};
    animator->GetByOwner(entity, &handle);
    return handle;
}

Handle BattleExecutionSystem::ResolveTfHandle(EntityID entity) const
{
    Handle tfHandle{};
    if (tfSys->GetByOwner(entity, &tfHandle)) return tfHandle;
    return {};
}

void BattleExecutionSystem::ConsumePulse(EntityID entity, ExecutionUnitRunTime& rt, float dt)
{
    if (!rt.pulse.active || rt.pulse.remainDist <= 0.f) return;

    const float step = min(rt.pulse.speed * dt, rt.pulse.remainDist);
    if (step <= 0.f) return;

    Handle tf    = ResolveTfHandle(entity);
    if (tf.IsValid())
    {
        const _float3 delta = _float3{ rt.pulse.dirXZ.x * step, 0.f, rt.pulse.dirXZ.y * step };
        tfSys->AddWorldOffset(tf, delta);
    }

    rt.pulse.remainDist -= step;
    if (rt.pulse.remainDist <= 0.f)
        rt.pulse.active = false;
}

void BattleExecutionSystem::MaintainFacing(EntityID self, ExecutionUnitRunTime& rt)
{
    if (rt.activeIntent.targetEntity == invalidEntity) return;

    Handle selfTf = ResolveTfHandle(self);
    Handle tgtTf = ResolveTfHandle(rt.activeIntent.targetEntity);
    if (!selfTf.IsValid() || !tgtTf.IsValid()) return;

    const _float3 a   = tfSys->GetPos(selfTf);
    const _float3 b   = tfSys->GetPos(tgtTf);
    const float   dx  = b.x - a.x, dz = b.z - a.z;
    const float   len = sqrtf(dx * dx + dz * dz);
    if (len <= 1e-5f) return;

    faceSrv->PushSmoothXZ(self, _float2{ dx / len, dz / len });
}

void BattleExecutionSystem::FinishAndIdle(EntityID entity, ExecutionUnitRunTime& rt)
{
    Handle animHandle = ResolveAnimHandle(entity);

    if (animHandle.IsValid())
    {
        const wstring& idle = animDataSys->GetClipName(rt.character, rt.context, AnimKey::Battle_Idle);
        if (!idle.empty())
            animator->CrossFade(animHandle, 0, 1, idle, 0.08f, ANIMTYPE::LOOP);
    }

    rt.pulse.active     = false;
    rt.pulse.remainDist = 0.f;
    rt.cursor.isActive  = false;

    timelineSys->NotifyActionFinished(entity, rt.activeIntent);
}
