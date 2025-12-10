#pragma once

NS_BEGIN(Engine)

enum class BattleTeam
{
	Ally, Enemy, Neutral
};
enum class BattlePhase
{
	Intro, Active, Result, Exit
};
enum class BattleCommand
{
	None, AttackBasic, Skill, Defend, Item, SwapLeader, Escape
};
enum class StatusTag
{
	None  = 0,
	Stop  = 1 << 0,  // 시간 정지
	Stun  = 1 << 1,  // 행동 불가
	Down  = 1 << 2,  // 전투 불능
	Haste = 1 << 3,  // 게이지 가속
	Slow  = 1 << 4,  // 게이지 감속
};
enum class ActionStage
{
	None, Preparation, Active, Recovery
};
inline bool HasStatus(_uint flags, StatusTag tag) { return (flags & static_cast<_uint>(tag)); }

NS_END