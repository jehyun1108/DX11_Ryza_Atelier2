#pragma once

#include "ActionAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ActionAnimRegistry
{
public:
	void Register(CharacterID character, const ActionAnimSpec& spec) { table[character] = spec; }
	const ActionAnimSpec* TryGet(CharacterID character) const
	{
		auto it = table.find(character);
		return (it == table.end()) ? nullptr : &it->second;
	}

	void RegisterDefaultAnim();

private:
	AnimStageSpec MakeStage(AnimKey clipKey, ActionStage stage, float fadeDur = 0.06f, bool useRootMotion = false);
	AnimChainSpec MakeChain(initializer_list<AnimStageSpec> stageList);
	ComboSpec     MakeSingleChainCombo(const AnimChainSpec& chain);

private:
	unordered_map<CharacterID, ActionAnimSpec> table;
};

NS_END