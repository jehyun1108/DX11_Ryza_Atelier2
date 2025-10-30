#pragma once

NS_BEGIN(Engine)

enum class BattleBusEventType
{
	// Session
	SessionBegan,
	SessionActivated,
	SessionResultDecided,
	SessionEnded,

	// Session-Intro
	IntroReady,

	// Timeline
	TimelineFullGauge,
	TimelineActionCommitted,
	TimelineActionFinished,
	TimelinePaused,
	TimelineResumed,
	TimelineApChanged,

	// Execution
	ExecutionStageAdvanced,
	ExecutionInterrupted,

	// Resolve
	ResolveDamageApplied,
	UnitDowned,
};

struct EventPayload_None {};

struct EventPayload_SessionPhase
{
	BattlePhase newPhase{};
};

// Timeline
struct EventPayload_ApChanged
{
	int deltaAp = 0;
	int curAp   = 0;
	int maxAp   = 0;
};

struct EventPayload_ActionIntent
{
	TimelineActionIntent intent{};
};

// Execution
struct EventPayload_ExecutionStage
{
	int  curChainIdx  = 0;
	int  curStageIdx  = 0;
	bool isFinalStage = false;
};

// Resolve
struct EventPayload_Damage
{
	EntityID attackerEntity = 0;
	EntityID targetEntity   = 0;
	int      damageAmount   = 0;
	bool     isCritical     = false;
};

struct BattleEvent
{
	BattleBusEventType eventType = BattleBusEventType::SessionBegan;

	EntityID   subjectEntity = 0;
	BattleTeam subjectTeam = BattleTeam::Neutral;

	wstring note{};
	int     frameIdx = -1;

	// Payload
	using PayloadVariant = variant<
		EventPayload_None,
		EventPayload_SessionPhase,
		EventPayload_ApChanged,
		EventPayload_ActionIntent,
		EventPayload_ExecutionStage, 
		EventPayload_Damage>;

	PayloadVariant payload{ EventPayload_None{} };
};

using BattleEventListenerId = _uint;
using BattleEventListener   = function<void(const BattleEvent&)>;

NS_END