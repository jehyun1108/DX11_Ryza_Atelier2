#include "Enginepch.h"
#include "BattleAttributeSystem.h"
#include "ActionFxRegistry.h"
#include "SoundSystem.h"
#include "BattleDamagePresenter.h"
// -----------------------------------------------------------------------------------------------------------------
namespace
{
    constexpr float kMoveDur = 0.6f;
    constexpr float kDist    = 200.f;
}
static inline void QueuePulse(ExecutionUnitRunTime& rt, const _float2& dirXZ, float distanceMeters, float durationSec)
{
    if (distanceMeters <= 0.f || durationSec <= 0.f) return;
    rt.pulse.dirXZ = dirXZ;
    rt.pulse.remainDist = distanceMeters;
    rt.pulse.speed = distanceMeters / durationSec;
    rt.pulse.active = true;
}
static inline int ComputeDamage(const ExecutionUnitRunTime& rt, float hitRatio)
{
    const float v = (float)rt.hit.baseDmg * rt.hit.tagMul * hitRatio;
    return (int)roundf(max(0.f, v));
}
static inline float RandomStunOnExecute()
{
    const int low = 5, high = 10;
    const int r = low + (rand() % (high - low + 1));
    return (float)r;
}
static inline bool ContainsTag(const ActionAnimSpec& spec, SpecialAnimTag tag)
{
    return spec.specials.find(tag) != spec.specials.end();
}
bool BattleExecutionSystem::IsComboStepTag(SpecialAnimTag t) const
{
    switch (t)
    {
    case SpecialAnimTag::SkillA_1:
    case SpecialAnimTag::SkillA_2:
    case SpecialAnimTag::SkillA_3:
    case SpecialAnimTag::SkillB_1:
    case SpecialAnimTag::SkillB_2:
    case SpecialAnimTag::SkillB_3:
    case SpecialAnimTag::SkillC_1:
    case SpecialAnimTag::SkillC_2:
    case SpecialAnimTag::SkillC_3:
    case SpecialAnimTag::SkillD_1:
    case SpecialAnimTag::SkillD_2:
    case SpecialAnimTag::SkillD_3:
        return true;
    default:
        return false;
    }
}
bool BattleExecutionSystem::CanQueueCombo(EntityID entity) const
{
    auto it = runtimeByEntity.find(entity);
    if (it == runtimeByEntity.end())
        return false;

    const ExecutionUnitRunTime& rt = it->second;

    if (!IsComboStepTag(rt.execTag))
        return false;

    if (rt.curTag != rt.execTag)
        return false;

    if (rt.hasQueuedCombo)
        return false;

    Handle animHandle = animator->Get(entity);
    float curNorm = animator->GetNormalizedTime(animHandle, 0);

    return (curNorm < rt.comboEndNorm);
}
bool BattleExecutionSystem::IsActing(EntityID entity) const
{
    auto it = runtimeByEntity.find(entity);
    if (it == runtimeByEntity.end()) return false;

    const ExecutionUnitRunTime& rt = it->second;
    return rt.cursor.isActive;
}
void BattleExecutionSystem::OnUnitRemoved(EntityID entity)
{
    runtimeByEntity.erase(entity);

    for (auto& pair : runtimeByEntity)
    {
        ExecutionUnitRunTime& rt = pair.second;
        if (rt.activeIntent.targetEntity == entity)
            rt.activeIntent.targetEntity = 0u;
    }
}
// ------------------------------------------------------------------------------------------------------------
void BattleExecutionSystem::OnBoot()
{
    actionReg    = &registry.Get<ActionAnimRegistry>();
    dataSys      = &registry.Get<CharacterDataSystem>();
    timelineSys  = &registry.Get<BattleTimelineSystem>();
    animDataSys  = &registry.Get<AnimDataSystem>();
    animator     = &registry.Get<AnimatorSystem>();
    tfSys        = &registry.Get<TransformSystem>();
    faceSrv      = &registry.Get<FacingForceService>();
    attrSys      = &registry.Get<BattleAttributeSystem>();
    eventBus     = &registry.Get<BattleEventBus>();
    actionCamReg = &registry.Get<ActionCamRegistry>();
    targetSys    = &registry.Get<BattleTargetSystem>();
    ctrlSys      = &registry.Get<BattleControllerSystem>();
    camReg       = &registry.Get<CamRegistry>();
    effectSys    = &registry.Get<EffectSystem>();
    fxReg        = &registry.Get<ActionFxRegistry>();
    sessionSys   = &registry.Get<BattleSessionSystem>();
    soundSys     = &registry.Get<SoundSystem>();
    dmgPresenter = &registry.Get<BattleDamagePresenter>();
    tacticSys    = &registry.Get<BattleTacticSystem>();
}

