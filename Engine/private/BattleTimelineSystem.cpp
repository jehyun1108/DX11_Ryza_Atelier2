#include "Enginepch.h"

void BattleTimelineSystem::InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig)
{
    timelineState.emplace();
    BattleTimelineState& state = *timelineState;

    state.clockState = TimelineClockState::Running;
    state.elapsedTime = 0.f;
    state.config = timelineConfig;
    state.leader.curLeader = sessionState.leaderEntity;

    state.alliesUsed = 0;
    state.enemiesUsed = 0;

    for (int allyIdx = 0; allyIdx < sessionState.allies.memberCount; ++allyIdx)
    {
        const EntityID entity = sessionState.allies.members[allyIdx];
        if (entity == invalidEntity) continue;

        state.allies[allyIdx] = {};
        state.allies[allyIdx].entity = entity;
        state.allies[allyIdx].team = BattleTeam::Ally;
        state.allies[allyIdx].ap = TimelineAP{};
        state.allies[allyIdx].ATB = TimelineGauge{ 0.f, state.config.gaugeMaxValue, state.config.gaugeFillSpeed };
        state.allies[allyIdx].gateState = TimelineUnitGate::Open;
        state.allies[allyIdx].motionState = TimelineMotionState::Queued;
        state.allies[allyIdx].canAction = state.config.canAction;

        state.alliesRuntime[allyIdx] = {};
        state.alliesRuntime[allyIdx].role.control = (entity == state.leader.curLeader) ? TimelineControlType::Player : TimelineControlType::Ally;
        state.alliesRuntime[allyIdx].role.allowCombo = (state.alliesRuntime[allyIdx].role.control == TimelineControlType::Player);

        FillSkillCatalog(entity, state.alliesRuntime[allyIdx].skillCatalog);

        idxByEntity[entity] = { BattleTeam::Ally, allyIdx };
        ++state.alliesUsed;
    }
    for (int enemyIdx = 0; enemyIdx < sessionState.enemies.memberCount; ++enemyIdx)
    {
        const EntityID entity = sessionState.enemies.members[enemyIdx];
        if (entity == invalidEntity) continue;

        state.enemies[enemyIdx] = {};
        state.enemies[enemyIdx].entity = entity;
        state.enemies[enemyIdx].team = BattleTeam::Enemy;
        state.enemies[enemyIdx].ap = TimelineAP{};
        state.enemies[enemyIdx].ATB = TimelineGauge{ 0.f, state.config.gaugeMaxValue, state.config.gaugeFillSpeed, false };
        state.enemies[enemyIdx].gateState = TimelineUnitGate::Open;
        state.enemies[enemyIdx].motionState = TimelineMotionState::Queued;
        state.enemies[enemyIdx].canAction = state.config.canAction;

        state.enemiesRuntime[enemyIdx] = {};
        state.enemiesRuntime[enemyIdx].role.control = TimelineControlType::Enemy;
        state.enemiesRuntime[enemyIdx].role.allowCombo = false;

        FillSkillCatalog(entity, state.enemiesRuntime[enemyIdx].skillCatalog);

        idxByEntity[entity] = { BattleTeam::Enemy, enemyIdx };
        ++state.enemiesUsed;
    }

    eventQueue.clear();
}

void BattleTimelineSystem::Tick(float dt)
{
    if (!timelineState.has_value()) return;
    BattleTimelineState& state = *timelineState;
    if (state.clockState != TimelineClockState::Running) return;

    state.elapsedTime += dt;

    for (int slotIdx = 0; slotIdx < state.alliesUsed; ++slotIdx)
    {
        TimelineUnitState& unit = state.allies[slotIdx];
        if (unit.entity == invalidEntity) continue;
        AdvanceGauge(unit, dt, unit.entity, BattleTeam::Ally);
    }
    for (int slotIdx = 0; slotIdx < state.enemiesUsed; ++slotIdx)
    {
        TimelineUnitState& unit = state.enemies[slotIdx];
        if (unit.entity == invalidEntity) continue;
        AdvanceGauge(unit, dt, unit.entity, BattleTeam::Enemy);
    }
}

void BattleTimelineSystem::EndSession()
{
    if (!timelineState.has_value()) return;
    timelineState.reset();
    idxByEntity.clear();
    eventQueue.clear();
}

