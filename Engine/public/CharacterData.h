#pragma once

NS_BEGIN(Engine)

enum class UITextureSlot
{
	FatalDrive,
	TimelineIcon,
};

struct CharacterUITextures
{
	unordered_map<UITextureSlot, wstring> slotToTexKey;
	
	void Set(UITextureSlot texSlot, const wstring& texKey)
	{
		slotToTexKey[texSlot] = texKey;
	}

	const wstring* TryGet(UITextureSlot texSlot) const
	{
		auto it = slotToTexKey.find(texSlot);
		return (it == slotToTexKey.end()) ? nullptr : &it->second;
	}

	bool Empty() const { return slotToTexKey.empty(); }
};

struct CharacterParams
{
	int     level = 1;
	int     maxHp = 100;
	int     curHp = 100;
};

NS_END