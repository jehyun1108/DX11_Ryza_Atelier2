#include "Enginepch.h"
#include "BattleEventBus.h"
#include "BattleSessionSystem.h"
#include "SoundSystem.h"

void BattleTimelineSystem::OnBoot()
{
    eventBus  = &registry.Get<BattleEventBus>();
    dataSys   = &registry.Get<CharacterDataSystem>();
    execSys   = &registry.Get<BattleExecutionSystem>();
    targetSys = &registry.Get<BattleTargetSystem>();
    actionReg = &registry.Get<ActionAnimRegistry>();
    ctrlSys   = &registry.Get<BattleControllerSystem>(); 
    soundSys  = &registry.Get<SoundSystem>();
}

void BattleTimelineSystem::InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig)
{
    timelineState.emplace();
    BattleTimelineState& state = *timelineState;

    state.clockState       = TimelineClockState::Running;
    state.elapsedTime      = 0.f;
    state.config           = timelineConfig;
    state.leader.curLeader = sessionState.leaderEntity;
    state.alliesUsed       = 0;
    state.enemiesUsed      = 0;

    state.alliesAp = TimelineTeamAP{};
    state.enemiesAp = TimelineTeamAP{};
    idxByEntity.clear();
    eventQueue.clear();

    for (int i = 0; i < sessionState.allies.memberCount; ++i)
    {
        const EntityID entity = sessionState.allies.members[i];
        if (entity == invalidEntity) continue;

        state.allies[i]               = {};
        state.allies[i].entity        = entity;
        state.allies[i].team          = BattleTeam::Ally;
        state.allies[i].ATB           = TimelineGauge{ 0.f, state.config.gaugeMaxValue, state.config.gaugeFillSpeed };
        state.allies[i].gateState     = TimelineUnitGate::Open;
        state.allies[i].motionState   = TimelineMotionState::Queued;
        state.allies[i].canAction     = state.config.canAction;
        state.allies[i].defendAllowed = true;

        state.alliesRuntime[i] = {};
        state.alliesRuntime[i].role.control = (entity == state.leader.curLeader) ? TimelineControlType::Player : TimelineControlType::Ally;
        state.alliesRuntime[i].role.allowCombo = (state.alliesRuntime[i].role.control == TimelineControlType::Player);

        FillSkillCatalog(entity, state.alliesRuntime[i].skillCatalog);

        idxByEntity[entity] = { BattleTeam::Ally, i };
        ++state.alliesUsed;
    }
    for (int i = 0; i < sessionState.enemies.memberCount; ++i)
    {
        const EntityID entity = sessionState.enemies.members[i];
        if (entity == invalidEntity) continue;

        state.enemies[i]             = {};
        state.enemies[i].entity      = entity;
        state.enemies[i].team        = BattleTeam::Enemy;
        state.enemies[i].ATB         = TimelineGauge{ 0.f, state.config.gaugeMaxValue, state.config.gaugeFillSpeed, false };
        state.enemies[i].gateState   = TimelineUnitGate::Open;
        state.enemies[i].motionState = TimelineMotionState::Queued;
        state.enemies[i].canAction   = state.config.canAction;

        state.enemiesRuntime[i] = {};
        state.enemiesRuntime[i].role.control = TimelineControlType::Enemy;
        state.enemiesRuntime[i].role.allowCombo = false;

        FillSkillCatalog(entity, state.enemiesRuntime[i].skillCatalog);

        idxByEntity[entity] = { BattleTeam::Enemy, i };
        ++state.enemiesUsed;
    }
}

void BattleTimelineSystem::Tick(float dt)
{
    if (!timelineState) return;
    BattleTimelineState& state = *timelineState;
    if (state.clockState != TimelineClockState::Running) return;

    state.elapsedTime += dt;

    for (int i = 0; i < state.alliesUsed; ++i)
    {
        auto& unit = state.allies[i];
        if (unit.entity != 0u)
            AdvanceGauge(unit, dt, unit.entity, BattleTeam::Ally);
    }
    for (int i = 0; i < state.enemiesUsed; ++i)
    {
        auto& unit = state.enemies[i];
        if (unit.entity != 0u)
            AdvanceGauge(unit, dt, unit.entity, BattleTeam::Enemy);
    }
}

