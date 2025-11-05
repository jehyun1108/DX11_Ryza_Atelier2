#pragma once

NS_BEGIN(Engine)
struct TimelineActionIntent;

enum class CommandMenuPage { Hidden, Primary, Skill };
enum class SubmitPolicy    { QueueOnly, AutoCommitIfReady };

struct CommandMenuModel
{
	bool canDefend = true;
	bool canAttack = true;
	bool canItemRush = true;
	bool canFlee = true;
};

struct CommandMenuRuntime
{
	CommandMenuPage page = CommandMenuPage::Hidden;
};

struct PrimaryKeymap
{
	KEY defend = KEY::RBUTTON;
	KEY item   = KEY::S;
	KEY flee   = KEY::E;
	KEY attack = KEY::LBUTTON;
};

struct SkillKeymap
{
	KEY openHold = KEY::SPACE;
	array<KEY, 4> skillKeys = { KEY::LBUTTON, KEY::RBUTTON, KEY::S, KEY::E };
};

struct ControllerKeymap
{
	PrimaryKeymap primary{};
	SkillKeymap   skill{};
};

struct ControllerTuning
{
	double escapeHoldNeedSec = 1.0;
};

struct ControllerConfig
{
	SubmitPolicy              submitPolicy = SubmitPolicy::AutoCommitIfReady;
	array<SpecialAnimTag, 4>  skillTags = { SpecialAnimTag::SkillA, SpecialAnimTag::SkillB, SpecialAnimTag::SkillC, SpecialAnimTag::SkillD };
	int                       preBufferCapacity = 1;
	ControllerKeymap          keymap{};
	ControllerTuning          tuning{};
}; 

struct BufferedIntent { bool hasValue = false; TimelineActionIntent intent{}; };

struct TurnConstraints
{
	unordered_set<SpecialAnimTag> usedTagsThisTurn;
	void ResetForThisTurn() { usedTagsThisTurn.clear();}
};

struct ControllerRuntime
{
	EntityID leaderEntity = invalidEntity;
	CommandMenuModel   menuModel{};
	CommandMenuRuntime menu{};
	array<bool, 4>  queuedSkillSlotFlags = { false, false, false, false };
	BufferedIntent  buffered{};
	bool            isExecuting = false;
	bool            isDefendingHold = false;
	bool            isEscapeHolding = false;
	double          escapeHoldStartSec = 0.0;
	TurnConstraints turn{};
};

NS_END