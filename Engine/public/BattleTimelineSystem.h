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

	//  Setting
	void SetClock(TimelineClockState newState);
	void SetUnitGate(EntityID entity, TimelineUnitGate gate);
	void SetUnitCanAction(EntityID entity, bool canAction);

	// EventQueue
	const vector<BattleTimelineEvent>& PeekEvents() const { return eventQueues; }
	void ClearEvents() { eventQueues.clear(); }

private:
	bool ResolveIdxByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx);
	void PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp = 0, const wstring& note = L"");

	void AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team);
	bool CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, const TimelineActionIntent& intent, EntityID entity, BattleTeam team);
	bool BuildAiIntent(const TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, TimelineActionIntent& outIntent);
	void ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp, const wstring& note);
	void ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team);

private:
	SystemRegistry& registry;
	optional<BattleTimelineState>                  timelineState; // snapShot
	unordered_map<EntityID, pair<BattleTeam, int>> idxByEntity;
	vector<BattleTimelineEvent>                    eventQueues;
};

NS_END