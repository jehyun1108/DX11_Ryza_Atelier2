#pragma once

NS_BEGIN(Engine)

struct ExecutionChainCursor
{
	int  curChainIdx = 0;
	int  curStageIdx = 0;
	bool isActive    = false;
};

struct MicroMotionPulse
{
	_float2 dirXZ      = {};
	float   remainDist = 0.f;
	float   speed      = 20.f;
	bool    active     = false;
};

struct ExecutionUnitRunTime
{
	CharacterID          character = CharacterID::Unknown;
	AnimContext          context   = AnimContext::Battle;
	ExecutionChainCursor cursor    = {};
	TimelineActionIntent activeIntent{};

	enum class Phase { None, AttackStart, Execute, AttackFinished, DefendStart, Defending, DefendEnd } phase = Phase::None;

	vector<SpecialAnimTag> plannedTags;
	int                    plannedIdx = -1;

	MicroMotionPulse       pulse{};
};

NS_END