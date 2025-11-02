#pragma once

NS_BEGIN(Engine)

enum class CharacterID { Ryza, Klaudia, Patricia, Angel, Unknown };
enum class AnimContext { Field, Battle };
enum class AnimKey
{
	// Field
	Idle,
	WalkStart, WalkLoop, WalkEnd,
	RunStart,  RunLoop,  RunEnd,
	JumpStart, JumpLoop, JumpEnd,
	FieldSwing,

	// Battle
	Battle_Idle,
	Battle_StartA,           Battle_StartB,           Battle_StartC,

	Battle_RunStart,         Battle_RunLoop,          Battle_RunEnd,

	Battle_AttackA,          Battle_AttackB,          Battle_AttackC,

	Battle_Defend_Ready,     Battle_Defending,        Battle_Defend_Finished,

	Battle_Celemony_1A,      Battle_Celemony_1B,
	Battle_Celemony_2A,      Battle_Celemony_2B,

	Battle_Attack_FinishedA, Battle_Attack_FinishedB, Battle_Attack_FinishedC,

	Battle_Skill_A1, Battle_Skill_A2, Battle_Skill_A3,
	Battle_Skill_B1, Battle_Skill_B2, Battle_Skill_B3,
	Battle_Skill_C1, Battle_Skill_C2, Battle_Skill_C3,
	Battle_Skill_D1, Battle_Skill_D2, Battle_Skill_D3,

	Battle_Hit,
	Battle_Down,

	Battle_Ultimate_Ryza1, Battle_Ultimate_Ryza2, Battle_Ultimate_Ryza3, Battle_Ultimate_Ryza4, Battle_Ultimate_Ryza5,

	Battle_Ultimate_Klaudia1, Battle_Ultimate_Klaudia2, Battle_Ultimate_Klaudia3, Battle_Ultimate_Klaudia4, Battle_Ultimate_Klaudia5,
	Battle_Ultimate_Klaudia6, Battle_Ultimate_Klaudia7, Battle_Ultimate_Klaudia8, Battle_Ultimate_Klaudia9,

	Battle_Ultimate_Patricia1, Battle_Ultimate_Patricia2, Battle_Ultimate_Patricia3, Battle_Ultimate_Patricia4, Battle_Ultimate_Patricia5,
	Battle_Ultimate_Patricia6, Battle_Ultimate_Patricia7
};

struct ClipTuning
{
	float startNormalized = 0.f;
	float endNormalized   = 1.f;
	float playbackSpeed   = 1.f;
	bool  useRootMotion   = false;
};

struct ClipSet
{
	unordered_map<AnimKey, wstring>    nameByKey;
	unordered_map<AnimKey, ClipTuning> tuningByKey;

	const wstring& Require(AnimKey key) const
	{
		static const wstring empty = L"";
		auto it = nameByKey.find(key);
		return (it != nameByKey.end()) ? it->second : empty;
	}
	ClipTuning ResolveTuning(AnimKey key) const
	{
		auto it = tuningByKey.find(key);
		if (it != tuningByKey.end())
			return it->second;
		return ClipTuning{};
	}
};

struct AnimProfile
{
	CharacterID character = CharacterID::Unknown;
	AnimContext context   = AnimContext::Field;
};

NS_END