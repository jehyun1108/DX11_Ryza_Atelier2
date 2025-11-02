#include "Enginepch.h"

static constexpr float kFadeDur = 0.05f;

const ActionAnimSpec* ActionAnimRegistry::TryGet(CharacterID character) const
{
	auto it = table.find(character);
	return (it == table.end()) ? nullptr : &it->second;
}

const AnimChainSpec* ActionAnimRegistry::TryGetSpecial(CharacterID character, SpecialAnimTag tag) const
{
	auto itChar = table.find(character);
	if (itChar == table.end()) return nullptr;
	const auto& spec = itChar->second;
	auto it = spec.specials.find(tag);
	return (it == spec.specials.end()) ? nullptr : &it->second;
}


void ActionAnimRegistry::RegisterSpecial(CharacterID character, SpecialAnimTag tag, const vector<AnimKey>& animKeys, ActionStage stageForAll, bool rootMotionForAll, float fadeDur, int apCost)
{
	ActionAnimSpec& spec  = table[character];
	spec.specials[tag]    = MakeChainSimple(animKeys, stageForAll, rootMotionForAll, fadeDur);
	spec.apCostByTag[tag] = apCost;
}

AnimStageSpec ActionAnimRegistry::MakeStage(AnimKey clipKey, ActionStage stage, float fadeDur, bool useRootMotion)
{
	AnimStageSpec spec{};
	spec.clipKey = clipKey;
	spec.stage = stage;
	spec.fadeDur = fadeDur;
	spec.rootMotion = useRootMotion;
	return spec;
}

AnimChainSpec ActionAnimRegistry::MakeChainSimple(const vector<AnimKey>& keys, ActionStage stageForAll, bool rootMotionForAll, float fadeDur)
{
	AnimChainSpec chain{};
	chain.stages.reserve(keys.size());
	for (AnimKey key : keys)
		chain.stages.push_back(MakeStage(key, stageForAll, fadeDur, rootMotionForAll));
	return chain;
}

void ActionAnimRegistry::RegisterDefaultAnim()
{
	RegisterRyzaAnim();
	RegisterPatriciaAnim();
	RegisterKlaudiaAnim();
	RegisterAngelAnim();
}

void ActionAnimRegistry::RegisterPatriciaAnim()
{
	// 1. BasicAttack 
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::BasicAttack, { AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC },
		ActionStage::Active, true, kFadeDur);

	// 2. Skill
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::SkillA, { AnimKey::Battle_Skill_A1, AnimKey::Battle_Skill_A2, AnimKey::Battle_Skill_A3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::SkillB, { AnimKey::Battle_Skill_B1, AnimKey::Battle_Skill_B2, AnimKey::Battle_Skill_B3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::SkillC, { AnimKey::Battle_Skill_C1, AnimKey::Battle_Skill_C2, AnimKey::Battle_Skill_C3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::SkillD, { AnimKey::Battle_Skill_D1, AnimKey::Battle_Skill_D2, AnimKey::Battle_Skill_D3 },
		ActionStage::Active, true, kFadeDur);

	// 3. FetalDrive
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::Ultimate,
		{ AnimKey::Battle_Ultimate_Patricia1,   AnimKey::Battle_Ultimate_Patricia2, AnimKey::Battle_Ultimate_Patricia3,  
		  AnimKey::Battle_Ultimate_Patricia4,   AnimKey::Battle_Ultimate_Patricia5, AnimKey::Battle_Ultimate_Patricia6,
	      AnimKey::Battle_Ultimate_Patricia7 },
		ActionStage::Active, true, kFadeDur);

	// 4. Intro
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::Intro, { AnimKey::Battle_StartA, AnimKey::Battle_StartB, AnimKey::Battle_StartC },
		ActionStage::Active, true, kFadeDur);

	// 5. AttackFinished
	RegisterSpecial(CharacterID::Patricia, SpecialAnimTag::AttackFinished, { AnimKey::Battle_Attack_FinishedA, AnimKey::Battle_Attack_FinishedB, AnimKey::Battle_Attack_FinishedC }, ActionStage::Active, true, kFadeDur);
}

