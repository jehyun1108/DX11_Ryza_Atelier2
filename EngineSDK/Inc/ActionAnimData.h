#pragma once

NS_BEGIN(Engine)

enum class SpecialAnimTag
{
	Intro, Ultimate, BasicAttack, SkillA, SkillB, SkillC, SkillD,
};

struct AnimStageSpec
{
	AnimKey     clipKey;
	ActionStage stage;
	float       fadeDur    = 0.f;
	bool        rootMotion = false;

	float minOverlapDur = 0.1f;
	float endNormalized = 1.f;
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