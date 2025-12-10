#include "Enginepch.h"

static constexpr float kFadeDur = 0.2f;

void ActionAnimRegistry::OnBoot()
{
	constexpr float kInputNorm = 0.5f;
	constexpr float kChainCutNorm = 0.6f;

	array<SkillStepInfo, 12> ryzaSteps =
	{ {
		{ SkillSlotTag::A, 0, SpecialAnimTag::SkillA_1, kInputNorm, kChainCutNorm, 0 }, // KEY::B
		{ SkillSlotTag::A, 1, SpecialAnimTag::SkillA_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::A, 2, SpecialAnimTag::SkillA_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::B, 0, SpecialAnimTag::SkillB_1, kInputNorm, kChainCutNorm, 0 }, // KEY::Y
		{ SkillSlotTag::B, 1, SpecialAnimTag::SkillB_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::B, 2, SpecialAnimTag::SkillB_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::C, 0, SpecialAnimTag::SkillC_1, kInputNorm, kChainCutNorm, 0 }, // KEY::X
		{ SkillSlotTag::C, 1, SpecialAnimTag::SkillC_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::C, 2, SpecialAnimTag::SkillC_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::D, 0, SpecialAnimTag::SkillD_1, kInputNorm, kChainCutNorm, 0 }, // KEY::A
		{ SkillSlotTag::D, 1, SpecialAnimTag::SkillD_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::D, 2, SpecialAnimTag::SkillD_3, kInputNorm, kChainCutNorm, 0 },
	} };																			
	SetSkillSteps(CharacterID::Ryza, ryzaSteps);									
																					
	array<SkillStepInfo, 12> klaudiaSteps =											
	{ {																				
		{ SkillSlotTag::A, 0, SpecialAnimTag::SkillA_1, kInputNorm, kChainCutNorm, 0 }, // KEY::B
		{ SkillSlotTag::A, 1, SpecialAnimTag::SkillA_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::A, 2, SpecialAnimTag::SkillA_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::B, 0, SpecialAnimTag::SkillB_1, kInputNorm, kChainCutNorm, 0 }, // KEY::Y
		{ SkillSlotTag::B, 1, SpecialAnimTag::SkillB_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::B, 2, SpecialAnimTag::SkillB_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::C, 0, SpecialAnimTag::SkillC_1, kInputNorm, kChainCutNorm, 0 }, // KEY::X
		{ SkillSlotTag::C, 1, SpecialAnimTag::SkillC_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::C, 2, SpecialAnimTag::SkillC_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::D, 0, SpecialAnimTag::SkillD_1, kInputNorm, kChainCutNorm, 0 }, // KEY::A
		{ SkillSlotTag::D, 1, SpecialAnimTag::SkillD_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::D, 2, SpecialAnimTag::SkillD_3, kInputNorm, kChainCutNorm, 0 },
		} };																		
	SetSkillSteps(CharacterID::Klaudia, klaudiaSteps);								
																					
	array<SkillStepInfo, 12> patriciaSteps =										
	{ {																				
		{ SkillSlotTag::A, 0, SpecialAnimTag::SkillA_1, kInputNorm, kChainCutNorm, 0 }, // KEY::B
		{ SkillSlotTag::A, 1, SpecialAnimTag::SkillA_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::A, 2, SpecialAnimTag::SkillA_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::B, 0, SpecialAnimTag::SkillB_1, kInputNorm, kChainCutNorm, 0 }, // KEY::Y
		{ SkillSlotTag::B, 1, SpecialAnimTag::SkillB_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::B, 2, SpecialAnimTag::SkillB_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::C, 0, SpecialAnimTag::SkillC_1, kInputNorm, kChainCutNorm, 0 }, // KEY::X
		{ SkillSlotTag::C, 1, SpecialAnimTag::SkillC_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::C, 2, SpecialAnimTag::SkillC_3, kInputNorm, kChainCutNorm, 0 },
																					
		{ SkillSlotTag::D, 0, SpecialAnimTag::SkillD_1, kInputNorm, kChainCutNorm, 0 }, // KEY::A
		{ SkillSlotTag::D, 1, SpecialAnimTag::SkillD_2, kInputNorm, kChainCutNorm, 0 },
		{ SkillSlotTag::D, 2, SpecialAnimTag::SkillD_3, kInputNorm, kChainCutNorm, 0 },
		} };
	SetSkillSteps(CharacterID::Patricia, patriciaSteps);

	RegisterRyzaAnim();
	RegisterPatriciaAnim();
	RegisterKlaudiaAnim();
	RegisterAngelAnim();
	RegisterRyzaHits();
}

