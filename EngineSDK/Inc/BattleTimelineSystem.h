#pragma once

#include "BattleTimelineData.h"

NS_BEGIN(Engine)
class BattleEventBus;

class ENGINE_DLL BattleTimelineSystem : public ISystem
{
public:
    explicit BattleTimelineSystem(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

    void InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig);
    void Tick(float dt);
    void EndSession();

    bool     TryCommitIntent(EntityID entity, const TimelineActionIntent& intent);
    bool     TrySetLeader(EntityID newLeader);
    EntityID GetLeader() const;

    void NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent);

    const BattleTimelineState* TryGetState()   const { return timelineState ? &(*timelineState) : nullptr; }
    TimelineClockState         GetClockState() const { return timelineState ? timelineState->clockState : TimelineClockState::Stopped; }

    bool TryGetUnitState(BattleTeam team, int slotIdx, const TimelineUnitState*& outState) const;
    bool TryGetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outState) const;

    bool IsGaugeFull(EntityID entity) const;
    bool IsDefendAllowed(EntityID entity) const;
    bool IsUnitReadyToAct(EntityID entity) const;
    int  ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const;

    void SetClock(TimelineClockState newState);
    void SetUnitGate(EntityID entity, TimelineUnitGate gate);
    void SetUnitCanAction(EntityID entity, bool canAction);

    const vector<BattleTimelineEvent>& PeekEvents() const { return eventQueue; }
    void                               ClearEvents()      { eventQueue.clear(); }

    void FreezeATB(EntityID entity, bool freeze);
    void PublishLeaderChanged(EntityID newLeader);

private:
    bool ResolveIdxByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx) const;
    void PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp = 0);

    void AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team);
    bool CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, TimelineActionIntent intent, EntityID entity, BattleTeam team);

    void ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp);
    void ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime,  const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team);

    void FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const;

private:
    SystemRegistry&        registry;
    BattleEventBus*        eventBus{};
    CharacterDataSystem*   dataSys{};
    BattleExecutionSystem* execSys{};
    BattleTargetSystem*    targetSys{};
    ActionAnimRegistry*    actionReg{};

    optional<BattleTimelineState>                   timelineState; // Ω∫≥¿º¶(ΩΩ∑‘/∑±≈∏¿”/∏Æ¥ı/º≥¡§)
    unordered_map<EntityID, pair<BattleTeam, int>>  idxByEntity;  // entity °Ê (team, slot)
    vector<BattleTimelineEvent>                     eventQueue;
};

NS_END