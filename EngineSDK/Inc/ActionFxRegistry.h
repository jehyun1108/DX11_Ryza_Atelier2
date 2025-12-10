#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ActionFxRegistry : public ISystem
{
public:
	explicit ActionFxRegistry(SystemRegistry& registry) : registry(registry) {}

	void               RegisterFx(CharacterID characterId, SpecialAnimTag tag, const ActionFxSet& fx);
	const ActionFxSet* FindFx(CharacterID characterId, SpecialAnimTag tag) const;

	static inline ActionFxKey MakeActionFxKey(CharacterID characterId, SpecialAnimTag tag)
	{
		return (ActionFxKey)ENUM(characterId) << 16 | (ActionFxKey)ENUM(tag);
	}

private:
	unordered_map<ActionFxKey, ActionFxSet> fxByKey;

private:
	SystemRegistry& registry;
};

NS_END