void BattleTimelineSystem::EndSession()
{
    if (!timelineState) return;
    timelineState.reset();
    idxByEntity.clear();
    eventQueue.clear();
}

void BattleTimelineSystem::CommitIntent(EntityID entity, const TimelineActionIntent& inIntent)
{
    TimelineActionIntent intent = inIntent;

    if (intent.battleCmd == BattleCommand::AttackBasic || intent.battleCmd == BattleCommand::Skill)
    {
        const CharacterID    characterId = dataSys->GetCharacterID(entity);
        const SpecialAnimTag execTag = intent.specialTag.value_or(SpecialAnimTag::BasicAttack);
        const AnimChainSpec& chainSpec = execSys->GetChain(characterId, AnimContext::Battle, execTag);

        if (intent.targetEntity == 0u)
            intent.targetEntity = targetSys->Get(entity);
    }

    auto [team, slotIndex] = RequireIdxByEntity(entity);
    BattleTimelineState& state = *timelineState;

    TimelineUnitState& unitState = (team == BattleTeam::Ally) ? state.allies[slotIndex] : state.enemies[slotIndex];
    TimelineUnitRunTime& unitRuntime = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIndex] : state.enemiesRuntime[slotIndex];

    CommitInternal(unitState, unitRuntime, intent, entity, team);
}

void BattleTimelineSystem::CommitComboIntent(EntityID entity, const TimelineActionIntent& inIntent)
{
    TimelineActionIntent intent = inIntent;

    const CharacterID    characterId = dataSys->GetCharacterID(entity);
    const SpecialAnimTag execTag = intent.specialTag.has_value() ? *intent.specialTag : SpecialAnimTag::BasicAttack;
    (void)execSys->GetChain(characterId, AnimContext::Battle, execTag);

    if (intent.targetEntity == 0u)
        intent.targetEntity = targetSys->Get(entity);

    auto [team, slotIndex] = RequireIdxByEntity(entity);
    BattleTimelineState& state = *timelineState;

    TimelineUnitState& unitState = (team == BattleTeam::Ally) ? state.allies[slotIndex] : state.enemies[slotIndex];
    TimelineUnitRunTime& unitRuntime = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIndex] : state.enemiesRuntime[slotIndex];

    unitState.gateState = TimelineUnitGate::Closed;

    const int resolvedCost = ResolveSkillApCost(entity, intent.specialTag);
    intent.apCost = resolvedCost;

    if (intent.apCost != 0)
        ApplyApDelta(team, entity, -intent.apCost);

    unitState.pendingIntent = intent;
    unitState.activeIntent = intent;
    unitState.motionState = TimelineMotionState::Executing;
    unitState.defendAllowed = false;

    const bool started = execSys->BeginAction(entity, intent, false);
    if (started)
        ctrlSys->OnActionExecutionStarted(entity, intent);
}

EntityID BattleTimelineSystem::GetLeader() const
{
    if (!timelineState.has_value()) return invalidEntity;
    return timelineState->leader.curLeader;
}

void BattleTimelineSystem::SetLeader(EntityID newLeader)
{
    auto [team, slotIndex] = RequireIdxByEntity(newLeader);

    BattleTimelineState& state = *timelineState;
    const EntityID previousLeader = state.leader.curLeader;
    if (previousLeader == newLeader) return;

    if (previousLeader != invalidEntity)
    {
        auto [prevTeam, prevSlotIndex] = RequireIdxByEntity(previousLeader);
        if (prevTeam == BattleTeam::Ally)
            state.alliesRuntime[prevSlotIndex].role.control = TimelineControlType::Ally;
    }

    state.alliesRuntime[slotIndex].role.control = TimelineControlType::Player;
    state.leader.curLeader = newLeader;

    PublishLeaderChanged(newLeader);
}