bool BattleExecutionSystem::BeginAction(EntityID entity, const TimelineActionIntent& intent, bool useWrapper)
{
    if (sessionSys->GetPhase() != BattlePhase::Active)   return false;

    ExecutionUnitRunTime& rt = runtimeByEntity[entity];
    rt.character             = dataSys->GetCharacterID(entity);
    rt.context               = AnimContext::Battle;
    rt.activeIntent          = intent;
    rt.cursor                = { 0, 0, true };
    rt.plannedIdx            = -1;
    rt.phase                 = ExecutionUnitRunTime::Phase::None;
    rt.hit                   = HitCursor{};
    rt.plannedTags.clear();
    rt.execTag               = SpecialAnimTag::BasicAttack;
    rt.curTag                = SpecialAnimTag::BasicAttack;
    rt.queuedTag             = SpecialAnimTag::BasicAttack;
    rt.comboEndNorm          = 1.f;
    rt.comboInputOpen        = false;
    rt.hasQueuedCombo        = false;

    switch (intent.battleCmd)
    {
    case BattleCommand::AttackBasic:
    case BattleCommand::Skill:
    {
        const EntityID leader = timelineSys->GetLeader();
        if (entity == leader && intent.battleCmd == BattleCommand::Skill)
            tacticSys->GainPip(1);   

        BuildPlanForAttack(entity, rt, intent, useWrapper);

        const SpecialAnimTag execTag = intent.specialTag.value_or(SpecialAnimTag::BasicAttack);
        rt.execTag = execTag;

        GetChain(rt.character, rt.context, execTag);

        if (IsComboStepTag(execTag))
        {
            const SkillStepInfo& stepInfo = actionReg->GetStepInfo(rt.character, execTag);
            rt.comboEndNorm = stepInfo.inputEndNorm;
            rt.comboChainCutNorm = stepInfo.chainCutNorm;
        }
        else
        {
            rt.comboEndNorm = 1.f;
            rt.comboChainCutNorm = 1.f;
        }

        const ActionAnimSpec& spec = actionReg->Get(rt.character);
        float tagMul = 1.f;
        auto itMul = spec.dmgMulByTag.find(execTag);
        if (itMul != spec.dmgMulByTag.end())
            tagMul = itMul->second;
        rt.hit.tagMul = ResolveTagDmgMul(rt.character, execTag);

        const CharacterSpec& chSpec = dataSys->GetSpec(rt.character);
        const int base = (chSpec.team == BattleTeam::Ally) ? chSpec.party.attackDmg : chSpec.enemy.attackDmg;
        rt.hit.baseDmg = base;

        EntityID victim = intent.targetEntity;
        if (victim == 0u)
            victim = targetSys->Get(entity);

        rt.activeIntent.targetEntity = victim;

        if (victim != 0u && entity == timelineSys->GetLeader())
        {
            TrackID camId = actionCamReg->PlaySkillCam(rt.character, execTag, entity, victim);

            if (!camId.IsValid())
                camId = actionCamReg->PlayActionCam(intent.battleCmd, entity, victim);
        }

        break;
    }

    case BattleCommand::Defend:
        BuildPlanForDefend(entity, rt);
        break;

    case BattleCommand::Escape:
        BuildPlanForEscape(entity, rt);
        return false;
    }

    if (rt.plannedTags.empty())
    {
        rt.cursor.isActive = false;
        timelineSys->NotifyActionFinished(entity, rt.activeIntent);
        runtimeByEntity.erase(entity);
        return false;
    }

    rt.plannedIdx = 0;
    const SpecialAnimTag firstTag = rt.plannedTags.front();
    rt.curTag = firstTag;

    const AnimChainSpec& first = GetChain(rt.character, rt.context, firstTag);

    if (IsComboStepTag(rt.execTag) && firstTag == rt.execTag)
        rt.comboInputOpen = true;
    else
        rt.comboInputOpen = false;

    const bool played = PlayStage(entity, rt, first);

    if (!played)
        return AdvanceChainOrFinish(entity, rt);

    SetUpSfxForCurTag(entity, rt);

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

        if (rt.phase == ExecutionUnitRunTime::Phase::AttackStart || rt.phase == ExecutionUnitRunTime::Phase::Execute || rt.phase == ExecutionUnitRunTime::Phase::AttackFinished)
            MaintainFacing(selfEntity, rt);

        ConsumePulse(selfEntity, rt, dt);
        UpdateSfxSequence(rt);

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

void BattleExecutionSystem::BuildPlanForAttack(EntityID entity, ExecutionUnitRunTime& rt, const TimelineActionIntent& intent, bool useWrapper)
{
    const ActionAnimSpec& spec = actionReg->Get(rt.character);

    const bool hasStart = ContainsTag(spec, SpecialAnimTag::AttackStarted);
    const bool hasFinish = ContainsTag(spec, SpecialAnimTag::AttackFinished);

    const SpecialAnimTag execTag = intent.specialTag.value_or(SpecialAnimTag::BasicAttack);
    GetChain(rt.character, rt.context, execTag);

    rt.plannedTags.clear();

    if (useWrapper && hasStart)
        rt.plannedTags.push_back(SpecialAnimTag::AttackStarted);

    rt.plannedTags.push_back(execTag);

    if (useWrapper && hasFinish)
        rt.plannedTags.push_back(SpecialAnimTag::AttackFinished);

    if (useWrapper && hasStart)
        rt.phase = ExecutionUnitRunTime::Phase::AttackStart;
    else
        rt.phase = ExecutionUnitRunTime::Phase::Execute;
}

void BattleExecutionSystem::BuildPlanForDefend(EntityID entity, ExecutionUnitRunTime& rt)
{
    const ActionAnimSpec& spec = actionReg->Get(rt.character);

    const bool hasStart = ContainsTag(spec, SpecialAnimTag::DefendStart);
    const bool hasHold  = ContainsTag(spec, SpecialAnimTag::Defending);
    const bool hasEnd   = ContainsTag(spec, SpecialAnimTag::DefendEnd);

    rt.plannedTags.clear();

    if (!rt.activeIntent.specialTag.has_value())
    {
        if (hasStart) rt.plannedTags.push_back(SpecialAnimTag::DefendStart);
        if (hasHold)  rt.plannedTags.push_back(SpecialAnimTag::Defending);
    }
    else
    {
        switch (rt.activeIntent.specialTag.value())
        {
        case SpecialAnimTag::DefendStart:
            rt.plannedTags.push_back(SpecialAnimTag::DefendStart);
            if (hasHold) rt.plannedTags.push_back(SpecialAnimTag::Defending);
            break;
        case SpecialAnimTag::Defending:
            rt.plannedTags.push_back(SpecialAnimTag::Defending);
            break;
        case SpecialAnimTag::DefendEnd:
            rt.plannedTags.push_back(SpecialAnimTag::DefendEnd);
            break;
        }
    }

    const SpecialAnimTag first = rt.plannedTags.front();
    rt.phase = (first == SpecialAnimTag::DefendStart) ? ExecutionUnitRunTime::Phase::DefendStart
             : (first == SpecialAnimTag::Defending)   ? ExecutionUnitRunTime::Phase::Defending
                                                      : ExecutionUnitRunTime::Phase::DefendEnd;
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
    const AnimChainSpec& chain = GetChain(rt.character, rt.context, curTag);
    if (chain.stages.empty()) return AdvanceChainOrFinish(entity, rt);

    float curNorm = 0.f;
    {
        Handle animHandle = animator->Get(entity);
        curNorm = animator->GetNormalizedTime(animHandle, 0);

        const AnimStageSpec& stage = chain.stages[(size_t)rt.cursor.curStageIdx];
        const auto& hits = stage.hits;

        if (rt.comboInputOpen && curTag == rt.execTag && curNorm >= rt.comboEndNorm)
            rt.comboInputOpen = false;

        while (rt.hit.stage == rt.cursor.curStageIdx && rt.hit.next < (int)hits.size())
        {
            const HitPoint& hp = hits[(size_t)rt.hit.next];
            if (hp.timeNorm <= curNorm)
            {
                EmitHit(entity, rt, rt.activeIntent.targetEntity, hp.dmgRatio, false);
                if (!hp.sfxKey.empty())
                    soundSys->Play(hp.sfxKey);

                ++rt.hit.next;
            }
            else
                break;
        }
        rt.hit.lastNorm = curNorm;
    }

    if (curTag == rt.execTag && rt.hasQueuedCombo && curNorm >= rt.comboChainCutNorm)
        return AdvanceChainOrFinish(entity, rt);

    if (!IsCurStageFinished(entity, rt, chain)) return false;

    ++rt.cursor.curStageIdx;
    if (rt.cursor.curStageIdx >= (int)chain.stages.size())
        return AdvanceChainOrFinish(entity, rt);

    return PlayStage(entity, rt, chain);
}

bool BattleExecutionSystem::AdvanceChainOrFinish(EntityID entity, ExecutionUnitRunTime& rt)
{
    if (rt.hasQueuedCombo && IsComboStepTag(rt.execTag) && rt.curTag == rt.execTag)
    {
        const SkillStepInfo& curInfo = actionReg->GetStepInfo(rt.character, rt.execTag);
        const SkillStepInfo& queuedInfo = actionReg->GetStepInfo(rt.character, rt.queuedTag);

        SkillSlotTag nextSlot{};
        int          nextStepIdx = -1;

        if (queuedInfo.slot == curInfo.slot)
        {
            if (curInfo.stepIdx < 2)
            {
                nextSlot = curInfo.slot;
                nextStepIdx = curInfo.stepIdx + 1;
            }
            else
                rt.hasQueuedCombo = false;
        }
        else
        {
            nextSlot = queuedInfo.slot;
            nextStepIdx = 0;
        }

        if (rt.hasQueuedCombo && nextStepIdx >= 0)
        {
            SpecialAnimTag nextExecTag = actionReg->GetStepTag(rt.character, nextSlot, nextStepIdx);
            const SkillStepInfo& nextInfo = actionReg->GetStepInfo(rt.character, nextExecTag);

            if (!rt.plannedTags.empty() && rt.plannedTags.back() == SpecialAnimTag::AttackFinished)
                rt.plannedTags.insert(rt.plannedTags.end() - 1, nextExecTag);
            else
                rt.plannedTags.push_back(nextExecTag);

            rt.execTag = nextExecTag;
            rt.comboEndNorm = nextInfo.inputEndNorm;
            rt.comboChainCutNorm = nextInfo.chainCutNorm;
            rt.hit.tagMul = ResolveTagDmgMul(rt.character, rt.execTag);
            ctrlSys->OnComboStepStarted(entity, nextInfo.slot, nextInfo.stepIdx);

            EntityID leader = timelineSys->GetLeader();
            if (entity == leader)
                tacticSys->GainPip(1);

            rt.hasQueuedCombo = false;
        }
    }
    ++rt.plannedIdx;
    rt.cursor.curStageIdx = 0;

    if (rt.plannedIdx >= (int)rt.plannedTags.size())
    {
        FinishAndIdle(entity, rt);
        return false;
    }

    const SpecialAnimTag nextTag = rt.plannedTags[(size_t)rt.plannedIdx];
    rt.curTag = nextTag;

    SetUpSfxForCurTag(entity, rt);

    if (IsComboStepTag(rt.execTag) && nextTag == rt.execTag)
        rt.comboInputOpen = true;
    else
        rt.comboInputOpen = false;

    const AnimChainSpec& next = GetChain(rt.character, rt.context, nextTag);

    switch (nextTag)
    {
    case SpecialAnimTag::AttackStarted:
        rt.phase = ExecutionUnitRunTime::Phase::AttackStart;
        break;
    case SpecialAnimTag::AttackFinished:
        rt.phase = ExecutionUnitRunTime::Phase::AttackFinished;
        break;
    case SpecialAnimTag::BasicAttack:
    case SpecialAnimTag::ItemRush:
    case SpecialAnimTag::SkillA_1: case SpecialAnimTag::SkillA_2: case SpecialAnimTag::SkillA_3:
    case SpecialAnimTag::SkillB_1: case SpecialAnimTag::SkillB_2: case SpecialAnimTag::SkillB_3:
    case SpecialAnimTag::SkillC_1: case SpecialAnimTag::SkillC_2: case SpecialAnimTag::SkillC_3:
    case SpecialAnimTag::SkillD_1: case SpecialAnimTag::SkillD_2: case SpecialAnimTag::SkillD_3:
        rt.phase = ExecutionUnitRunTime::Phase::Execute;
        break;
    default:
        rt.phase = ExecutionUnitRunTime::Phase::Defending;
        break;
    }

    if (next.stages.empty())
        return AdvanceChainOrFinish(entity, rt);

    rt.cursor.isActive = true;
    return PlayStage(entity, rt, next);
}

bool BattleExecutionSystem::PlayStage(EntityID entity, ExecutionUnitRunTime& rt, const AnimChainSpec& chain)
{
    const SpecialAnimTag curTag   = rt.plannedTags[(size_t)rt.plannedIdx];
    const AnimStageSpec& stage    = chain.stages[(size_t)rt.cursor.curStageIdx];
    const AnimKey        clipKey  = stage.clipKey;
    const wstring&       clipName = animDataSys->GetClipName(rt.character, rt.context, clipKey);

    ClipTuning tuning = animDataSys->GetClipTuning(rt.character, rt.context, clipKey);
    if (stage.startNormalizedOverride) tuning.startNormalized = *stage.startNormalizedOverride;
    if (stage.endNormalizedOverride)   tuning.endNormalized = *stage.endNormalizedOverride;

    if (IsComboStepTag(curTag) && rt.cursor.curStageIdx == 0)
    {
        const SkillStepInfo& info = actionReg->GetStepInfo(rt.character, curTag);
        tuning.startNormalized = info.nextStepStartNorm;
    }

    Handle animHandle = animator->Get(entity);
    animator->SetPlaybackSpeed(animHandle, 0, tuning.playbackSpeed);

    const float fadeSeconds = max(0.f, stage.fadeDur);
    const bool  loopStage = (curTag == SpecialAnimTag::Defending);

    if (fadeSeconds > 0.f)
        animator->CrossFade(animHandle, 0, 1, clipName, fadeSeconds,
            loopStage ? ANIMTYPE::LOOP : ANIMTYPE::ONCE,
            tuning.startNormalized, tuning.endNormalized);
    else
        animator->PlaySection(animHandle, 0, clipName,
            loopStage ? ANIMTYPE::LOOP : ANIMTYPE::ONCE,
            tuning.startNormalized, tuning.endNormalized);

    rt.hit.stage = rt.cursor.curStageIdx;
    rt.hit.next = 0;
    rt.hit.lastNorm = 0.f;

    if (rt.activeIntent.targetEntity != 0u && rt.cursor.curStageIdx == 0)
    {
        Handle selfTf = tfSys->Get(entity);
        Handle tgtTf = tfSys->Get(rt.activeIntent.targetEntity);

        const _float3 a = tfSys->GetPos(selfTf);
        const _float3 b = tfSys->GetPos(tgtTf);
        const float dx = b.x - a.x;
        const float dz = b.z - a.z;
        const float len = sqrtf(dx * dx + dz * dz);
        if (len > 0.01f)
        {
            const _float2 dir = { dx / len, dz / len };
            if (curTag == SpecialAnimTag::AttackStarted)
                QueuePulse(rt, dir, kDist, kMoveDur);
            else if (curTag == SpecialAnimTag::AttackFinished)
                QueuePulse(rt, _float2{ -dir.x, -dir.y }, kDist, kMoveDur);
        }
    }

    // ========================= FX: Trail (AnimKey ±âÁØ) =========================
    const ActionFxSet* fxSet = fxReg->FindFx(rt.character, curTag);
    if (fxSet)
    {
        for (const ActionTrailClipFx& tfx : fxSet->trails)
        {
            if (tfx.clipKey != clipKey)
                continue;

            const float clipDur = animator->GetClipDuration(animHandle, clipName);
            if (clipDur <= 0.f)
                continue;

            float a = tuning.startNormalized;
            float b = tuning.endNormalized;
            if (b < a) swap(a, b);

            const float stageRangeNorm = b - a;
            if (stageRangeNorm <= 0.f)
                continue;

            const float speed = (tuning.playbackSpeed != 0.f) ? tuning.playbackSpeed : 1.f;
            const float stageDur = (stageRangeNorm * clipDur) / speed;

            float startNorm = tfx.startNorm;
            float endNorm = tfx.endNorm;
            if (endNorm < startNorm)
                endNorm = startNorm;

            const float fxStartSec = startNorm * stageDur;
            const float fxEndSec = endNorm * stageDur;

            const float duration = max(0.01f, fxEndSec - fxStartSec);
            const float afterDur = max(0.f, fxStartSec);

            EntityID attachOwner = entity;

            if (tfx.attachToWeapon)
            {
                const EntityID charEntity = dataSys->GetEntityID(rt.character);
                const EntityID weaponEntity = charEntity + 1;
                attachOwner = weaponEntity;
            }
            effectSys->PlayTrail(tfx.effectKey, attachOwner, duration, afterDur);
        }
    }
    return true;
}

bool BattleExecutionSystem::IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& rt, const AnimChainSpec& chain) const
{
    if (!rt.cursor.isActive) return true;

    const SpecialAnimTag curTag = rt.plannedTags[(size_t)rt.plannedIdx];
    if (curTag == SpecialAnimTag::Defending) return false;

    const AnimStageSpec& stage = chain.stages[(size_t)rt.cursor.curStageIdx];
    const wstring& clipName = animDataSys->GetClipName(rt.character, rt.context, stage.clipKey);
    if (clipName.empty()) return true;

    Handle animHandle = animator->Get(entity);
    if (!animator->IsPlaying(animHandle, 0)) return true;
    if (animator->IsCrossFading(animHandle)) return false;

    const float remaining = animator->GetRemainingTime(animHandle, 0);
    const float window = max(stage.fadeDur, stage.minOverlapDur);
    return (remaining <= window);
}