void ActionAnimRegistry::RegisterRyzaAnim()
{
	// 1) Basic 
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::BasicAttack, { AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC },
		ActionStage::Active, true, kFadeDur, 0);

	// 2) Skill 
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::SkillA, { AnimKey::Battle_Skill_A1, AnimKey::Battle_Skill_A2, AnimKey::Battle_Skill_A3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::SkillB, { AnimKey::Battle_Skill_B1, AnimKey::Battle_Skill_B2, AnimKey::Battle_Skill_B3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::SkillC, { AnimKey::Battle_Skill_C1, AnimKey::Battle_Skill_C2, AnimKey::Battle_Skill_C3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::SkillD, { AnimKey::Battle_Skill_D1, AnimKey::Battle_Skill_D2, AnimKey::Battle_Skill_D3 },
		ActionStage::Active, true, kFadeDur);

	// 3) Ultimate
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::Ultimate,
		{ AnimKey::Battle_Ultimate_Ryza1, AnimKey::Battle_Ultimate_Ryza2, AnimKey::Battle_Ultimate_Ryza3, 
		  AnimKey::Battle_Ultimate_Ryza4, AnimKey::Battle_Ultimate_Ryza5 },
		  ActionStage::Active, true, kFadeDur);

	// 4) Intro
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::Intro, 
		{ AnimKey::Battle_StartA, AnimKey::Battle_StartB, AnimKey::Battle_StartC }, ActionStage::Active,  true, kFadeDur);

	// 5. AttackFinished
	RegisterSpecial(CharacterID::Ryza, SpecialAnimTag::AttackFinished, { AnimKey::Battle_Attack_FinishedA, AnimKey::Battle_Attack_FinishedB, AnimKey::Battle_Attack_FinishedC }, ActionStage::Active, true, kFadeDur);
}

void ActionAnimRegistry::RegisterKlaudiaAnim()
{
	// 1) Basic
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::BasicAttack, {AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC},
		ActionStage::Active, true, kFadeDur, 0);

	// 2) Skill 
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::SkillA, { AnimKey::Battle_Skill_A1, AnimKey::Battle_Skill_A2, AnimKey::Battle_Skill_A3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::SkillB, { AnimKey::Battle_Skill_B1, AnimKey::Battle_Skill_B2, AnimKey::Battle_Skill_B3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::SkillC, { AnimKey::Battle_Skill_C1, AnimKey::Battle_Skill_C2, AnimKey::Battle_Skill_C3 },
		ActionStage::Active, true, kFadeDur);
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::SkillD, { AnimKey::Battle_Skill_D1, AnimKey::Battle_Skill_D2, AnimKey::Battle_Skill_D3 },
		ActionStage::Active, true, kFadeDur);

	// 3) Ultimate
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::Ultimate,
		{ AnimKey::Battle_Ultimate_Klaudia1, AnimKey::Battle_Ultimate_Klaudia2, AnimKey::Battle_Ultimate_Klaudia3, AnimKey::Battle_Ultimate_Klaudia4,
		  AnimKey::Battle_Ultimate_Klaudia5, AnimKey::Battle_Ultimate_Klaudia6, AnimKey::Battle_Ultimate_Klaudia7, AnimKey::Battle_Ultimate_Klaudia8,
		  AnimKey::Battle_Ultimate_Klaudia9 },
		  ActionStage::Active, true, kFadeDur);

	// 4) Intro
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::Intro,  
		{ AnimKey::Battle_StartA, AnimKey::Battle_StartB, AnimKey::Battle_StartC }, ActionStage::Active, true, kFadeDur);

	// 5. AttackFinished
	RegisterSpecial(CharacterID::Klaudia, SpecialAnimTag::AttackFinished, { AnimKey::Battle_Attack_FinishedA, AnimKey::Battle_Attack_FinishedB, AnimKey::Battle_Attack_FinishedC }, ActionStage::Active, true, kFadeDur);
}

void ActionAnimRegistry::RegisterAngelAnim()
{
	RegisterSpecial(CharacterID::Angel, SpecialAnimTag::BasicAttack, { AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC },
		ActionStage::Active, true, kFadeDur, 0);

	RegisterSpecial(CharacterID::Angel, SpecialAnimTag::SkillA, { AnimKey::Battle_Skill_A1, AnimKey::Battle_Skill_A2, AnimKey::Battle_Skill_A3 },
		ActionStage::Active, true, kFadeDur);

	//RegisterSpecial(CharacterID::Angel, SpecialAnimTag::Intro, { AnimKey::Battle_StartA }, ActionStage::Preparation,  true, 0.06f);
}