#pragma once

#include "UIArchetypeLoader.h"

NS_BEGIN(Client)

class UILoader
{
public:
	inline static void RegisterUIResources(AssetSystem& assets)
	{
		TextureMeta meta{};
		meta.colorSpace = TextureColorSpace::sRGB;
		// patricia_fataldrive
		meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/patricia.png";
		assets.RegisterTexture(L"patricia_fataldrive", meta);
		// klaudia_fataldrive
		meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/klaudia.png";
		assets.RegisterTexture(L"klaudia_fataldrive", meta);
		// ryza_fataldrive
		meta.fullPath = L"../bin/Resources/UI/Battle/fataldrive/ryza.png";
		assets.RegisterTexture(L"ryza_fataldrive", meta);
		// patricia_battleui
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/patricia.png";
		assets.RegisterTexture(L"patricia_battleui", meta);
		// klaudia_battleui
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/klaudia.png";
		assets.RegisterTexture(L"klaudia_battleui", meta);
		// ryza_battleui
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/ryza.png";
		assets.RegisterTexture(L"ryza_battleui", meta);
		// leader_barback_bottom
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_barback_bottom.png";
		assets.RegisterTexture(L"leader_barback_bottom", meta);
		// leader_barback_top
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_barback_top.png";
		assets.RegisterTexture(L"leader_barback_top", meta);
		// ap_barback
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/ap_barback.png";
		assets.RegisterTexture(L"ap_barback", meta);
		// leader_switching_barback
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_switching_barback.png";
		assets.RegisterTexture(L"leader_switching_barback", meta);
		// leader_switching_left
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_switching_left.png";
		assets.RegisterTexture(L"leader_switching_left", meta);
		// leader_switching_right
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_switching_right.png";
		assets.RegisterTexture(L"leader_switching_right", meta);
		// battle_minimap
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/battle_minimap.png";
		assets.RegisterTexture(L"battle_minimap", meta);
		// supportmode
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/supportmode.png";
		assets.RegisterTexture(L"supportmode", meta);
		// agressivemode
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/aggressivemode.png";
		assets.RegisterTexture(L"aggressivemode", meta);
		// timelinebar
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/timelinebar.png";
		assets.RegisterTexture(L"timelinebar", meta);
		// tactic_barback
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_barback.png";
		assets.RegisterTexture(L"tactic_barback", meta);
		// tactic_levelup
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/tactic_levelup.png";
		assets.RegisterTexture(L"tactic_levelup", meta);
		// hp_font
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/hp_font.png";
		assets.RegisterTexture(L"hp_font", meta);
		// party_hpbarfront
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/party_hpbarfront.png";
		assets.RegisterTexture(L"party_hpbarfront", meta);
		// party_hpbarback
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/party_hpbarback.png";
		assets.RegisterTexture(L"party_hpbarback", meta);
		// leader_hpbarfront
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/leader_hpbarfront.png";
		assets.RegisterTexture(L"leader_hpbarfront", meta);
		// red_barback
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/red_barback.png";
		assets.RegisterTexture(L"red_barback", meta);
		// red_barfront
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/red_barfront.png";
		assets.RegisterTexture(L"red_barfront", meta);
		// waittime_barback1
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barback1.png";
		assets.RegisterTexture(L"waittime_barback1", meta);
		// waittime_barback2
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barback2.png";
		assets.RegisterTexture(L"waittime_barback2", meta);
		// waittime_barfront
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barfront.png";
		assets.RegisterTexture(L"waittime_barfront", meta);
		// waittime_barfull
		meta.fullPath = L"../bin/Resources/UI/Battle/bar_back/waittime_barfull.png";
		assets.RegisterTexture(L"waittime_barfull", meta);
		// input_outline
		meta.fullPath = L"../bin/Resources/UI/Battle/input/outline.png";
		assets.RegisterTexture(L"input_outline", meta);
		// input_barback
		meta.fullPath = L"../bin/Resources/UI/Battle/input/input_barback.png";
		assets.RegisterTexture(L"input_barback", meta);
		// timeline_ryza
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_ryza.png";
		assets.RegisterTexture(L"timeline_ryza", meta);
		// timeline_patricia
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_patricia.png";
		assets.RegisterTexture(L"timeline_patricia", meta);
		// timeline_klaudia
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_klaudia.png";
		assets.RegisterTexture(L"timeline_klaudia", meta);
		// timeline_npc
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_npc.png";
		assets.RegisterTexture(L"timeline_npc", meta);
		// timeline_leader
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_leader.png";
		assets.RegisterTexture(L"timeline_leader", meta);
		// timeline_enemy1
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_enemy1.png";
		assets.RegisterTexture(L"timeline_enemy1", meta);
		// timeline_enemy2
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_enemy2.png";
		assets.RegisterTexture(L"timeline_enemy2", meta);
		// timeline_enemy3
		meta.fullPath = L"../bin/Resources/UI/Battle/character_icon/timeline_enemy3.png";
		assets.RegisterTexture(L"timeline_enemy3", meta);

		// field_minimap
		meta.fullPath = L"../bin/Resources/UI/Field/minimap/field_minimap.png";
		assets.RegisterTexture(L"field_minimap", meta);
		// field_minimap_in
		meta.fullPath = L"../bin/Resources/UI/Field/minimap/field_minimap_in.png";
		assets.RegisterTexture(L"field_minimap_in", meta);
		// minimap_cursor
		meta.fullPath = L"../bin/Resources/UI/Field/minimap/minimap_cursor.png";
		assets.RegisterTexture(L"minimap_cursor", meta);
		// north
		meta.fullPath = L"../bin/Resources/UI/Field/minimap/north.png";
		assets.RegisterTexture(L"north", meta);
		// pm_am
		meta.fullPath = L"../bin/Resources/UI/Field/minimap/pm_am.png";
		assets.RegisterTexture(L"pm_am", meta);
		// night
		meta.fullPath = L"../bin/Resources/UI/Field/minimap/night.png";
		assets.RegisterTexture(L"night", meta);
	}

};

NS_END