bool BattleExecutionSystem::IsComboInputOpen(EntityID entity) const
{
    auto it = runtimeByEntity.find(entity);
    if (it == runtimeByEntity.end())
        return false;

    const ExecutionUnitRunTime& rt = it->second;

    if (!IsComboStepTag(rt.execTag))
        return false;

    if (rt.curTag != rt.execTag)
        return false;

    Handle animHandle = animator->Get(entity);
    float curNorm = animator->GetNormalizedTime(animHandle, 0);

    return (curNorm < rt.comboEndNorm);
}

void BattleExecutionSystem::NotifyComboQueued(EntityID entity, SpecialAnimTag nextTag)
{
    auto it = runtimeByEntity.find(entity);
    if (it == runtimeByEntity.end()) return;

    ExecutionUnitRunTime& rt = it->second;

    if (!IsComboStepTag(rt.execTag))
        return;
    if (rt.hasQueuedCombo)
        return;

    rt.hasQueuedCombo = true;
    rt.queuedTag = nextTag;
}

void BattleExecutionSystem::ConsumePulse(EntityID entity, ExecutionUnitRunTime& rt, float dt)
{
    if (!rt.pulse.active || rt.pulse.remainDist <= 0.f) return;

    const float step = min(rt.pulse.speed * dt, rt.pulse.remainDist);
    if (step <= 0.f) return;

    Handle tf    = tfSys->Get(entity);
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
    EntityID target = rt.activeIntent.targetEntity;

    if (target == 0u)
    {
        target = targetSys->Get(self);
        if (target == 0u)
            return;

        rt.activeIntent.targetEntity = target;
    }

    Handle selfTf = tfSys->Get(self);
    Handle tgtTf = tfSys->Get(target);

    const _float3 a = tfSys->GetPos(selfTf);
    const _float3 b = tfSys->GetPos(tgtTf);
    const float dx = b.x - a.x, dz = b.z - a.z;
    const float len = sqrtf(dx * dx + dz * dz);
    if (len <= 1e-5f) return;

    faceSrv->PushSmoothXZ(self, _float2{ dx / len, dz / len });
}

