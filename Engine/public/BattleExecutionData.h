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
struct HitCursor
{
	int   stage     = -1;
	int   next      = 0;
	float lastNorm  = 0.f;
	float tagMul    = 1.f;
	int   baseDmg   = 0;
};
struct SfxSequenceRuntime
{
	vector<wstring> keys;
	int             curIdx = -1;
	_uint           curVoiceId = 0;
	bool            active = false;
	float           volume = 0.5f;
};
struct ExecutionUnitRunTime
{
	CharacterID          character = CharacterID::Unknown;
	AnimContext          context   = AnimContext::Battle;
	ExecutionChainCursor cursor    = {};
	TimelineActionIntent activeIntent{};
	SfxSequenceRuntime   sfxSeq;

	enum class Phase
	{
		None, AttackStart, Execute, AttackFinished, DefendStart, Defending, DefendEnd
	} 
	phase = Phase::None;

	vector<SpecialAnimTag> plannedTags;
	int                    plannedIdx = -1;
	MicroMotionPulse       pulse{};
	HitCursor              hit{};

	SpecialAnimTag execTag  = SpecialAnimTag::BasicAttack;
	SpecialAnimTag curTag   = SpecialAnimTag::BasicAttack;
	SpecialAnimTag queuedTag = SpecialAnimTag::BasicAttack;

	float comboEndNorm      = 1.f;
	float comboChainCutNorm = 0.6f;
	bool  comboInputOpen    = false;
	bool  hasQueuedCombo    = false;
};

NS_END