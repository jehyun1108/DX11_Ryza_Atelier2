#pragma once

#include "CharaAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL AnimDataSystem
{
public:
	explicit AnimDataSystem(SystemRegistry& registry) : registry(registry) {}

	void RegisterDefaultClips();

	void  Register(CharacterID character, AnimContext context, const ClipSet& set) { catalog[Key{ character, context }] = set; }
	const ClipSet* GetClipSet(CharacterID character, AnimContext context) const;
	const wstring& GetClipName(CharacterID character, AnimContext context, AnimKey key) const;

private:
	struct Key
	{
		CharacterID character;
		AnimContext context;
		bool operator==(const Key& o) const noexcept { return character == o.character && context == o.context; }
	};
	struct KeyHash
	{
		size_t operator()(const Key& key) const noexcept 
		{
			return (static_cast<size_t>(key.character) * 131u) ^ static_cast<size_t>(key.context);
		}
	};

private:
	SystemRegistry& registry;
	unordered_map<Key, ClipSet, KeyHash> catalog;
};

NS_END