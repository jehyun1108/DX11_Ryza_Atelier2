#include "pch.h"
#include "SoundLoader.h"
#include "SoundRegistry.h"

void SoundLoader::Load(SoundRegistry* sound)
{
	LoadRyzaSound(sound);
	LoadKlaudiaSound(sound);
	LoadPatriciaSound(sound);
	LoadBGM(sound);
	LoadEffectSound(sound);
}

void SoundLoader::LoadRyzaSound(SoundRegistry* sound)
{
	constexpr _uint count = 58u;
	for (_uint i = 0; i < count; ++i)
		sound->Register(L"ryza_" + to_wstring(i), L"../bin/Resources/Sound/Ryza/ryza_" + to_wstring(i) + L".wav");
}

void SoundLoader::LoadKlaudiaSound(SoundRegistry* sound)
{
	constexpr _uint count = 48u;
	for (_uint i = 0; i < count; ++i)
		sound->Register(L"klaudia_" + to_wstring(i), L"../bin/Resources/Sound/Klaudia/klaudia_" + to_wstring(i) + L".wav");
}

void SoundLoader::LoadPatriciaSound(SoundRegistry* sound)
{
	constexpr _uint count = 41u;
	for (_uint i = 0; i < count; ++i)
		sound->Register(L"patricia_" + to_wstring(i), L"../bin/Resources/Sound/Patricia/patricia_" + to_wstring(i) + L".wav");
}

void SoundLoader::LoadBGM(SoundRegistry* sound)
{
	//sound->Register(L"Battle_BGM_1", L"../bin/Resources/Sound/BGM/Battle_BGM_1.wav");
	//sound->Register(L"Battle_BGM_2", L"../bin/Resources/Sound/BGM/Battle_BGM_2.wav");
	sound->Register(L"Battle_BGM_3",      L"../bin/Resources/Sound/BGM/Battle_BGM_3.wav");
	sound->Register(L"Intro_BGM",         L"../bin/Resources/Sound/BGM/Intro_BGM.wav"  );
	sound->Register(L"Central_BGM",       L"../bin/Resources/Sound/BGM/Central_BGM.wav");
	sound->Register(L"Valley_BGM",        L"../bin/Resources/Sound/BGM/Valley_BGM.wav");
	sound->Register(L"House_BGM",         L"../bin/Resources/Sound/BGM/House_BGM.wav");
	sound->Register(L"Battle_Reward_BGM", L"../bin/Resources/Sound/BGM/Battle_Reward_BGM.wav");
}

void SoundLoader::LoadEffectSound(SoundRegistry* sound)
{
	sound->Register(L"Press_Start", L"../bin/Resources/Sound/Effect/Press_Start.wav");

	sound->Register(L"01_turn_get",          L"../bin/Resources/Sound/BattleSystem/01_turn_get.wav");
	sound->Register(L"02_apmax",             L"../bin/Resources/Sound/BattleSystem/02_apmax.wav");
	sound->Register(L"03_ccmax",             L"../bin/Resources/Sound/BattleSystem/03_ccmax.wav");
	sound->Register(L"04_tlvup",             L"../bin/Resources/Sound/BattleSystem/04_tlvup.wav");
	sound->Register(L"05_skill_chain",       L"../bin/Resources/Sound/BattleSystem/05_skill_chain.wav");
	sound->Register(L"06_item_rush",         L"../bin/Resources/Sound/BattleSystem/06_item_rush.wav");
	sound->Register(L"07_order_issue",       L"../bin/Resources/Sound/BattleSystem/07_order_issue.wav");
	sound->Register(L"08_order_comp",        L"../bin/Resources/Sound/BattleSystem/08_order_comp.wav");
	sound->Register(L"09_break",             L"../bin/Resources/Sound/BattleSystem/09_break.wav");
	sound->Register(L"010_enemydown",        L"../bin/Resources/Sound/BattleSystem/010_enemydown.wav");
	sound->Register(L"011_raredrop",         L"../bin/Resources/Sound/BattleSystem/011_raredrop.wav");
	sound->Register(L"012_chara_select",     L"../bin/Resources/Sound/BattleSystem/012_chara_select.wav");
	sound->Register(L"013_click",            L"../bin/Resources/Sound/BattleSystem/013_click.wav");
	sound->Register(L"014_cancel",           L"../bin/Resources/Sound/BattleSystem/014_cancel.wav");
	sound->Register(L"015_switch",           L"../bin/Resources/Sound/BattleSystem/015_switch.wav");
	sound->Register(L"016_shift_ready",      L"../bin/Resources/Sound/BattleSystem/016_shift_ready.wav");
	sound->Register(L"017_shift_activate",   L"../bin/Resources/Sound/BattleSystem/017_shift_activate.wav");
	sound->Register(L"018_fatal_charge",     L"../bin/Resources/Sound/BattleSystem/018_fatal_charge.wav");
	sound->Register(L"019_fatal_activate",   L"../bin/Resources/Sound/BattleSystem/019_fatal_activate.wav");
	sound->Register(L"020_command_help",     L"../bin/Resources/Sound/BattleSystem/020_command_help.wav");
	sound->Register(L"021_lvup",             L"../bin/Resources/Sound/BattleSystem/021_lvup.wav");
	sound->Register(L"022_battle_start",     L"../bin/Resources/Sound/BattleSystem/022_battle_start.wav");
	sound->Register(L"023_lvup2",            L"../bin/Resources/Sound/BattleSystem/023_lvup2.wav");
	sound->Register(L"024_apbonus",          L"../bin/Resources/Sound/BattleSystem/024_apbonus.wav");

	sound->Register(L"enter_dressing", L"../bin/Resources/Sound/Effect/enter_dressing.wav");
	sound->Register(L"exit_dressing", L"../bin/Resources/Sound/Effect/exit_dressing.wav");
	sound->Register(L"enter_worldmap", L"../bin/Resources/Sound/Effect/enter_worldmap.wav");
	sound->Register(L"worldmap_transfer", L"../bin/Resources/Sound/Effect/worldmap_transfer.wav");
	sound->Register(L"swing_wand", L"../bin/Resources/Sound/Effect/swing_wand.wav");

	sound->Register(L"wear_cloth", L"../bin/Resources/Sound/Foot/wear_cloth.wav");
	sound->Register(L"ground01_a", L"../bin/Resources/Sound/Foot/wear_cloth.wav");
	sound->Register(L"ground01_b", L"../bin/Resources/Sound/Foot/wear_cloth.wav");
	sound->Register(L"jump", L"../bin/Resources/Sound/Foot/jump.wav");
	sound->Register(L"land", L"../bin/Resources/Sound/Foot/land.wav");
}
