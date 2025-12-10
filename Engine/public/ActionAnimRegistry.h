#pragma once

#include "ActionAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ActionAnimRegistry : public ISystem
{
public:
	explicit ActionAnimRegistry(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Register(CharacterID id, const ActionAnimSpec& spec) { table[id] = spec; }
	void RegisterSpecial(CharacterID id, SpecialAnimTag tag, const vector<AnimKey>& animKeys, ActionStage stageForAll = ActionStage::Preparation, bool rootMotionForAll = true, float fadeDur = 0.2f, int apCost = 0);

	const ActionAnimSpec& Get(CharacterID id) const;
	const AnimChainSpec&  GetSpecial(CharacterID id, SpecialAnimTag tag) const;

	SpecialAnimTag       GetStepTag(CharacterID id, SkillSlotTag slot, int stepIdx) const;
	const SkillStepInfo& GetStepInfo(CharacterID id, SpecialAnimTag tag) const;

	void SetTagdmgMul(CharacterID id, SpecialAnimTag tag, float mul);
	void SetStageHits(CharacterID id, SpecialAnimTag tag, int stageIdx, const vector<HitPoint>& hits);
	void SetChainHits(CharacterID id, SpecialAnimTag tag, const vector<vector<HitPoint>>& hitsByStage);

	void SetSfxSequence(CharacterID id, SpecialAnimTag tag, const vector<wstring>& keys);

	void SetSkillSteps(CharacterID id, const array<SkillStepInfo, 12>& steps) { skillStepsByChar[id] = steps; }
	
private:
	AnimStageSpec MakeStage(AnimKey clipKey, ActionStage stage, float fadeDur = 0.1f, bool useRootMotion = true);
	AnimChainSpec MakeChainSimple(const vector<AnimKey>& keys, ActionStage stageForAll = ActionStage::Active, bool rootMotionForAll = false, float fadeDur = 0.1f);

private:
	unordered_map<CharacterID, ActionAnimSpec> table{};
	unordered_map<CharacterID, array<SkillStepInfo, 12>> skillStepsByChar{};

private:
	void RegisterPatriciaAnim();
	void RegisterRyzaAnim();
	void RegisterKlaudiaAnim();
	void RegisterAngelAnim();
	void RegisterRyzaHits();

private:
	SystemRegistry& registry;
};

NS_END