bool BattleTimelineSystem::TryCommitIntent(EntityID entity, const TimelineActionIntent& intent)
{
    if (!timelineState.has_value()) return false;

    BattleTeam team{};
    int slotIdx{};
    if (!ResolveIdxByEntity(entity, team, slotIdx)) return false;

    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unit = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];
    TimelineUnitRunTime& run = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIdx] : state.enemiesRuntime[slotIdx];

    return CommitInternal(unit, run, intent, entity, team);
}

void BattleTimelineSystem::NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent)
{
    if (!timelineState.has_value()) return;

    BattleTeam team{};
    int slotIdx{};
    if (!ResolveIdxByEntity(entity, team, slotIdx)) return;

    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unit = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];
    TimelineUnitRunTime& run = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIdx] : state.enemiesRuntime[slotIdx];

    ApplyResolveReward(unit, run, finishedIntent, entity, team);
    unit.motionState = TimelineMotionState::Queued;
    unit.ATB.isFrozen = false;
    PushEvent(BattleTimelineEventType::ActionFinished, entity, team, 0);
}

bool BattleTimelineSystem::TryGetUnitState(BattleTeam team, int slotIdx, const TimelineUnitState*& outState) const
{
    if (!timelineState.has_value()) return false;
    const BattleTimelineState& state = *timelineState;

    if (team == BattleTeam::Ally)
    {
        if (slotIdx >= state.alliesUsed) return false;
        outState = &state.allies[slotIdx];
        return true;
    }
    else if (team == BattleTeam::Enemy)
    {
        if (slotIdx >= state.enemiesUsed) return false;
        outState = &state.enemies[slotIdx];
        return true;
    }
    return false;
}

bool BattleTimelineSystem::TryGetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outState) const
{
    if (!timelineState.has_value()) return false;
    if (!ResolveIdxByEntity(entity, outTeam, outSlotIdx)) return false;

    const BattleTimelineState& state = *timelineState;
    if (outTeam == BattleTeam::Ally)       outState = &state.allies[outSlotIdx];
    else if (outTeam == BattleTeam::Enemy) outState = &state.enemies[outSlotIdx];
    else                                   return false;
    return true;
}

bool BattleTimelineSystem::IsGaugeFull(EntityID entity) const
{
    const TimelineUnitState* unit{};
    BattleTeam team{};
    int slotIdx{};
    if (!TryGetUnitStateByEntity(entity, team, slotIdx, unit)) return false;
    return (unit->ATB.curValue >= unit->ATB.maxValue);
}

bool BattleTimelineSystem::IsUnitReadyToAct(EntityID entity) const
{
    const TimelineUnitState* unit{};
    BattleTeam team{};
    int slotIdx{};
    if (!TryGetUnitStateByEntity(entity, team, slotIdx, unit)) return false;

    if (unit->gateState != TimelineUnitGate::Open)        return false;
    if (!unit->canAction)                                 return false;
    if (unit->motionState != TimelineMotionState::Queued) return false;
    return (unit->ATB.curValue >= unit->ATB.maxValue);
}

void BattleTimelineSystem::SetClock(TimelineClockState newState)
{
    if (!timelineState.has_value()) return;
    BattleTimelineState& state = *timelineState;

    if (state.clockState == newState) return;
    state.clockState = newState;

    PushEvent((newState == TimelineClockState::Stopped) ? BattleTimelineEventType::TimelinePaused : BattleTimelineEventType::TimelineResumed, 
        invalidEntity,  BattleTeam::Ally, 0);
}

void BattleTimelineSystem::SetUnitGate(EntityID entity, TimelineUnitGate gate)
{
    if (!timelineState.has_value()) return;

    BattleTeam team{};
    int slotIdx{};
    if (!ResolveIdxByEntity(entity, team, slotIdx)) return;

    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unit = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];
    unit.gateState = gate;
}

void BattleTimelineSystem::SetUnitCanAction(EntityID entity, bool canAction)
{
    if (!timelineState.has_value()) return;

    BattleTeam team{};
    int slotIdx{};
    if (!ResolveIdxByEntity(entity, team, slotIdx)) return;

    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unit = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];
    unit.canAction = canAction;
}

bool BattleTimelineSystem::ResolveIdxByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx) const
{
    auto it = idxByEntity.find(entity);
    if (it == idxByEntity.end()) return false;
    outTeam = it->second.first;
    outSlotIdx = it->second.second;
    return true;
}

void BattleTimelineSystem::PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp)
{
    BattleTimelineEvent evt{};
    evt.eventType = type;
    evt.subjectEntity = subject;
    evt.subjectTeam = team;
    evt.deltaAp = deltaAp;
    eventQueue.push_back(evt);
}

