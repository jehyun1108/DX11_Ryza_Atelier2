#pragma once

#include "ActionAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ActionAnimRegistry
{
public:
	void Register(CharacterID character, const ActionAnimSpec& spec) { table[character] = spec; }
	const ActionAnimSpec* TryGet(CharacterID character)                            const;
	const AnimChainSpec*  TryGetSpecial(CharacterID character, SpecialAnimTag tag) const;
	void RegisterDefaultAnim();

	void RegisterSpecial(CharacterID character, SpecialAnimTag tag, const vector<AnimKey>& animKeys, ActionStage stageForAll = ActionStage::Preparation, bool rootMotionForAll = true, float fadeDur = 0.06f, int apCost = 0);
	
private:
	AnimStageSpec MakeStage(AnimKey clipKey, ActionStage stage, float fadeDur = 0.06f, bool useRootMotion = true);
	AnimChainSpec MakeChainSimple(const vector<AnimKey>& keys, ActionStage stageForAll = ActionStage::Active, bool rootMotionForAll = false, float fadeDur = 0.06f);

private:
	void RegisterPatriciaAnim();
	void RegisterRyzaAnim();
	void RegisterKlaudiaAnim();
	void RegisterAngelAnim();

private:
	unordered_map<CharacterID, ActionAnimSpec> table;
};

NS_END