void BattleExecutionSystem::FinishAndIdle(EntityID entity, ExecutionUnitRunTime& rt)
{
    camReg->SmoothBackToFollow(); 

    Handle animHandle = animator->Get(entity);

    if (animHandle.IsValid())
    {
        const wstring& idle = animDataSys->GetClipName(rt.character, rt.context, AnimKey::Battle_Idle);
        if (!idle.empty())
            animator->CrossFade(animHandle, 0, 1, idle, 0.08f, ANIMTYPE::LOOP);
    }

    rt.pulse.active     = false;
    rt.pulse.remainDist = 0.f;
    rt.cursor.isActive  = false;
    rt.comboInputOpen   = false;
    rt.hasQueuedCombo   = false;

    rt.sfxSeq.active     = false;
    rt.sfxSeq.curVoiceId = 0;
    rt.sfxSeq.curIdx     = -1;

    timelineSys->NotifyActionFinished(entity, rt.activeIntent);
    if (entity == timelineSys->GetLeader())
        comboCount = 0;
}

void BattleExecutionSystem::EmitHit(EntityID attacker, const ExecutionUnitRunTime& rt, EntityID target, float dmgRatio, bool critical)
{
    const float f = (float)rt.hit.baseDmg * rt.hit.tagMul * dmgRatio;
    const int dmg = (f >= 0.f) ? (int)(f + 0.5f) : (int)(f - 0.5f);
    float stun = (dmg > 0) ? (attrSys->GetConfig().stunPerDamage * dmg) : 0.f;

    attrSys->ApplyHit(target, dmg, stun);

    BattleEvent event{};
    event.eventType     = BattleBusEventType::ResolveDamageApplied;
    event.subjectEntity = attacker;
    event.payload = EventPayload_Damage{ attacker, target, dmg, critical };
    eventBus->Publish(event);

    EntityID leader = timelineSys->GetLeader();
    if (attacker == leader && dmg > 0)
    {
        if (comboCount < comboMaxCount)
            comboCount += 1;
    }
}

