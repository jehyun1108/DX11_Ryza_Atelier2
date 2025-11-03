#pragma once

NS_BEGIN(Engine)

enum class SpecialAnimTag
{
	Intro, Ultimate, BasicAttack, SkillA, SkillB, SkillC, SkillD, ItemRush, AttackFinished, AttackStarted, DefendStart, Defending, DefendEnd
};

struct AnimStageSpec
{
	AnimKey     clipKey;
	ActionStage stage;
	float       fadeDur    = 0.f;
	bool        rootMotion = false;

	float minOverlapDur = 0.05f;

	optional<float> startNormalizedOverride; 
	optional<float> endNormalizedOverride;   
	optional<float> playbackSpeedOverride;
};

struct AnimChainSpec
{
	vector<AnimStageSpec> stages;
};

struct ActionAnimSpec
{
	unordered_map<SpecialAnimTag, AnimChainSpec> specials;
	unordered_map<SpecialAnimTag, int>           apCostByTag;
};

NS_END