void BattleTimelineSystem::AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team)
{
    if (unit.ATB.isFrozen) return;
    if (unit.gateState != TimelineUnitGate::Open) return;
    if (!unit.canAction) return;
    if (unit.motionState == TimelineMotionState::Executing) return;

    const float prev = unit.ATB.curValue;
    unit.ATB.curValue += unit.ATB.fillSpeed * dt;
    if (unit.ATB.curValue > unit.ATB.maxValue) unit.ATB.curValue = unit.ATB.maxValue;

    if (prev < unit.ATB.maxValue && unit.ATB.curValue >= unit.ATB.maxValue)
        PushEvent(BattleTimelineEventType::FullGauge, entity, team, 0);
}

bool BattleTimelineSystem::CommitInternal(TimelineUnitState& unitState,
    TimelineUnitRunTime& unitRunTime,
    TimelineActionIntent intent,
    EntityID entity, BattleTeam team)
{
    if (!unitState.canAction)                                 return false;
    if (unitState.gateState   != TimelineUnitGate::Open)        return false;
    if (unitState.motionState != TimelineMotionState::Queued) return false;
    if (unitState.ATB.curValue < unitState.ATB.maxValue)      return false;

    const int resolvedCost = ResolveSkillApCost(entity, intent.specialTag);
    intent.apCost = resolvedCost;

    if (intent.apCost > unitState.ap.curAp) return false;

    unitState.ap.curAp -= intent.apCost;
    if (unitState.ap.curAp < 0) unitState.ap.curAp = 0;

    unitState.ATB.curValue = 0.f;
    unitState.ATB.isFrozen = true;
    unitState.pendingIntent = intent;
    unitState.activeIntent = intent;
    unitState.motionState = TimelineMotionState::Preparing;

    PushEvent(BattleTimelineEventType::ActionCommitted, entity, team, -intent.apCost);

    unitState.motionState = TimelineMotionState::Executing;
    return true;
}

void BattleTimelineSystem::ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp)
{
    const int before = unit.ap.curAp;
    unit.ap.curAp += deltaAp;
    if (unit.ap.curAp < 0)              unit.ap.curAp = 0;
    if (unit.ap.curAp > unit.ap.maxAp)  unit.ap.curAp = unit.ap.maxAp;

    const int applied = unit.ap.curAp - before;
    if (applied != 0)
        PushEvent(BattleTimelineEventType::ApChanged, entity, team, applied);
}

void BattleTimelineSystem::ApplyResolveReward(TimelineUnitState& unitState,
    const TimelineUnitRunTime& unitRuntime,
    const TimelineActionIntent& resolvedIntent,
    EntityID entity, BattleTeam team)
{
    const bool isBasic = (resolvedIntent.specialTag.has_value() &&  resolvedIntent.specialTag.value() == SpecialAnimTag::BasicAttack);
    const int  deltaAp = isBasic ? unitRuntime.policy.apGainBasicAttack : unitRuntime.policy.apGainSkillAttack;

    ApplyApDelta(unitState, entity, team, deltaAp);
}

int BattleTimelineSystem::ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const
{
    if (!specialTag.has_value()) return 0;

    auto& actionReg = registry.Get<ActionAnimRegistry>();
    auto& dataSys = registry.Get<CharacterDataSystem>();

    const auto characterId = dataSys.GetCharacterID(entity);
    const auto* spec = actionReg.TryGet(characterId);
    if (!spec) return 0;

    auto it = spec->apCostByTag.find(specialTag.value());
    return (it == spec->apCostByTag.end()) ? 0 : it->second;
}

void BattleTimelineSystem::FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const
{
    outCatalog.clear();

    auto* actionReg = registry.TryGet<ActionAnimRegistry>();
    auto* dataSys = registry.TryGet<CharacterDataSystem>();
    if (!actionReg || !dataSys) return;

    const auto characterId = dataSys->GetCharacterID(entity);
    const auto* spec = actionReg->TryGet(characterId);
    if (!spec) return;

    auto shouldExpose = [](SpecialAnimTag tag)
        {
            if (tag == SpecialAnimTag::Intro) return false;
            return true;
        };

    for (const auto& kv : spec->specials)
    {
        const SpecialAnimTag tag = kv.first;
        if (!shouldExpose(tag)) continue;

        int apCost = 0;
        if (auto it = spec->apCostByTag.find(tag); it != spec->apCostByTag.end())
            apCost = it->second;

        outCatalog.push_back(TimelineSkillInfo{ tag, apCost });
    }
}