float BattleExecutionSystem::EstimateChainDuration(EntityID entity, const ExecutionUnitRunTime& rt, const AnimChainSpec& chain) const
{
    Handle animHandle = animator->Get(entity);

    float total = 0.f;

    for (const AnimStageSpec& stage : chain.stages)
    {
        const wstring& clipName = animDataSys->GetClipName(rt.character, rt.context, stage.clipKey);
        if (clipName.empty()) continue;

        float clipDur = animator->GetClipDuration(animHandle, clipName);
        if (clipDur <= 0.f) continue;

        ClipTuning tuning = animDataSys->GetClipTuning(rt.character, rt.context, stage.clipKey);
        if (stage.startNormalizedOverride) tuning.startNormalized = *stage.startNormalizedOverride;
        if (stage.endNormalizedOverride)   tuning.endNormalized = *stage.endNormalizedOverride;

        float a = tuning.startNormalized;
        float b = tuning.endNormalized;
        if (b < a) swap(a, b);

        float range = b - a;
        if (range <= 0.f) continue;

        float speed = (tuning.playbackSpeed != 0.f) ? tuning.playbackSpeed : 1.f;
        float stageTime = (range * clipDur) / speed;

        total += stageTime;
    }

    return total;
}

void BattleExecutionSystem::UpdateSfxSequence(ExecutionUnitRunTime& rt)
{
    if (!rt.sfxSeq.active) return;
    if (rt.sfxSeq.keys.empty()) { rt.sfxSeq.active = false; return; }

    if (rt.sfxSeq.curVoiceId == 0)
    {
        rt.sfxSeq.curIdx = 0;
        const wstring& key = rt.sfxSeq.keys[rt.sfxSeq.curIdx];
        rt.sfxSeq.curVoiceId = soundSys->Play(key);
        return;
    }

    if (soundSys->IsPlaying(rt.sfxSeq.curVoiceId))
        return; 

    rt.sfxSeq.curIdx++;

    if (rt.sfxSeq.curIdx >= static_cast<int>(rt.sfxSeq.keys.size()))
    {
        rt.sfxSeq.active = false;
        rt.sfxSeq.curVoiceId = 0;
        return;
    }
    const wstring& key = rt.sfxSeq.keys[rt.sfxSeq.curIdx];
    rt.sfxSeq.curVoiceId = soundSys->Play(key);
}

