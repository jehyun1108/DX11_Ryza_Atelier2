#pragma once

#include "CharaAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL AnimDataSystem
{
public:
	explicit AnimDataSystem(SystemRegistry& registry) : registry(registry) {}

	void RegisterDefaultClips();
	void SetDefaultTunings();

	void  Register(CharacterID character, AnimContext context, const ClipSet& set) { catalog[Key{ character, context }] = set; }
	const ClipSet* GetClipSet(CharacterID character, AnimContext context) const;
	const wstring& GetClipName(CharacterID character, AnimContext context, AnimKey key) const;
	
	void       SetClipTuning(CharacterID character, AnimContext context, AnimKey key, const ClipTuning& tuning);
	ClipTuning GetClipTuning(CharacterID character, AnimContext context, AnimKey key)  const;

private:
	static ClipTuning NormalizedTuning(ClipTuning tuning)
	{
		tuning.startNormalized = Utility::Saturate(tuning.startNormalized);
		tuning.endNormalized = Utility::Saturate(tuning.endNormalized);
		if (tuning.endNormalized < tuning.startNormalized)
			swap(tuning.endNormalized, tuning.startNormalized);
		if (fabsf(tuning.endNormalized - tuning.startNormalized) < 1e-5f)
			tuning.endNormalized = min(1.f, tuning.startNormalized + 1e-4f);
		return tuning;
	}

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