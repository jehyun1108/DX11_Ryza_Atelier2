#include "pch.h"
#include "UILoader.h"

#include "FontLoader.h"

void UILoader::RegisterUIResources(AssetSystem* assets)
{
	RegisterFieldUI(assets);
	RegisterBattleUI(assets);
	RegisterFont(assets);
	RegisterDressingUI(assets);
	RegisterDressingTextures(assets);
}

void UILoader::RegisterLoadingUI(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;

	meta.fullPath = L"../bin/Resources/UI/Loading/exit_game_0.png";
	assets->RegisterTexture(L"exit_game_0", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/exit_game_1.png";
	assets->RegisterTexture(L"exit_game_1", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/load_game_0.png";
	assets->RegisterTexture(L"load_game_0", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/load_game_1.png";
	assets->RegisterTexture(L"load_game_1", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/new_game_0.png";
	assets->RegisterTexture(L"new_game_0", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/new_game_1.png";
	assets->RegisterTexture(L"new_game_1", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/press_any_button_0.png";
	assets->RegisterTexture(L"press_any_button_0", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/press_any_button_1.png";
	assets->RegisterTexture(L"press_any_button_1", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/menu_hoverbar.png";
	assets->RegisterTexture(L"menu_hoverbar", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/setting_0.png";
	assets->RegisterTexture(L"setting_0", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/setting_1.png";
	assets->RegisterTexture(L"setting_1", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/title.png";
	assets->RegisterTexture(L"title", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/logo_bg.png";
	assets->RegisterTexture(L"logo_bg", meta);
	// loading
	meta.fullPath = L"../bin/Resources/UI/Loading/loading_bg.png";
	assets->RegisterTexture(L"loading_bg", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/dot.png";
	assets->RegisterTexture(L"loading_dot", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/now_loading.png";
	assets->RegisterTexture(L"now_loading", meta);
	meta.fullPath = L"../bin/Resources/UI/Loading/loading_back.png";
	assets->RegisterTexture(L"loading_back", meta);
}
// ======================================================================================
void UILoader::RegisterFieldUI(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;

	// field_minimap
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/field_minimap.png";
	assets->RegisterTexture(L"field_minimap", meta);
	// field_minimap_in
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/field_minimap_in.png";
	assets->RegisterTexture(L"field_minimap_in", meta);
	// minimap_cursor
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/minimap_cursor.png";
	assets->RegisterTexture(L"minimap_cursor", meta);
	// north
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/north.png";
	assets->RegisterTexture(L"north", meta);
	// pm_am
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/pm_am.png";
	assets->RegisterTexture(L"pm_am", meta);
	// night
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/night.png";
	assets->RegisterTexture(L"night", meta);
	// minimap_enemy_icon
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/minimap_enemy_icon.png";
	assets->RegisterTexture(L"minimap_enemy_icon", meta);
	// minimap_central
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/minimap_central.png";
	assets->RegisterTexture(L"minimap_central", meta);
	// dummy
	meta.fullPath = L"../bin/Resources/UI/Field/minimap/dummy.png";
	assets->RegisterTexture(L"dummy", meta);
	// worldmap
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/worldmap.png";
	assets->RegisterTexture(L"worldmap", meta);
	// worldmap_icon
	for (_uint i = 0; i < 8; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Field/worldmap/" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"worldmap_icon_" + to_wstring(i), meta);
	}
	// worldmap_clickicon
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/9.png";
	assets->RegisterTexture(L"worldmap_clickicon", meta);
	// worldmap_barback
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/10.png";
	assets->RegisterTexture(L"worldmap_barback", meta);
	// worldmap_character_back
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/worldmap_character_back.png";
	assets->RegisterTexture(L"worldmap_character_back", meta);
	// worldmap_dialogue
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/worldmap_dialogue.png";
	assets->RegisterTexture(L"worldmap_dialogue", meta);
	// worldmap_patricia
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/worldmap_patricia.png";
	assets->RegisterTexture(L"worldmap_patricia", meta);
	// worldmap_select
	meta.fullPath = L"../bin/Resources/UI/Field/worldmap/worldmap_select.png";
	assets->RegisterTexture(L"worldmap_select", meta);

	// maptitle_barback
	meta.fullPath = L"../bin/Resources/UI/Field/main/maptitle_barback.png";
	assets->RegisterTexture(L"maptitle_barback", meta);
	// maptitle_spinner
	meta.fullPath = L"../bin/Resources/UI/Field/main/maptitle_spinner.png";
	assets->RegisterTexture(L"maptitle_spinner", meta);
	// field_select_highlight
	meta.fullPath = L"../bin/Resources/UI/Field/main/field_select_highlight.png";
	assets->RegisterTexture(L"field_select_highlight", meta);
	// field_selectbar
	meta.fullPath = L"../bin/Resources/UI/Field/main/field_selectbar.png";
	assets->RegisterTexture(L"field_selectbar", meta);
}
// ========================================================================================
void UILoader::RegisterBattleUI(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;

	// ryza_reward
	meta.fullPath = L"../bin/Resources/UI/Battle/reward/ryza_reward.png";
	assets->RegisterTexture(L"ryza_reward", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/reward/battle_result_under.png";
	assets->RegisterTexture(L"battle_result_under", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/reward/klaudia_reward.png";
	assets->RegisterTexture(L"klaudia_reward", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/reward/patricia_reward.png";
	assets->RegisterTexture(L"patricia_reward", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/reward/serri_reward.png";
	assets->RegisterTexture(L"serri_reward", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/reward/battle_result.png";
	assets->RegisterTexture(L"battle_result", meta);

	// get_damage_ui
	meta.fullPath = L"../bin/Resources/UI/Battle/letter/battle_damage_glow.png";
	assets->RegisterTexture(L"battle_damage_glow", meta);
	// serri
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/serri.png";
	assets->RegisterTexture(L"serri", meta);

	// patricia_fataldrive
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/patricia.png";
	assets->RegisterTexture(L"patricia_fataldrive", meta);
	// klaudia_fataldrive
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/klaudia.png";
	assets->RegisterTexture(L"klaudia_fataldrive", meta);
	// ryza_fataldrive
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/ryza.png";
	assets->RegisterTexture(L"ryza_fataldrive", meta);
	// patricia_battleui
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/patricia.png";
	assets->RegisterTexture(L"patricia_battleui", meta);
	// klaudia_battleui
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/klaudia.png";
	assets->RegisterTexture(L"klaudia_battleui", meta);
	// ryza_battleui
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/ryza.png";
	assets->RegisterTexture(L"ryza_battleui", meta);
	// leader_barback_bottom
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_barback_bottom.png";
	assets->RegisterTexture(L"leader_barback_bottom", meta);
	// leader_barback_top
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_barback_top.png";
	assets->RegisterTexture(L"leader_barback_top", meta);
	// ap_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/ap_barback.png";
	assets->RegisterTexture(L"ap_barback", meta);
	// battle_minimap
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/battle_minimap.png";
	assets->RegisterTexture(L"battle_minimap", meta);
	// supportmode
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/supportmode.png";
	assets->RegisterTexture(L"supportmode", meta);
	// agressivemode
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/aggressivemode.png";
	assets->RegisterTexture(L"aggressivemode", meta);
	// timelinebar
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/timelinebar.png";
	assets->RegisterTexture(L"timelinebar", meta);
	// tactic_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barback.png";
	assets->RegisterTexture(L"tactic_barback", meta);
	// tactic_levelup
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_levelup.png";
	assets->RegisterTexture(L"tactic_levelup", meta);
	// hp_font
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/hp_font.png";
	assets->RegisterTexture(L"hp_font", meta);
	// party_hpbarfront
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/party_hpbarfront.png";
	assets->RegisterTexture(L"party_hpbarfront", meta);
	// party_hpbarback
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/party_hpbarback.png";
	assets->RegisterTexture(L"party_hpbarback", meta);
	// leader_hpbarfront
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_hpbarfront.png";
	assets->RegisterTexture(L"leader_hpbarfront", meta);
	// red_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/red_barback.png";
	assets->RegisterTexture(L"red_barback", meta);
	// red_barfront
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/red_barfront.png";
	assets->RegisterTexture(L"red_barfront", meta);
	// waittime_barback1
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barback1.png";
	assets->RegisterTexture(L"waittime_barback1", meta);
	// waittime_barback2
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barback2.png";
	assets->RegisterTexture(L"waittime_barback2", meta);
	// waittime_barfront
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barfront.png";
	assets->RegisterTexture(L"waittime_barfront", meta);
	// waittime_barfull
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barfull.png";
	assets->RegisterTexture(L"waittime_barfull", meta);
	// input_outline
	meta.fullPath = L"../bin/Resources/UI/Battle/input/outline.png";
	assets->RegisterTexture(L"input_outline", meta);
	// input_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/input/input_barback.png";
	assets->RegisterTexture(L"input_barback", meta);
	// timeline_ryza
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_ryza.png";
	assets->RegisterTexture(L"timeline_ryza", meta);
	// timeline_patricia
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_patricia.png";
	assets->RegisterTexture(L"timeline_patricia", meta);
	// timeline_klaudia
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_klaudia.png";
	assets->RegisterTexture(L"timeline_klaudia", meta);
	// timeline_npc
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_npc.png";
	assets->RegisterTexture(L"timeline_npc", meta);
	// timeline_leader
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_leader.png";
	assets->RegisterTexture(L"timeline_leader", meta);
	// pinch
	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/pinch.png";
	assets->RegisterTexture(L"pinch", meta);

	// target_rect
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_rect.png";
	assets->RegisterTexture(L"target_rect", meta);

	// defend_icon
	meta.fullPath = L"../bin/Resources/UI/Battle/input/defend_icon.png";
	assets->RegisterTexture(L"defend_icon", meta);
	// attack_icon
	meta.fullPath = L"../bin/Resources/UI/Battle/input/attack_icon.png";
	assets->RegisterTexture(L"attack_icon", meta);
	// flee_icon
	meta.fullPath = L"../bin/Resources/UI/Battle/input/flee_icon.png";
	assets->RegisterTexture(L"flee_icon", meta);
	// itemrush_icon
	meta.fullPath = L"../bin/Resources/UI/Battle/input/itemrush_icon.png";
	assets->RegisterTexture(L"itemrush_icon", meta);

	// input_y
	meta.fullPath = L"../bin/Resources/UI/Battle/input/Y.png";
	assets->RegisterTexture(L"input_y", meta);
	// input_b
	meta.fullPath = L"../bin/Resources/UI/Battle/input/B.png";
	assets->RegisterTexture(L"input_b", meta);
	// input_x
	meta.fullPath = L"../bin/Resources/UI/Battle/input/X.png";
	assets->RegisterTexture(L"input_x", meta);
	// input_a
	meta.fullPath = L"../bin/Resources/UI/Battle/input/A.png";
	assets->RegisterTexture(L"input_a", meta);

	// skill_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/input/skill_barback.png";
	assets->RegisterTexture(L"skill_barback", meta);
	// punch_icon
	meta.fullPath = L"../bin/Resources/UI/Battle/input/punch.png";
	assets->RegisterTexture(L"punch_icon", meta);

	// skill_highlight_bar
	meta.fullPath = L"../bin/Resources/UI/Battle/input/skill_highlight_bar.png";
	assets->RegisterTexture(L"skill_highlight_bar", meta);

	// tactic_barfront1~5
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barfront_1.png";
	assets->RegisterTexture(L"tactic_barfront1", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barfront_2.png";
	assets->RegisterTexture(L"tactic_barfront2", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barfront_3.png";
	assets->RegisterTexture(L"tactic_barfront3", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barfront_4.png";
	assets->RegisterTexture(L"tactic_barfront4", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barfront_5.png";
	assets->RegisterTexture(L"tactic_barfront5", meta);
	// tactic_barfull
	meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barfull.png";
	assets->RegisterTexture(L"tactic_barfull", meta);

	// switching_emerald
	meta.fullPath = L"../bin/Resources/UI/Battle/switching/emerald.png";
	assets->RegisterTexture(L"leader_switching_emerald", meta);
	// leader_switching_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/switching/barback.png";
	assets->RegisterTexture(L"leader_switching_barback", meta);
	// leader_switching_left
	meta.fullPath = L"../bin/Resources/UI/Battle/switching/left.png";
	assets->RegisterTexture(L"leader_switching_left", meta);
	// leader_switching_right
	meta.fullPath = L"../bin/Resources/UI/Battle/switching/right.png";
	assets->RegisterTexture(L"leader_switching_right", meta);

	// fataldrive_barfront
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/front.png";
	assets->RegisterTexture(L"fataldrive_barfront", meta);
	// fataldrive_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/back.png";
	assets->RegisterTexture(L"fataldrive_barback", meta);
	// fataldrive_fatal
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/fatal.png";
	assets->RegisterTexture(L"fataldrive_fatal", meta);
	// fataldrive_drive
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/drive.png";
	assets->RegisterTexture(L"fataldrive_drive", meta);
	// fataldrive_redlight
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/fataldrive_redlight.png";
	assets->RegisterTexture(L"fataldrive_redlight", meta);
	// fataldrive_rect

	// fataldrive_F
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/F.png";
	assets->RegisterTexture(L"fataldrive_f", meta);
	// fataldrive_A
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/A.png";
	assets->RegisterTexture(L"fataldrive_a", meta);
	// fataldrive_T
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/T.png";
	assets->RegisterTexture(L"fataldrive_t", meta);
	// fataldrive_L
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/L.png";
	assets->RegisterTexture(L"fataldrive_l", meta);
	// fataldrive_D
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/D.png";
	assets->RegisterTexture(L"fataldrive_d", meta);
	// fataldrive_R
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/R.png";
	assets->RegisterTexture(L"fataldrive_r", meta);
	// fataldrive_I
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/I.png";
	assets->RegisterTexture(L"fataldrive_i", meta);
	// fataldrive_V
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/V.png";
	assets->RegisterTexture(L"fataldrive_v", meta);
	// fataldrive_E
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/E.png";
	assets->RegisterTexture(L"fataldrive_e", meta);

	// fataldrive_ring
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/clock.png";
	assets->RegisterTexture(L"fataldrive_ring", meta);
	// fataldrive_bg
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/bg.png";
	assets->RegisterTexture(L"fataldrive_bg", meta);
	// fataldrive_bg2
	meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/bg_fadeout.png";
	assets->RegisterTexture(L"fataldrive_bg2", meta);

	// dmgletter_ + (0~9)
	for (int i = 0; i <= 9; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/letter/dmgletter_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"dmgletter_" + to_wstring(i), meta);
	}
	// chainletter_ + (0~9)
	for (int i = 0; i <= 9; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/letter/chainletter_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"chainletter_" + to_wstring(i), meta);
	}
	// battleletter_ + (0~9)
	for (int i = 0; i <= 9; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/letter/battleletter_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"battleletter_" + to_wstring(i), meta);
	}
	// battleletter_large_ + (0~9)
	for (int i = 0; i <= 9; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/letter/battleletter_large_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"battleletter_large_" + to_wstring(i), meta);
	}
	 
	// total_damage
	meta.fullPath = L"../bin/Resources/UI/Battle/letter/total_damage.png";
	assets->RegisterTexture(L"total_damage", meta);
	// input_ (red,blue,yellow,green)
	meta.fullPath = L"../bin/Resources/UI/Battle/input/input_red.png";
	assets->RegisterTexture(L"input_red", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/input/input_blue.png";
	assets->RegisterTexture(L"input_blue", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/input/input_yellow.png";
	assets->RegisterTexture(L"input_yellow", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/input/input_green.png";
	assets->RegisterTexture(L"input_green", meta);

	// ryza_skill_
	for (int i = 1; i <= 4; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/input/ryza_skill_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"ryza_skill_" + to_wstring(i), meta);
	}
	for (int i = 1; i <= 4; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/input/patricia_skill_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"patricia_skill_" + to_wstring(i), meta);
	}
	for (int i = 1; i <= 4; ++i)
	{
		meta.fullPath = L"../bin/Resources/UI/Battle/input/klaudia_skill_" + to_wstring(i) + L".png";
		assets->RegisterTexture(L"klaudia_skill_" + to_wstring(i), meta);
	}
	meta.fullPath = L"../bin/Resources/UI/Battle/input/input_highlight_circle.png";
	assets->RegisterTexture(L"input_highlight_circle", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/switching/change_character.png";
	assets->RegisterTexture(L"change_character", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/switching/change_character_back.png";
	assets->RegisterTexture(L"change_character_back", meta);

	meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/leader_icon_highlight.png";
	assets->RegisterTexture(L"leader_icon_highlight", meta);

	// enemy_ABC
	meta.fullPath = L"../bin/Resources/UI/Battle/target/enemy_A.png";
	assets->RegisterTexture(L"enemy_A", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/target/enemy_B.png";
	assets->RegisterTexture(L"enemy_B", meta);
	meta.fullPath = L"../bin/Resources/UI/Battle/target/enemy_C.png";
	assets->RegisterTexture(L"enemy_C", meta);
	// target_cursor
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_cursor.png";
	assets->RegisterTexture(L"target_cursor", meta);
	// target_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_barback.png";
	assets->RegisterTexture(L"target_barback", meta);
	// target_hp_barback
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_hp_barback.png";
	assets->RegisterTexture(L"target_hp_barback", meta);
	// target_hp_barfront
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_hp_barfront.png";
	assets->RegisterTexture(L"target_hp_barfront", meta);
	// target_letter
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_letter.png";
	assets->RegisterTexture(L"target_letter", meta);
	// target_timeline_ring_back
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_timeline_ring_back.png";
	assets->RegisterTexture(L"target_timeline_ring_back", meta);
	// target_timeline_ring_front
	meta.fullPath = L"../bin/Resources/UI/Battle/target/target_timeline_ring_front.png";
	assets->RegisterTexture(L"target_timeline_ring_front", meta);

	// chain
	meta.fullPath = L"../bin/Resources/UI/Battle/letter/chain.png";
	assets->RegisterTexture(L"chain", meta);
	// chain_damage
	meta.fullPath = L"../bin/Resources/UI/Battle/letter/chain_damage.png";
	assets->RegisterTexture(L"chain_damage", meta);
	// chain_x
	meta.fullPath = L"../bin/Resources/UI/Battle/letter/chain_x.png";
	assets->RegisterTexture(L"chain_x", meta);

	// battle_letter_
}

void UILoader::RegisterFont(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;

	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_24.png";
	assets->RegisterTexture(L"GmarketSansTTFMedium_24", meta);
	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_28.png";
	assets->RegisterTexture(L"GmarketSansTTFMedium_28", meta);
	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_32.png"; 
	assets->RegisterTexture(L"GmarketSansTTFMedium_32", meta);
	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_36.png";
	assets->RegisterTexture(L"GmarketSansTTFMedium_36", meta);
	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_40.png";
	assets->RegisterTexture(L"GmarketSansTTFMedium_40", meta);
	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_44.png";
	assets->RegisterTexture(L"GmarketSansTTFMedium_44", meta);
	meta.fullPath = L"../bin/Resources/Fonts/GmarketSansTTFMedium_48.png";
	assets->RegisterTexture(L"GmarketSansTTFMedium_48", meta);
}

void UILoader::RegisterOverlayUI(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;

	meta.fullPath = L"../bin/Resources/UI/Overlay/black.png";
	assets->RegisterTexture(L"black", meta);
	
	meta.fullPath = L"../bin/Resources/UI/Overlay/white.png";
	assets->RegisterTexture(L"white", meta);
}

void UILoader::RegisterDressingUI(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;
	// dressing_bg
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_bg.png";
	assets->RegisterTexture(L"dressing_bg", meta);
	// dressing_circle
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_circle.png";
	assets->RegisterTexture(L"dressing_circle", meta);
	// dressing_divider
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_divider.png";
	assets->RegisterTexture(L"dressing_divider", meta);
	// dressing_paper
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_paper.png";
	assets->RegisterTexture(L"dressing_paper", meta);
	// dressing_select
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_select.png";
	assets->RegisterTexture(L"dressing_select", meta);
	// dressing_select_highlight
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_select_highlight.png";
	assets->RegisterTexture(L"dressing_select_highlight", meta);
	// dressing_tab_equipbar
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_tab_equipbar.png";
	assets->RegisterTexture(L"dressing_tab_equipbar", meta);
	// dressing_dummy
	meta.fullPath = L"../bin/Resources/UI/DressingRoom/dressing_char_rt.png";
	assets->RegisterTexture(L"dressing_char_rt", meta);
}

void UILoader::RegisterDressingTextures(AssetSystem* assets)
{
	TextureMeta meta{};
	meta.colorSpace = TextureColorSpace::sRGB;
	// Ryza
	meta.fullPath = L"../bin/Resources/Models/Ryza/0_0.dds";
	assets->RegisterTexture(L"ryza/0_0", meta);
	meta.fullPath = L"../bin/Resources/Models/Ryza/0_1.dds";
	assets->RegisterTexture(L"ryza/0_1", meta);
	meta.fullPath = L"../bin/Resources/Models/Ryza/0_2.dds";
	assets->RegisterTexture(L"ryza/0_2", meta);
	meta.fullPath = L"../bin/Resources/Models/Ryza/0_3.dds";
	assets->RegisterTexture(L"ryza/0_3", meta);
	// klaudia
	meta.fullPath = L"../bin/Resources/Models/Klaudia/0_0.dds";
	assets->RegisterTexture(L"klaudia/0_0", meta);
	meta.fullPath = L"../bin/Resources/Models/Klaudia/0_1.dds";
	assets->RegisterTexture(L"klaudia/0_1", meta);
	meta.fullPath = L"../bin/Resources/Models/Klaudia/0_2.dds";
	assets->RegisterTexture(L"klaudia/0_2", meta);
	meta.fullPath = L"../bin/Resources/Models/Klaudia/0_3.dds";
	assets->RegisterTexture(L"klaudia/0_3", meta);
	// patricia
	meta.fullPath = L"../bin/Resources/Models/Patricia/0_0.dds";
	assets->RegisterTexture(L"patricia/0_0", meta);
	meta.fullPath = L"../bin/Resources/Models/Patricia/0_1.dds";
	assets->RegisterTexture(L"patricia/0_1", meta);
	meta.fullPath = L"../bin/Resources/Models/Patricia/0_2.dds";
	assets->RegisterTexture(L"patricia/0_2", meta);
	meta.fullPath = L"../bin/Resources/Models/Patricia/0_3.dds";
	assets->RegisterTexture(L"patricia/0_3", meta);
}

void UILoader::InitFonts(SystemRegistry& registry)
{
	auto& fontSys = registry.Get<FontSystem>();
	namespace fs = std::filesystem;

	auto LoadOneFont = [&](const wchar_t* metaPathW)
		{
			fs::path fontMetaPath = metaPathW;

			FontDesc desc{};
			string error;
			if (!FontLoader::LoadFontDescFromBinary(fontMetaPath, desc, error))
			{
				assert(false && "Failed to load font meta");
				return;
			}

			auto it = desc.glyphs.find(U'a');  // 0xC6D0
			OutputDebugStringA("InitFonts: start\n");
			if (it != desc.glyphs.end())
				OutputDebugStringA("font has ¿ø\n");
			else
				OutputDebugStringA("font has NO ¿ø\n");

			wstring key = fontMetaPath.stem().wstring();

			desc.name = key;
			desc.atlasNameKey = key;  

			fontSys.RegisterFont(key, desc);
		};
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_24.font");
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_28.font");
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_32.font");
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_36.font");
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_40.font");
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_44.font");
	LoadOneFont(L"../bin/Resources/Fonts/GmarketSansTTFMedium_48.font");
}