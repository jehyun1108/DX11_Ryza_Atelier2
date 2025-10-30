#pragma once

NS_BEGIN(Engine)

enum class CharacterID { Ryza, Kluadia, Patricia, Angel, Unknown };
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
	Battle_Celemony_1A,      Battle_Celemony_1B,
	Battle_Celemony_2A,      Battle_Celemony_2B,
	Battle_Attack_FinishedA, Battle_Attack_FinishedB, Battle_Attack_FinishedC,
	Battle_Skill_1A,         Battle_Skill_1B,         Battle_Skill_1C,

	Battle_SkillA,
	Battle_Hit,
	Battle_Down,
};

struct ClipSet
{
	unordered_map<AnimKey, wstring> nameByKey;

	const wstring& Require(AnimKey key) const
	{
		static const wstring empty = L"";
		auto it = nameByKey.find(key);
		return (it != nameByKey.end()) ? it->second : empty;
	}
};

struct AnimProfile
{
	CharacterID character = CharacterID::Unknown;
	AnimContext context   = AnimContext::Field;
};

NS_END