void BattleTimelineSystem::NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent)
{
    auto [team, slotIndex] = RequireIdxByEntity(entity);

    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unitState = (team == BattleTeam::Ally) ? state.allies[slotIndex] : state.enemies[slotIndex];
    TimelineUnitRunTime& unitRuntime = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIndex] : state.enemiesRuntime[slotIndex];

    if (finishedIntent.battleCmd == BattleCommand::Defend)
    {
        ctrlSys->OnActionExecutionFinished(entity, finishedIntent);
        return;
    }
    ApplyResolveReward(unitState, unitRuntime, finishedIntent, entity, team);

    unitState.motionState = TimelineMotionState::Queued;
    unitState.ATB.isFrozen = false;

    if (team == BattleTeam::Ally && state.leader.comboOwner == entity)
    {
        state.leader.comboOwner = 0u;
        unitRuntime.role.allowCombo = false;
    }

    unitState.defendAllowed =
        (unitState.motionState == TimelineMotionState::Queued) &&
        (unitState.gateState == TimelineUnitGate::Open) &&
        unitState.canAction;

    PushEvent(BattleTimelineEventType::ActionFinished, entity, team, 0);

    ctrlSys->OnActionExecutionFinished(entity, finishedIntent);
}

const TimelineUnitState& BattleTimelineSystem::GetUnitState(BattleTeam team, int slotIdx) const
{
    const BattleTimelineState& s = *timelineState;

    if (team == BattleTeam::Ally)
        return s.allies[slotIdx];
    else
        return s.enemies[slotIdx];
}

const TimelineUnitState& BattleTimelineSystem::GetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIndex) const
{
    auto [team, slotIndex] = RequireIdxByEntity(entity);
    outTeam = team; outSlotIndex = slotIndex;
    return GetUnitState(team, slotIndex);
}

bool BattleTimelineSystem::IsGaugeFull(EntityID entity) const
{
    BattleTeam team{}; int slotIndex{};
    const TimelineUnitState& unitState = GetUnitStateByEntity(entity, team, slotIndex);
    return (unitState.ATB.curValue >= unitState.ATB.maxValue);
}

bool BattleTimelineSystem::IsDefendAllowed(EntityID entity) const
{
    BattleTeam team{}; int slotIndex{};
    const TimelineUnitState& unitState = GetUnitStateByEntity(entity, team, slotIndex);

    if (unitState.motionState != TimelineMotionState::Queued) return false;
    if (unitState.gateState != TimelineUnitGate::Open)        return false;
    if (!unitState.canAction)                                 return false;

    return true;
}

bool BattleTimelineSystem::IsUnitReadyToAct(EntityID entity) const
{
    BattleTeam team;
    int        slotIdx;
    tie(team, slotIdx) = RequireIdxByEntity(entity);

    const BattleTimelineState& state = *timelineState;
    const TimelineUnitState& unit = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];

    if (team == BattleTeam::Ally)
    {
        const EntityID comboOwner = state.leader.comboOwner;
        if (comboOwner != invalidEntity && comboOwner != entity)
            return false;
    }

    if (unit.gateState != TimelineUnitGate::Open)        return false;
    if (!unit.canAction)                                 return false;
    if (unit.motionState != TimelineMotionState::Queued) return false;

    return (unit.ATB.curValue >= unit.ATB.maxValue);
}

void BattleTimelineSystem::SetClock(TimelineClockState newState)
{
    if (!timelineState) return;
    BattleTimelineState& state = *timelineState;

    if (state.clockState == newState) return;
    state.clockState = newState;

    PushEvent((newState == TimelineClockState::Stopped) ? BattleTimelineEventType::TimelinePaused : BattleTimelineEventType::TimelineResumed,0u,
        BattleTeam::Ally, 0);
}

void BattleTimelineSystem::SetUnitGate(EntityID entity, TimelineUnitGate gate)
{
    auto [team, slotIndex] = RequireIdxByEntity(entity);
    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unitState = (team == BattleTeam::Ally) ? state.allies[slotIndex] : state.enemies[slotIndex];
    unitState.gateState = gate;
}

void BattleTimelineSystem::SetUnitCanAction(EntityID entity, bool canAction)
{
    auto [team, slotIndex] = RequireIdxByEntity(entity);
    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unitState = (team == BattleTeam::Ally) ? state.allies[slotIndex] : state.enemies[slotIndex];
    unitState.canAction = canAction;
}

void BattleTimelineSystem::FreezeATB(EntityID entity, bool freeze)
{
    auto [team, slotIndex] = RequireIdxByEntity(entity);
    BattleTimelineState& state = *timelineState;
    TimelineUnitState& unitState = (team == BattleTeam::Ally) ? state.allies[slotIndex] : state.enemies[slotIndex];
    unitState.ATB.isFrozen = freeze;
}

