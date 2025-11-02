#pragma once

#include "BattleTimelineData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleTimelineSystem
{
public:
    explicit BattleTimelineSystem(SystemRegistry& registry) : registry(registry) {}

    // 세션 수명
    void InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig);
    void Tick(float dt);
    void EndSession();

    // Intent 경로(플레이어/AI 공용)
    bool TryCommitIntent(EntityID entity, const TimelineActionIntent& intent);

    // 실행 종료 콜백
    void NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent);

    // 질의/설정
    const BattleTimelineState* TryGetState()   const { return timelineState ? &(*timelineState) : nullptr; }
    TimelineClockState         GetClockState() const { return timelineState ? timelineState->clockState : TimelineClockState::Stopped; }

    bool TryGetUnitState(BattleTeam team, int slotIdx, const TimelineUnitState*& outState) const;
    bool TryGetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outState) const;

    bool IsGaugeFull(EntityID entity) const;
    bool IsUnitReadyToAct(EntityID entity) const;
    int  ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const;

    void SetClock(TimelineClockState newState);
    void SetUnitGate(EntityID entity, TimelineUnitGate gate);
    void SetUnitCanAction(EntityID entity, bool canAction);

    // 이벤트 큐
    const vector<BattleTimelineEvent>& PeekEvents() const { return eventQueue; }
    void                              ClearEvents()       { eventQueue.clear(); }

private:
    static constexpr int kMaxSlots = 3;

    bool ResolveIdxByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx) const;
    void PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp = 0);

    void AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team);
    bool CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, TimelineActionIntent intent, EntityID entity, BattleTeam team);

    void ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp);
    void ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime,  const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team);

    void FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const;

private:
    SystemRegistry& registry;

    optional<BattleTimelineState>                   timelineState; // 스냅샷(슬롯/런타임/리더/설정)
    unordered_map<EntityID, pair<BattleTeam, int>>  idxByEntity;  // entity → (team, slot)
    vector<BattleTimelineEvent>                     eventQueue;
};

NS_END