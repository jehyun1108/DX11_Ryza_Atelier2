#pragma once

NS_BEGIN(Engine)

enum class BattleBusEventType
{
	None,
	SessionBegan, SessionActivated, SessionResultDecided,SessionEnded, 	// Session
	IntroReady, 	// Session-Intro
	TimelineFullGauge, TimelineActionCommitted, TimelineActionFinished, TimelinePaused, TimelineResumed,	// Timeline
	TimelineApChanged,
	ExecutionStageAdvanced, ExecutionInterrupted,	// Execution
	UnitDowned, ResolveDamageApplied, LeaderChanged, 	// Resolve
	TacticChanged, TacticLevelUp, TacticMaxBlinkOn, TacticMaxBlinkOff	// Tactic
};
struct EventPayload_None {};
struct EventPayload_Tactic
{
	int             level    = 1;
	int             pips     = 0;
	bool            maxBlink = false;
	TacticEventType type     = TacticEventType::Changed;
};
struct EventPayload_SessionPhase
{
	BattlePhase newPhase{};
};
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
struct EventPayload_ExecutionStage
{
	int  curChainIdx  = 0;
	int  curStageIdx  = 0;
	bool isFinalStage = false;
};
struct EventPayload_Damage
{
	EntityID attackerEntity = 0;
	EntityID targetEntity   = 0;
	int      damageAmount   = 0;
	bool     isCritical     = false;
};
struct BattleEvent
{
	BattleBusEventType eventType     = BattleBusEventType::SessionBegan;
	EntityID           subjectEntity = 0;
	BattleTeam         subjectTeam   = BattleTeam::Neutral;
	int                frameIdx      = -1;

	// Payload
	using PayloadVariant = variant< EventPayload_None,
		EventPayload_SessionPhase,
		EventPayload_ApChanged, 
		EventPayload_ActionIntent,
		EventPayload_ExecutionStage,
		EventPayload_Damage,
		EventPayload_Tactic>;

	PayloadVariant payload{ EventPayload_None{} };
};
using BattleEventListener = function<void(const BattleEvent&)>;

NS_END