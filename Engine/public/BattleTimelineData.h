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
};

struct TimelineRole
{
	TimelineControlType control = TimelineControlType::Player;
	bool allowCombo  = false;
};

struct TimelineAP
{
	int tacticLv = 1;
	int curAp    = 0;
	int maxAp    = 10;
};

struct TimelineGauge
{
	float curValue  = 0.f;
	float maxValue  = 0.f;
	float fillSpeed = 1000.f;
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

// Player 전용
struct TimelineInputQueue
{
	vector<TimelineActionIntent> pendingCombos;
	bool Empty() const { return pendingCombos.empty(); }
};

struct TimelineUnitPolicy
{
	int apGainBasicAttack = 1;
	int apGainSkillAttack = 1;
	bool aiPreferSkillAtMaxAp   = true;
	bool aiRandomAmongAfforable = true; // 가능한 후보중랜덤
};

struct TimelineUnitState
{
	EntityID   entity = invalidEntity;
	BattleTeam team   = BattleTeam::Ally;

	TimelineAP           ap{};
	TimelineGauge        ATB{};
	TimelineUnitGate     gateState   = TimelineUnitGate::Open;
	TimelineMotionState  motionState = TimelineMotionState::Queued;

	// 현재/대기 액션
	TimelineActionIntent pendingIntent{};
	TimelineActionIntent activeIntent{};

	bool canAction = true;
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
	float gaugeFillSpeed = 100.f;
	bool  canAction = true;
};

struct TimelineLeaderState
{
	EntityID curLeader = invalidEntity;
	bool     swapApOnLeaderChange = true;
};

struct BattleTimelineState
{
	TimelineClockState clockState = TimelineClockState::Running;
	float              elapsedTime = 0.f;

	BattleTimelineConfig config{};
	TimelineLeaderState  leader{};

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
	EntityID                subjectEntity = invalidEntity;
	BattleTeam              subjectTeam   = BattleTeam::Ally;
	int                     deltaAp       = 0; // 상승 차감 알림
	wstring                 note; // 디버그용
};

NS_END