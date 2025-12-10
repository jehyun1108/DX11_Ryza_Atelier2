#pragma once

NS_BEGIN(Engine)
// ---------------------------------- Timeline -----------------------------------
enum class TimelineClockState  { Running, Stopped };
enum class TimelineUnitGate    { Closed, Open };
enum class TimelineControlType { Player, Ally, Enemy };
enum class TimelineMotionState { Queued, Preparing, Executing, Finished };
enum class BattleTimelineEventType
{
	FullGauge,
	ActionCommitted,
	ActionFinished,
	TimelinePaused,
	TimelineResumed,
	ApChanged,
	LeaderChanged,
};
struct TimelineRole
{
	TimelineControlType control = TimelineControlType::Player;
	bool allowCombo  = false;
};
struct TimelineTeamAP
{
	int tacticLv = 1;
	int curAp    = 0;
	int maxAp    = 10;
};
struct TimelineGauge
{
	float curValue  = 0.f;
	float maxValue  = 0.f;
	float fillSpeed = 100.f;
	bool  isFrozen  = false;
};
struct TimelineSkillInfo
{
	SpecialAnimTag tag = SpecialAnimTag::BasicAttack;
	int apCost = 0;
};
struct TimelineActionIntent
{
	BattleCommand battleCmd       = BattleCommand::None;
	EntityID      targetEntity    = invalidEntity;
	int           apCost          = 0;
	optional<SpecialAnimTag> specialTag;
};
struct TimelineInputQueue
{
	vector<TimelineActionIntent> pendingCombos;
	bool Empty() const { return pendingCombos.empty(); }
};
struct TimelineUnitPolicy
{
	int  apGainBasicAttack      = 1;
	int  apGainSkillAttack      = 1;
	bool aiPreferSkillAtMaxAp   = true;
	bool aiRandomAmongAfforable = true; 
};
struct TimelineUnitState
{
	EntityID   entity = invalidEntity;
	BattleTeam team   = BattleTeam::Ally;

	TimelineGauge        ATB{};
	TimelineUnitGate     gateState   = TimelineUnitGate::Open;
	TimelineMotionState  motionState = TimelineMotionState::Queued;

	TimelineActionIntent pendingIntent{};
	TimelineActionIntent activeIntent{};

	bool defendAllowed = true;
	bool canAction     = true;
};
struct TimelineUnitRunTime
{
	TimelineRole              role{};
	vector<TimelineSkillInfo> skillCatalog;
	TimelineInputQueue        inputQueue{};
	TimelineUnitPolicy        policy{};
};
struct BattleTimelineConfig
{
	float gaugeMaxValue = 100.f;
	float gaugeFillSpeed = 50.f;
	bool  canAction = true;

	float onHitBackPercent = 0.2f;
	float onHitBackSec = 0.f;
};
struct LeaderState
{
	EntityID curLeader  = 0u;
	EntityID comboOwner = 0u;
};
struct BattleTimelineState
{
	TimelineClockState clockState = TimelineClockState::Running;
	float              elapsedTime = 0.f;

	BattleTimelineConfig config{};
	LeaderState          leader{};

	TimelineTeamAP alliesAp{};
	TimelineTeamAP enemiesAp{};

	array<TimelineUnitState,   3> allies{};
	array<TimelineUnitState,   3> enemies{};
	array<TimelineUnitRunTime, 3> alliesRuntime{};
	array<TimelineUnitRunTime, 3> enemiesRuntime{};
	int alliesUsed  = 0;
	int enemiesUsed = 0;
};

struct BattleTimelineEvent
{
	BattleTimelineEventType eventType     = BattleTimelineEventType::FullGauge;
	EntityID                subjectEntity = 0u;
	BattleTeam              subjectTeam   = BattleTeam::Ally;
	int                     deltaAp       = 0; 
};

NS_END