const ActionAnimSpec& ActionAnimRegistry::Get(CharacterID id) const
{
	auto it = table.find(id);
	return it->second;
}

const AnimChainSpec& ActionAnimRegistry::GetSpecial(CharacterID id, SpecialAnimTag tag) const
{
	auto itChar = table.find(id);
	const auto& spec = itChar->second;
	auto it = spec.specials.find(tag);
	return it->second;
}

SpecialAnimTag ActionAnimRegistry::GetStepTag(CharacterID id, SkillSlotTag slot, int stepIdx) const
{
	auto it = skillStepsByChar.find(id);
	assert(it != skillStepsByChar.end());

	const auto& steps = it->second;

	for (const auto& info : steps)
	{
		if (info.slot == slot && info.stepIdx == stepIdx)
			return info.tag;
	}

	assert(false && "GetStepTag: invalid slot/stepIdx for this character");
	return SpecialAnimTag::Intro;
}

const SkillStepInfo& ActionAnimRegistry::GetStepInfo(CharacterID id, SpecialAnimTag tag) const
{
	auto it = skillStepsByChar.find(id);
	assert(it != skillStepsByChar.end());

	const auto& steps = it->second;

	for (const auto& info : steps)
	{
		if (info.tag == tag)
			return info;
	}

	assert(false && "GetStepInfo: unknown SpecialAnimTag for this character");
	return steps[0];
}

void ActionAnimRegistry::RegisterSpecial(CharacterID id, SpecialAnimTag tag, const vector<AnimKey>& animKeys, ActionStage stageForAll, bool rootMotionForAll, float fadeDur, int apCost)
{
	ActionAnimSpec& spec = table[id];
	spec.specials[tag] = MakeChainSimple(animKeys, stageForAll, rootMotionForAll, fadeDur);
	spec.apCostByTag[tag] = apCost;
	if (!spec.dmgMulByTag.count(tag)) spec.dmgMulByTag[tag] = 1.f; 
}

void ActionAnimRegistry::SetTagdmgMul(CharacterID id, SpecialAnimTag tag, float mul)
{
	ActionAnimSpec& spec = table[id];
	spec.dmgMulByTag[tag] = mul;
}

void ActionAnimRegistry::SetStageHits(CharacterID id, SpecialAnimTag tag, int stageIdx, const vector<HitPoint>& hits)
{
	auto itChar = table.find(id);
	ActionAnimSpec& aspec = itChar->second;
	auto it = aspec.specials.find(tag);
	AnimChainSpec& chain = it->second;
	chain.stages[(size_t)stageIdx].hits = hits;
}

void ActionAnimRegistry::SetChainHits(CharacterID id, SpecialAnimTag tag, const vector<vector<HitPoint>>& hitsByStage)
{
	auto itChar = table.find(id);
	ActionAnimSpec& aspec = itChar->second;
	auto it = aspec.specials.find(tag);
	AnimChainSpec& chain = it->second;
	for (size_t i = 0; i < hitsByStage.size(); ++i)
		chain.stages[i].hits = hitsByStage[i];
}

