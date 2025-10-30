#pragma once

#include "BattleTimelineData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleTimelineSystem
{
public:
	explicit BattleTimelineSystem(SystemRegistry& registry) : registry(registry) {}

	void InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig);
	void Tick(float dt);
	void EndSession();

	// Player 
	bool EnqueuePlayerIntent(EntityID playerEntity, const TimelineActionIntent& intent);
	bool TryCommitIntent(EntityID entity, const TimelineActionIntent& intent);
	void AutoCommitForAI();
	void NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent);
	bool SwapLeader(EntityID newLeaderEntity);

	// State
	const BattleTimelineState* TryGetState()   const { return timelineState ? &(*timelineState) : nullptr; }
	TimelineClockState         GetClockState() const { return timelineState ? timelineState->clockState : TimelineClockState::Stopped; }

	// Gauge
	bool TryGetUnitState(BattleTeam team, int slotIdx, const TimelineUnitState*& outState) const;
	bool TryGetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outState) const;
	bool IsGaugeFull(EntityID entity) const;
	bool IsUnitReadyToAct(EntityID entity) const;
	int  ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const;

	//  Setting
	void SetClock(TimelineClockState newState);
	void SetUnitGate(EntityID entity, TimelineUnitGate gate);
	void SetUnitCanAction(EntityID entity, bool canAction);

	// EventQueue
	const vector<BattleTimelineEvent>& PeekEvents() const { return eventQueues; }
	void                              ClearEvents()       { eventQueues.clear(); }

private:
	static constexpr int kMaxCount = 3;
	static BattleTeam OppositeTeam(BattleTeam team) { return (team == BattleTeam::Ally) ? BattleTeam::Enemy : BattleTeam::Ally;}
	
	bool ResolveIdxByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx);
	void PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp = 0);

	void AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team);
	bool CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, TimelineActionIntent intent, EntityID entity, BattleTeam team);
	bool BuildAiIntent(TimelineUnitState& unitState,TimelineUnitRunTime& unitRuntime, TimelineActionIntent& outIntent);
	void ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp);
	void ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team);

	void     FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const;
	EntityID ResolveOpponentTargetEntity(BattleTeam myTeam) const;

private:
	SystemRegistry& registry;
	optional<BattleTimelineState>                  timelineState; // snapShot
	unordered_map<EntityID, pair<BattleTeam, int>> idxByEntity;
	vector<BattleTimelineEvent>                    eventQueues;
};

NS_END