float BattleExecutionSystem::ResolveTagDmgMul(CharacterID ch, SpecialAnimTag tag) const
{
    const ActionAnimSpec& spec = actionReg->Get(ch);
    auto it = spec.dmgMulByTag.find(tag);
    if (it == spec.dmgMulByTag.end())
        return 1.f;
    return it->second;
}

void BattleExecutionSystem::SetUpSfxForCurTag(EntityID entity, ExecutionUnitRunTime& rt)
{
    if (rt.curTag == SpecialAnimTag::BasicAttack)
    {
        if (entity != timelineSys->GetLeader())
        {
            rt.sfxSeq.active = false;
            rt.sfxSeq.curVoiceId = 0;
            rt.sfxSeq.curIdx = -1;
            rt.sfxSeq.keys.clear();
            return;
        }
    }

    const ActionAnimSpec& spec = actionReg->Get(rt.character);

    auto it = spec.sfxSeqByTag.find(rt.curTag);
    if (it == spec.sfxSeqByTag.end())
    {
        rt.sfxSeq.active = false;
        rt.sfxSeq.curVoiceId = 0;
        rt.sfxSeq.curIdx = -1;
        rt.sfxSeq.keys.clear();
        return;
    }

    rt.sfxSeq.keys = it->second;
    rt.sfxSeq.curIdx = -1;
    rt.sfxSeq.curVoiceId = 0;
    rt.sfxSeq.active = true;

    if (entity == timelineSys->GetLeader())
        rt.sfxSeq.volume = 0.5f;
    else
        rt.sfxSeq.volume = 0.3f;
}

void BattleExecutionSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI

#endif
}