void ActionAnimRegistry::SetSfxSequence(CharacterID id, SpecialAnimTag tag, const vector<wstring>& keys)
{
	ActionAnimSpec& spec = table[id];
	spec.sfxSeqByTag[tag] = keys;
}

AnimStageSpec ActionAnimRegistry::MakeStage(AnimKey clipKey, ActionStage stage, float fadeDur, bool useRootMotion)
{
	AnimStageSpec spec{};
	spec.clipKey    = clipKey;
	spec.stage      = stage;
	spec.fadeDur    = fadeDur;
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

void ActionAnimRegistry::RegisterPatriciaAnim()
{
	auto patricia = CharacterID::Patricia;

	RegisterSpecial(patricia, SpecialAnimTag::BasicAttack,
		{ AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC });
	SetTagdmgMul(patricia, SpecialAnimTag::BasicAttack, 1.f);
	SetChainHits(patricia, SpecialAnimTag::BasicAttack,
		{ { { 0.3f, 1.1f } }, { { 0.3f, 0.9f } }, { { 0.3f, 1.2f } }, });
	SetSfxSequence(patricia, SpecialAnimTag::BasicAttack,
		{ L"patricia_17", L"patricia_32", L"patricia_9" });

	struct Row
	{
		SpecialAnimTag   tag;
		AnimKey          anim;
		const wchar_t* sfx;
	};

	const Row rows[] =
	{
		{ SpecialAnimTag::SkillA_1, AnimKey::Battle_Skill_A1, L"patricia_34" },
		{ SpecialAnimTag::SkillA_2, AnimKey::Battle_Skill_A2, L"patricia_13" },
		{ SpecialAnimTag::SkillA_3, AnimKey::Battle_Skill_A3, L"patricia_12" },

		{ SpecialAnimTag::SkillB_1, AnimKey::Battle_Skill_B1, L"patricia_23" },
		{ SpecialAnimTag::SkillB_2, AnimKey::Battle_Skill_B2, L"patricia_11" },
		{ SpecialAnimTag::SkillB_3, AnimKey::Battle_Skill_B3, L"patricia_39" },

		{ SpecialAnimTag::SkillC_1, AnimKey::Battle_Skill_C1, L"patricia_26" },
		{ SpecialAnimTag::SkillC_2, AnimKey::Battle_Skill_C2, L"patricia_21" },
		{ SpecialAnimTag::SkillC_3, AnimKey::Battle_Skill_C3, L"patricia_18" },

		{ SpecialAnimTag::SkillD_1, AnimKey::Battle_Skill_D1, L"patricia_3" },
		{ SpecialAnimTag::SkillD_2, AnimKey::Battle_Skill_D2, L"patricia_33" },
		{ SpecialAnimTag::SkillD_3, AnimKey::Battle_Skill_D3, L"patricia_2" },
	};

	for (const Row& r : rows)
	{
		RegisterSpecial(patricia, r.tag, { r.anim });

		SetTagdmgMul(patricia, r.tag, 1.0f); // 스킬은 일단 전부 1배

		if (r.sfx)
			SetSfxSequence(patricia, r.tag, { r.sfx });

		HitPoint hp{};
		hp.timeNorm = 0.5f;  // 0.5에서 히트
		hp.dmgRatio = 1.0f;  // 1배

		vector<vector<HitPoint>> hits(1);
		hits[0].push_back(hp);

		SetChainHits(patricia, r.tag, hits);
	}

	RegisterSpecial(patricia, SpecialAnimTag::Ultimate,
		{ AnimKey::Battle_Ultimate_Patricia1, AnimKey::Battle_Ultimate_Patricia2,
		AnimKey::Battle_Ultimate_Patricia3, AnimKey::Battle_Ultimate_Patricia4,
		AnimKey::Battle_Ultimate_Patricia5, AnimKey::Battle_Ultimate_Patricia6,
		AnimKey::Battle_Ultimate_Patricia7 });

	RegisterSpecial(patricia, SpecialAnimTag::Intro,
		{ AnimKey::Battle_StartA, AnimKey::Battle_StartB, AnimKey::Battle_StartC });

	RegisterSpecial(patricia, SpecialAnimTag::AttackFinished,
		{ AnimKey::Battle_Attack_FinishedA, AnimKey::Battle_Attack_FinishedB, AnimKey::Battle_Attack_FinishedC });

	RegisterSpecial(patricia, SpecialAnimTag::AttackStarted,
		{ AnimKey::Battle_RunLoop, AnimKey::Battle_RunEnd });

	RegisterSpecial(patricia, SpecialAnimTag::DefendStart, { AnimKey::Battle_Defend_Ready });
	RegisterSpecial(patricia, SpecialAnimTag::Defending, { AnimKey::Battle_Defending });
	RegisterSpecial(patricia, SpecialAnimTag::DefendEnd, { AnimKey::Battle_Defend_Finished });
}

void ActionAnimRegistry::RegisterRyzaAnim()
{
	auto ryza = CharacterID::Ryza;

	RegisterSpecial(ryza, SpecialAnimTag::BasicAttack,
		{
			AnimKey::Battle_AttackA,
			AnimKey::Battle_AttackB,
			AnimKey::Battle_AttackC
		});

	SetTagdmgMul(ryza, SpecialAnimTag::BasicAttack, 1.0f);
	SetChainHits(ryza, SpecialAnimTag::BasicAttack,
		{
			{ { 0.4f, 1.1f } }, // stage 0
		    { { 0.4f, 0.9f } }, // stage 1
		    { { 0.9f, 1.2f } }, // stage 2
		});

	SetSfxSequence(ryza, SpecialAnimTag::BasicAttack, { L"ryza_4", L"ryza_22" });


	struct Row
	{
		SpecialAnimTag tag;
		AnimKey        anim;
		const wchar_t* sfx;
		float          dmgMul;

		bool           hasHit;
		float          hitTimeNorm;
		float          hitRatio;
	};

	const Row rows[] =
	{
		// Skill A
		{ SpecialAnimTag::SkillA_1, AnimKey::Battle_Skill_A1, L"ryza_5",  1.2f, true, 0.3f, 1.0f },
		{ SpecialAnimTag::SkillA_2, AnimKey::Battle_Skill_A2, L"ryza_29", 1.3f, true, 0.3f, 1.1f },
		{ SpecialAnimTag::SkillA_3, AnimKey::Battle_Skill_A3, L"ryza_7",  1.5f, true, 0.3f, 1.3f },

		// Skill B
		{ SpecialAnimTag::SkillB_1, AnimKey::Battle_Skill_B1, L"ryza_18", 1.1f, true, 0.3f, 1.0f },
		{ SpecialAnimTag::SkillB_2, AnimKey::Battle_Skill_B2, L"ryza_31", 1.2f, true, 0.3f, 1.1f },
		{ SpecialAnimTag::SkillB_3, AnimKey::Battle_Skill_B3, L"ryza_32", 1.4f, true, 0.3f, 1.3f },

		// Skill C
		{ SpecialAnimTag::SkillC_1, AnimKey::Battle_Skill_C1, L"ryza_20", 1.0f, true, 0.3f, 1.0f },
		{ SpecialAnimTag::SkillC_2, AnimKey::Battle_Skill_C2, L"ryza_14", 1.1f, true, 0.3f, 1.1f },
		{ SpecialAnimTag::SkillC_3, AnimKey::Battle_Skill_C3, L"ryza_33", 1.3f, true, 0.3f, 1.3f },

		// Skill D
		{ SpecialAnimTag::SkillD_1, AnimKey::Battle_Skill_D1, L"ryza_17", 1.0f, true, 0.3f, 1.0f },
		{ SpecialAnimTag::SkillD_2, AnimKey::Battle_Skill_D2, L"ryza_35", 1.2f, true, 0.3f, 1.2f },
		{ SpecialAnimTag::SkillD_3, AnimKey::Battle_Skill_D3, L"ryza_43", 1.5f, true, 0.3f, 1.5f },
	};

	for (const Row& r : rows)
	{
		RegisterSpecial(ryza, r.tag, { r.anim });
		SetTagdmgMul(ryza, r.tag, r.dmgMul);

		if (r.sfx)
			SetSfxSequence(ryza, r.tag, { r.sfx });

		if (r.hasHit)
		{
			HitPoint hp{};
			hp.timeNorm = r.hitTimeNorm;
			hp.dmgRatio = r.hitRatio;  

			vector<vector<HitPoint>> hits(1);
			hits[0].push_back(hp);   

			SetChainHits(ryza, r.tag, hits);
		}
	}

	// 3) Ultimate
	RegisterSpecial(ryza, SpecialAnimTag::Ultimate,
		{
			AnimKey::Battle_Ultimate_Ryza1,
			AnimKey::Battle_Ultimate_Ryza2,
			AnimKey::Battle_Ultimate_Ryza3,
			AnimKey::Battle_Ultimate_Ryza4,
			AnimKey::Battle_Ultimate_Ryza5
		});

	// 4) Intro
	RegisterSpecial(ryza, SpecialAnimTag::Intro, { AnimKey::Battle_StartA, AnimKey::Battle_StartB, AnimKey::Battle_StartC });

	// 5) AttackFinished
	RegisterSpecial(ryza, SpecialAnimTag::AttackFinished, { AnimKey::Battle_Attack_FinishedA, AnimKey::Battle_Attack_FinishedB, AnimKey::Battle_Attack_FinishedC });

	// 6) AttackStarted
	RegisterSpecial(ryza, SpecialAnimTag::AttackStarted, { AnimKey::Battle_RunLoop, AnimKey::Battle_RunEnd });

	// 7) Defend
	RegisterSpecial(ryza, SpecialAnimTag::DefendStart, { AnimKey::Battle_Defend_Ready });
	RegisterSpecial(ryza, SpecialAnimTag::Defending, { AnimKey::Battle_Defending });
	RegisterSpecial(ryza, SpecialAnimTag::DefendEnd, { AnimKey::Battle_Defend_Finished });
}

void ActionAnimRegistry::RegisterKlaudiaAnim()
{
	auto klaudia = CharacterID::Klaudia;

	RegisterSpecial(klaudia, SpecialAnimTag::BasicAttack,
		{ AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC });
	SetTagdmgMul(klaudia, SpecialAnimTag::BasicAttack, 1.2f);
	SetChainHits(klaudia, SpecialAnimTag::BasicAttack,
		{ { { 0.3f, 1.1f } }, { { 0.3f, 0.9f } }, { { 0.3f, 1.2f } }, });
	SetSfxSequence(klaudia, SpecialAnimTag::BasicAttack,
		{ L"klaudia_10", L"klaudia_28" });

	struct Row
	{
		SpecialAnimTag   tag;
		AnimKey          anim;
		const wchar_t* sfx;
	};

	const Row rows[] =
	{
		{ SpecialAnimTag::SkillA_1, AnimKey::Battle_Skill_A1, L"klaudia_11" },
		{ SpecialAnimTag::SkillA_2, AnimKey::Battle_Skill_A2, L"klaudia_5" },
		{ SpecialAnimTag::SkillA_3, AnimKey::Battle_Skill_A3, L"klaudia_35" },

		{ SpecialAnimTag::SkillB_1, AnimKey::Battle_Skill_B1, L"klaudia_24" },
		{ SpecialAnimTag::SkillB_2, AnimKey::Battle_Skill_B2, L"klaudia_7" },
		{ SpecialAnimTag::SkillB_3, AnimKey::Battle_Skill_B3, L"klaudia_2" },

		{ SpecialAnimTag::SkillC_1, AnimKey::Battle_Skill_C1, L"klaudia_1" },
		{ SpecialAnimTag::SkillC_2, AnimKey::Battle_Skill_C2, L"klaudia_15" },
		{ SpecialAnimTag::SkillC_3, AnimKey::Battle_Skill_C3, L"klaudia_38" },

		{ SpecialAnimTag::SkillD_1, AnimKey::Battle_Skill_D1, L"klaudia_20" },
		{ SpecialAnimTag::SkillD_2, AnimKey::Battle_Skill_D2, L"klaudia_26" },
		{ SpecialAnimTag::SkillD_3, AnimKey::Battle_Skill_D3, L"klaudia_32" },
	};

	for (const Row& r : rows)
	{
		RegisterSpecial(klaudia, r.tag, { r.anim });

		SetTagdmgMul(klaudia, r.tag, 1.0f); // 스킬은 전부 1배

		if (r.sfx)
			SetSfxSequence(klaudia, r.tag, { r.sfx });

		HitPoint hp{};
		hp.timeNorm = 0.5f;
		hp.dmgRatio = 1.0f;

		vector<vector<HitPoint>> hits(1);
		hits[0].push_back(hp);

		SetChainHits(klaudia, r.tag, hits);
	}

	RegisterSpecial(klaudia, SpecialAnimTag::Ultimate,
		{ AnimKey::Battle_Ultimate_Klaudia1, AnimKey::Battle_Ultimate_Klaudia2,
		AnimKey::Battle_Ultimate_Klaudia3, AnimKey::Battle_Ultimate_Klaudia4,
		AnimKey::Battle_Ultimate_Klaudia5, AnimKey::Battle_Ultimate_Klaudia6,
		AnimKey::Battle_Ultimate_Klaudia7, AnimKey::Battle_Ultimate_Klaudia8,
		AnimKey::Battle_Ultimate_Klaudia9 });

	RegisterSpecial(klaudia, SpecialAnimTag::Intro,
		{ AnimKey::Battle_StartA, AnimKey::Battle_StartB, AnimKey::Battle_StartC });

	RegisterSpecial(klaudia, SpecialAnimTag::AttackFinished,
		{ AnimKey::Battle_Attack_FinishedA, AnimKey::Battle_Attack_FinishedB, AnimKey::Battle_Attack_FinishedC });

	RegisterSpecial(klaudia, SpecialAnimTag::AttackStarted,
		{ AnimKey::Battle_RunLoop, AnimKey::Battle_RunEnd });

	RegisterSpecial(klaudia, SpecialAnimTag::DefendStart, { AnimKey::Battle_Defend_Ready });
	RegisterSpecial(klaudia, SpecialAnimTag::Defending, { AnimKey::Battle_Defending });
	RegisterSpecial(klaudia, SpecialAnimTag::DefendEnd, { AnimKey::Battle_Defend_Finished });
}

void ActionAnimRegistry::RegisterAngelAnim()
{
	auto angel = CharacterID::Angel;
	RegisterSpecial(angel, SpecialAnimTag::BasicAttack, { AnimKey::Battle_AttackA });
	SetTagdmgMul(angel, SpecialAnimTag::BasicAttack, 1.f);
	SetChainHits(angel, SpecialAnimTag::BasicAttack, {{ { 0.3f, 1.f } },});

	//RegisterSpecial(CharacterID::Angel, SpecialAnimTag::SkillA, { AnimKey::Battle_Skill_A1, AnimKey::Battle_Skill_A2, AnimKey::Battle_Skill_A3 });
	//RegisterSpecial(CharacterID::Angel, SpecialAnimTag::Intro, { AnimKey::Battle_StartA }, ActionStage::Preparation,  true, 0.06f);
}

void ActionAnimRegistry::RegisterRyzaHits()
{

}