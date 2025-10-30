#include "Enginepch.h"

AnimStageSpec ActionAnimRegistry::MakeStage(AnimKey clipKey, ActionStage stage, float fadeDur, bool useRootMotion)
{
	AnimStageSpec spec{};
	spec.clipKey    = clipKey;
	spec.stage      = stage;
	spec.fadeDur    = fadeDur;
	spec.rootMotion = useRootMotion;
	return spec;
}

AnimChainSpec ActionAnimRegistry::MakeChain(initializer_list<AnimStageSpec> stageList)
{
	AnimChainSpec chain{};
	chain.stages.assign(stageList.begin(), stageList.end());
	return chain;
}

ComboSpec ActionAnimRegistry::MakeSingleChainCombo(const AnimChainSpec& chain)
{
	ComboSpec combo{};
	combo.chainByOrder.push_back(chain);
	return combo;
}

void ActionAnimRegistry::RegisterDefaultAnim()
{
	AnimChainSpec basicAttackChain = MakeChain({
		MakeStage(AnimKey::Battle_AttackA, ActionStage::Active),
		MakeStage(AnimKey::Battle_AttackB, ActionStage::Active),
		MakeStage(AnimKey::Battle_AttackC, ActionStage::Active),
		});

	SkillSpec SkillA{};
	SkillA.apCost = 6;
	SkillA.chain  = MakeChain({
		MakeStage(AnimKey::Battle_Skill_1A, ActionStage::Active),
		MakeStage(AnimKey::Battle_Skill_1B, ActionStage::Active),
		MakeStage(AnimKey::Battle_Skill_1C, ActionStage::Active),
		});

	AnimChainSpec introChain = MakeChain({
		MakeStage(AnimKey::Battle_StartA, ActionStage::Active, 0.08f),
		MakeStage(AnimKey::Battle_StartB, ActionStage::Active, 0.08f),
		MakeStage(AnimKey::Battle_StartC, ActionStage::Active, 0.08f)
		});

	ActionAnimSpec spec{};
	spec.basicAttackCombo = MakeSingleChainCombo(basicAttackChain);
	spec.skillByKey.emplace(L"skillA", SkillA);
	spec.introChain = introChain;

	Register(CharacterID::Ryza, spec);
	Register(CharacterID::Kluadia, spec);
	Register(CharacterID::Patricia, spec);
}