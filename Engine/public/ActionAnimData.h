#pragma once

NS_BEGIN(Engine)

struct AnimStageSpec
{
	AnimKey     clipKey;
	ActionStage stage;
	float       fadeDur = 0.f;
	bool        rootMotion = false;
};

struct AnimChainSpec
{
	vector<AnimStageSpec> stages;
};

struct ComboSpec
{
	vector<AnimChainSpec> chainByOrder;
};

struct SkillSpec
{
	AnimChainSpec chain;
	int apCost = 0;
};

struct ActionAnimSpec
{
	ComboSpec basicAttackCombo;
	unordered_map<wstring, SkillSpec> skillByKey;
	optional<AnimChainSpec> introChain;
};

NS_END