#pragma once

NS_BEGIN(Engine)
struct TimelineActionIntent;

enum class ControllerMode
{
	Idle, BasicAttack, Skill, Defend, Escape, SwapLeader
};

enum class SubmitPolicy
{
	QueueOnly, AutoCommitIfReady 
};

enum class PrimaryCommand
{
	None, BasicAttack, Defend, Escape
};

struct PrimaryCommandBindings
{
	KEY basicAttackKey   = KEY::LBUTTON;
	KEY defendKey        = KEY::RBUTTON;
	KEY escapeKey        = KEY::E;
	KEY openSkillMenuKey = KEY::SPACE;
};

struct QuickSkillBindings
{
	KEY key    = KEY::W;
	SpecialAnimTag tag = SpecialAnimTag::SkillA;
	int apCost = 0;
};

struct ControllerConfig
{
	SubmitPolicy                 submitPolicy = SubmitPolicy::AutoCommitIfReady;
	PrimaryCommandBindings       primary{};
	array<QuickSkillBindings, 4> quickSkills = { {
		{ KEY::W, SpecialAnimTag::SkillA, 0 },
		{ KEY::A, SpecialAnimTag::SkillB, 0 },
		{ KEY::S, SpecialAnimTag::SkillC, 0 },
		{ KEY::D, SpecialAnimTag::SkillD, 0 },
		} };
	int   preBufferCapacity   = 1;
}; 

struct BufferedIntent
{
	bool hasValue = false;
	TimelineActionIntent intent{};
};

struct TurnConstraints
{
	unordered_set<SpecialAnimTag> usedTagsThisTurn;

	void ResetForThisTurn()
	{
		usedTagsThisTurn.clear();
	}
};

struct ControllerRuntime
{
	EntityID leaderEntity           = invalidEntity;
	ControllerMode  mode            = ControllerMode::Idle;
	PrimaryCommand  selectedPrimary = PrimaryCommand::None;

	array<bool, 4> queuedSkillSlotFlags = { false, false, false, false };

	BufferedIntent  buffered{};
	bool            isExecuting = false;
	bool            isDefendingHold = false;
	TurnConstraints turn{};
};

NS_END