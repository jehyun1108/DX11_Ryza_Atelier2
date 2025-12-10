#include "Enginepch.h"
#include "ActionFxRegistry.h"

void ActionFxRegistry::RegisterFx(CharacterID characterId, SpecialAnimTag tag, const ActionFxSet& fx)
{
	ActionFxKey key = MakeActionFxKey(characterId, tag);
	fxByKey[key] = fx;
}

const ActionFxSet* ActionFxRegistry::FindFx(CharacterID characterId, SpecialAnimTag tag) const
{
	ActionFxKey key = MakeActionFxKey(characterId, tag);
	auto it = fxByKey.find(key);
	if (it == fxByKey.end()) return nullptr;
	return &it->second;
}