void BattleTimelineSystem::PublishLeaderChanged(EntityID newLeader)
{
    BattleTimelineState& state = *timelineState;
    PushEvent(BattleTimelineEventType::LeaderChanged, newLeader, BattleTeam::Ally, 0);

    soundSys->Play(L"015_switch", 0.3f);
    CharacterID characterId = dataSys->GetCharacterID(newLeader);
    switch (characterId)
    {
    case CharacterID::Ryza:     soundSys->Play(L"ryza_19", 0.3f); break;
    case CharacterID::Klaudia:  soundSys->Play(L"klaudia_25", 0.3f); break;
    case CharacterID::Patricia: soundSys->Play(L"patricia_16", 0.3f); break;
    }
    			

    BattleEvent busEvent{};
    busEvent.eventType = BattleBusEventType::LeaderChanged;
    busEvent.subjectEntity = newLeader;
    eventBus->Publish(busEvent);
}

void BattleTimelineSystem::OnDamageApplied(const EventPayload_Damage& dmg)
{
    BattleTimelineState& state = *timelineState;

    EntityID attacker = dmg.attackerEntity;
    if (attacker != invalidEntity && dmg.damageAmount > 0)
    {
        auto [atkTeam, atkSlot] = RequireIdxByEntity(attacker);
        if (atkTeam == BattleTeam::Ally)
        {
            ApplyApDelta(atkTeam, attacker, +1);
        }
    }

    EntityID target = dmg.targetEntity;
    auto [team, slotIdx] = RequireIdxByEntity(target);
    TimelineUnitState& unit = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];

    if (unit.motionState == TimelineMotionState::Executing || unit.motionState == TimelineMotionState::Preparing)
        return;

    float backAmount = 0.f;

    if (state.config.onHitBackPercent > 0.f)
        backAmount = state.config.gaugeMaxValue * state.config.onHitBackPercent;
    else if (state.config.onHitBackSec > 0.f)
        backAmount = unit.ATB.fillSpeed * state.config.onHitBackSec;

    unit.ATB.curValue = max(0.f, unit.ATB.curValue - backAmount);
}

pair<BattleTeam, int> BattleTimelineSystem::RequireIdxByEntity(EntityID entity) const
{
    auto it = idxByEntity.find(entity);
    return it->second;
}

void BattleTimelineSystem::PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp)
{
    BattleTimelineEvent timelineEvent{};
    timelineEvent.eventType = type;
    timelineEvent.subjectEntity = subject;
    timelineEvent.subjectTeam = team;
    timelineEvent.deltaAp = deltaAp;
    eventQueue.push_back(timelineEvent);
}

void BattleTimelineSystem::AdvanceGauge(TimelineUnitState& unitState, float dt, EntityID entity, BattleTeam team)
{
    unitState.defendAllowed = (unitState.motionState == TimelineMotionState::Queued) && (unitState.gateState == TimelineUnitGate::Open) && unitState.canAction;

    if (unitState.ATB.isFrozen)  return;
    if (unitState.gateState != TimelineUnitGate::Open) return;
    if (!unitState.canAction)  return;
    if (unitState.motionState == TimelineMotionState::Executing) return;

    const float prevValue = unitState.ATB.curValue;
    unitState.ATB.curValue = min(unitState.ATB.curValue + unitState.ATB.fillSpeed * dt, unitState.ATB.maxValue);

    if (prevValue < unitState.ATB.maxValue && unitState.ATB.curValue >= unitState.ATB.maxValue)
    {
        PushEvent(BattleTimelineEventType::FullGauge, entity, team, 0);

        if (team == BattleTeam::Ally)
        {
            BattleTimelineState& s = *timelineState;
            if (s.leader.curLeader == entity)
                soundSys->Play(L"01_turn_get", 0.2f);
        }
    }
}

