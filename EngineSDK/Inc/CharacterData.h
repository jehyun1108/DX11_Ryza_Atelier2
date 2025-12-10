#pragma once

NS_BEGIN(Engine)

enum class UITextureSlot
{
	FatalDrive,
	TimelineIcon,
	BattleCharacterIcon,
};

struct CharacterUITextures
{
	unordered_map<UITextureSlot, wstring> slotToTexKey;
	void Set(UITextureSlot texSlot, const wstring& texKey){ slotToTexKey[texSlot] = texKey; }
	const wstring* TryGet(UITextureSlot texSlot) const
	{
		auto it = slotToTexKey.find(texSlot);
		return (it == slotToTexKey.end()) ? nullptr : &it->second;
	}
};

struct PartySpec
{
	int baseLevel  = 1;
	int baseMaxHp  = 100;
	int attackDmg  = 10;
	int baseMaxExp = 100;
};

struct EnemySpec
{
	int baseMapHp = 60;
	int attackDmg = 8;
};

struct CharacterSpec
{
	BattleTeam          team = BattleTeam::Ally;
	CharacterUITextures ui;
	PartySpec           party{};
	EnemySpec           enemy{};
	EntityID            camAnchorEntity{};
};

NS_END