#pragma once

NS_BEGIN(Engine)
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
	KEY defend = KEY::Y;
	KEY item   = KEY::B;
	KEY flee   = KEY::A;
	KEY attack = KEY::X;
};
struct SkillKeymap
{
	KEY openHold = KEY::SPACE;
	array<KEY, 4> skillKeys = { KEY::B, KEY::Y, KEY::X, KEY::A };
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
	int                       preBufferCapacity = 1;
	ControllerKeymap          keymap{};
	ControllerTuning          tuning{};
}; 
struct BufferedIntent 
{ 
	bool hasValue = false;
	TimelineActionIntent intent{}; 
};
struct ControllerRuntime
{
	EntityID           leaderEntity = 0u;
	CommandMenuModel   menuModel{};
	CommandMenuRuntime menu{};

	array<bool, 4>     queuedSkillSlotFlags = { false, false, false, false };

	BufferedIntent     buffered{};
	bool               isExecuting = false;
	bool               isDefendingHold = false;
	bool               isEscapeHolding = false;
	double             escapeHoldStartSec = 0.0;

	array<int, 4>  usedStepCountPerSlot = { 0, 0, 0, 0 };
	array<bool, 4> slotLockedThisTurn   = { false, false, false, false };
	int            primarySlotIndexThisTurn = -1;
};

NS_END