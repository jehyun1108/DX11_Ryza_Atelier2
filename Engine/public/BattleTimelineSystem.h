#pragma once

#include "BattleEventBusData.h"

NS_BEGIN(Engine)
class BattleEventBus;
struct BattleSessionState;

class ENGINE_DLL BattleTimelineSystem : public ISystem
{
public:
    explicit BattleTimelineSystem(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

    void InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig);
    void Tick(float dt);
    void EndSession();

    void CommitIntent(EntityID entity, const TimelineActionIntent& inIntent);
    void CommitComboIntent(EntityID entity, const TimelineActionIntent& inIntent);

    void     SetLeader(EntityID newLeader);
    EntityID GetLeader() const;

    void NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent);

    const BattleTimelineState& GetState()      const { return *timelineState; }
    TimelineClockState         GetClockState() const { return timelineState ? timelineState->clockState : TimelineClockState::Stopped; }
    const TimelineUnitState&   GetUnitState(BattleTeam team, int slotIdx) const;
    const TimelineUnitState&   GetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx) const;

    bool IsGaugeFull(EntityID entity) const;
    bool IsDefendAllowed(EntityID entity) const;
    bool IsUnitReadyToAct(EntityID entity) const;
    int  ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const;
    void GetApSnapshot(EntityID entity, int& outCurAp, int& outMaxAp) const;

    void SetClock(TimelineClockState newState);
    void SetUnitGate(EntityID entity, TimelineUnitGate gate);
    void SetUnitCanAction(EntityID entity, bool canAction);

    const vector<BattleTimelineEvent>& PeekEvents() const { return eventQueue; }
    void                               ClearEvents()      { eventQueue.clear(); }

    void FreezeATB(EntityID entity, bool freeze);
    void PublishLeaderChanged(EntityID newLeader);
    void OnDamageApplied(const EventPayload_Damage& dmg);
    void OnUnitRemoved(EntityID entity);
    void ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team);

private:
    pair<BattleTeam, int> RequireIdxByEntity(EntityID entity) const;
    void PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp = 0);
    void AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team);
    void CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, TimelineActionIntent intent, EntityID entity, BattleTeam team);
    void ApplyApDelta(BattleTeam team, EntityID entity, int deltaAp);
    void FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const;

private:
    optional<BattleTimelineState>                   timelineState; 
    unordered_map<EntityID, pair<BattleTeam, int>>  idxByEntity; 
    vector<BattleTimelineEvent>                     eventQueue;

private:
    SystemRegistry&         registry;
    BattleEventBus*         eventBus{};
    CharacterDataSystem*    dataSys{};
    BattleExecutionSystem*  execSys{};
    BattleTargetSystem*     targetSys{};
    ActionAnimRegistry*     actionReg{};
    BattleControllerSystem* ctrlSys{};
    SoundSystem*            soundSys{};
};

NS_END