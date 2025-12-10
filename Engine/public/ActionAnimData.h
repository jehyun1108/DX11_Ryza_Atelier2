#pragma once

NS_BEGIN(Engine)

enum class SkillSlotTag { A, B, C, D };
enum class SpecialAnimTag
{
	None,
	Intro, Ultimate, BasicAttack, ItemRush,
	SkillA_1, SkillA_2, SkillA_3,
	SkillB_1, SkillB_2, SkillB_3,
	SkillC_1, SkillC_2, SkillC_3,
	SkillD_1, SkillD_2, SkillD_3,
	AttackFinished, AttackStarted,
	DefendStart, Defending, DefendEnd
};
struct SkillStepInfo
{
	SkillSlotTag   slot;
	int            stepIdx;
	SpecialAnimTag tag;
	float          inputEndNorm;
	float          chainCutNorm;
	float          nextStepStartNorm;
};
// ===============================
struct HitPoint
{
	float timeNorm = 0.f;
	float dmgRatio = 1.f;
	wstring sfxKey;
};
struct AnimStageSpec
{
	AnimKey     clipKey;
	ActionStage stage;
	float       fadeDur       = 0.1f;
	bool        rootMotion    = false;
	float       minOverlapDur = 0.05f;

	optional<float> startNormalizedOverride; 
	optional<float> endNormalizedOverride;   
	optional<float> playbackSpeedOverride;

	vector<HitPoint> hits;
	wstring startSfxKey;
};
struct AnimChainSpec
{
	vector<AnimStageSpec> stages;
};
struct ActionAnimSpec
{
	unordered_map<SpecialAnimTag, AnimChainSpec> specials;
	unordered_map<SpecialAnimTag, int>           apCostByTag;
	unordered_map<SpecialAnimTag, float>         dmgMulByTag;
	// 순차 재생용
	unordered_map<SpecialAnimTag, vector<wstring>> sfxSeqByTag;
};
struct StageDesc
{
	AnimKey          key;
	ActionStage      stage      = ActionStage::Active;
	bool             rootMotion = false;
	float            fadeDur    = 0.06f;
	vector<HitPoint> hits;
};

NS_END