void BattleTimelineSystem::CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, TimelineActionIntent intent, EntityID entity, BattleTeam team)
{
    const int resolvedCost = ResolveSkillApCost(entity, intent.specialTag);
    intent.apCost = resolvedCost;

    if (intent.apCost != 0)
        ApplyApDelta(team, entity, -intent.apCost);

    unitState.ATB.curValue = 0.f;
    unitState.ATB.isFrozen = true;

    unitState.pendingIntent = intent;
    unitState.activeIntent = intent;
    unitState.motionState = TimelineMotionState::Preparing;

    PushEvent(BattleTimelineEventType::ActionCommitted, entity, team, -intent.apCost);

    if (intent.battleCmd == BattleCommand::AttackBasic || intent.battleCmd == BattleCommand::Skill)
        unitState.defendAllowed = false;

    if (team == BattleTeam::Ally)
    {
        const bool startComboLock =
            (unitRunTime.role.control == TimelineControlType::Player) &&
            unitRunTime.role.allowCombo &&
            (intent.battleCmd == BattleCommand::AttackBasic || intent.battleCmd == BattleCommand::Skill);

        if (startComboLock)
        {
            BattleTimelineState& state = *timelineState;
            state.leader.comboOwner = entity;

            for (int allyIndex = 0; allyIndex < state.alliesUsed; ++allyIndex)
                state.alliesRuntime[allyIndex].role.allowCombo = false;

            unitRunTime.role.allowCombo = true;
        }
    }

    unitState.motionState = TimelineMotionState::Executing;

    const bool started = execSys->BeginAction(entity, intent, true);
    if (started)
        ctrlSys->OnActionExecutionStarted(entity, intent);
}

void BattleTimelineSystem::ApplyApDelta(BattleTeam team, EntityID entity, int deltaAp)
{
    BattleTimelineState& s = *timelineState;
    TimelineTeamAP& shared = (team == BattleTeam::Ally) ? s.alliesAp : s.enemiesAp;

    const int prevAp = shared.curAp;
    shared.curAp = clamp(prevAp + deltaAp, 0, shared.maxAp);

    const int appliedDelta = shared.curAp - prevAp;
    if (appliedDelta != 0)
        PushEvent(BattleTimelineEventType::ApChanged, entity, team, appliedDelta);
}

int BattleTimelineSystem::ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const
{
    if (!specialTag.has_value()) return 0;

    const CharacterID characterId = dataSys->GetCharacterID(entity);
    const auto& actionSpec = actionReg->Get(characterId);
    auto iterator = actionSpec.apCostByTag.find(specialTag.value());
    return (iterator == actionSpec.apCostByTag.end()) ? 0 : iterator->second;
}

void BattleTimelineSystem::GetApSnapshot(EntityID entity, int& outCurAp, int& outMaxAp) const
{
    auto [team, slotIdx] = RequireIdxByEntity(entity);
    const BattleTimelineState& s = *timelineState;

    const TimelineTeamAP& shared = (team == BattleTeam::Ally) ? s.alliesAp : s.enemiesAp;
    outCurAp = shared.curAp;
    outMaxAp = shared.maxAp;
}

void BattleTimelineSystem::FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const
{
    outCatalog.clear();

    const CharacterID characterId = dataSys->GetCharacterID(entity);
    const auto& actionSpec = actionReg->Get(characterId);

    auto shouldExpose = [](SpecialAnimTag tag) { return tag != SpecialAnimTag::Intro; };

    for (const auto& entry : actionSpec.specials)
    {
        const SpecialAnimTag tag = entry.first;
        if (!shouldExpose(tag)) continue;

        int apCost = 0;
        if (auto iterator = actionSpec.apCostByTag.find(tag); iterator != actionSpec.apCostByTag.end())
            apCost = iterator->second;

        outCatalog.push_back(TimelineSkillInfo{ tag, apCost });
    }
}

void BattleTimelineSystem::OnUnitRemoved(EntityID entity)
{
    auto [team, slotIdx] = RequireIdxByEntity(entity);
    BattleTimelineState& s = *timelineState;

    TimelineUnitState& u = (team == BattleTeam::Ally) ? s.allies[slotIdx] : s.enemies[slotIdx];

    u.canAction = false;
    u.gateState = TimelineUnitGate::Closed;
    u.ATB.isFrozen = true;
    u.motionState = TimelineMotionState::Finished;
}

void BattleTimelineSystem::ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team)
{
    BattleTimelineState& state = *timelineState;

    const bool isLeader = (team == BattleTeam::Ally && entity == state.leader.curLeader);
    const bool isSkill = (resolvedIntent.battleCmd == BattleCommand::Skill);

    if (isLeader && isSkill)
    {
        state.alliesAp.tacticLv += 1;
        soundSys->Play(L"04_tlvup", 0.2f);
    }
}