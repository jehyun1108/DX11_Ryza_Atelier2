#include "pch.h"
#include "UIArchetypeLoader.h"

void UIArchetypeLoader::RegisterUIResources(UIRegistry* uiRegistry, UISystem* uiSys)
{
	RegisterFieldUI(uiRegistry, uiSys);
	RegisterBattleUI(uiRegistry);
	RegisterFontUI(uiRegistry, uiSys);
	RegisterDressingUI(uiRegistry, uiSys);
}

void UIArchetypeLoader::RegisterLoadingUI(UIRegistry* ui)
{
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::Loading;
		spec.zOrder = 10020;
		spec.texKey = L"now_loading";
		spec.initPosX = -50.f;
		spec.initPosY = 600.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::Loading;
		spec.zOrder = 10010;
		spec.texKey = L"loading_bg";
		spec.initPosX = 100.f;
		spec.initPosY = 490.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::Loading;
		spec.zOrder = 10020;
		spec.texKey = L"loading_dot";
		spec.initPosX = 100.f;
		spec.initPosY = 610.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	//{
	//	UIArchetypeSpec spec;
	//	spec.context = UIContext::Loading;
	//	spec.zOrder = 10000;
	//	spec.texKey = L"loading_back";
	//	spec.initPosY = 500.f;
	//	ui->RegisterArchetype(spec.texKey, spec);
	//}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 2;
		spec.context = UIContext::Logo;
		spec.initPosY = 50.f;
		spec.texKey = L"new_game_0";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"new_game_1";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder   = 2;
		spec.context  = UIContext::Logo;
		spec.initPosY = 125.f;
		spec.texKey   = L"load_game_0";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey   = L"load_game_1";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder   = 2;
		spec.context  = UIContext::Logo;
		spec.initPosY = 200.f;
		spec.texKey   = L"setting_0";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey   = L"setting_1";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder   = 2;
		spec.context  = UIContext::Logo;
		spec.initPosY = 275.f;
		spec.texKey   = L"exit_game_0";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey   = L"exit_game_1";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder   = 2;
		spec.context  = UIContext::Logo;
		spec.texKey   = L"press_any_button_0";
		spec.initPosY = 200.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder  = 1;
		spec.context = UIContext::Logo;
		spec.texKey  = L"menu_hoverbar";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder   = 1;
		spec.context  = UIContext::Logo;
		spec.texKey   = L"title";
		spec.initPosY = -250.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode    = UISizeMode::Fixed;
		spec.fixedWidth  = WinX;
		spec.fixedHeight = WinY;
		spec.zOrder      = 0;
		spec.texKey      = L"logo_bg";
		spec.context     = UIContext::Logo;
		ui->RegisterArchetype(spec.texKey, spec);
	}
}

void UIArchetypeLoader::RegisterFieldUI(UIRegistry* ui, UISystem* uiSys)
{
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::None;
		spec.zOrder = 103;
		spec.initPosY = -500.f;
		spec.texKey = L"battle_result";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"battle_result_under";
		spec.initPosY = -425.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::None;
		spec.zOrder = 103;
		spec.initPosX = -750.f;
		spec.initPosY = 100.f;
		spec.texKey = L"ryza_reward";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::None;
		spec.zOrder = 102;
		spec.initPosX = -175.f;
		spec.initPosY = 100.f;
		spec.texKey = L"klaudia_reward";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::None;
		spec.zOrder = 101;
		spec.initPosX = 400.f;
		spec.initPosY = 100.f;
		spec.texKey = L"patricia_reward";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::None;
		spec.zOrder = 100.f;
		spec.initPosX = 975.f;
		spec.initPosY = 100.f;
		spec.texKey = L"serri_reward";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context  = UIContext::Field;
		spec.zOrder   = 0;
		spec.texKey   = L"field_minimap";
		spec.initPosX = 1050.f;
		spec.initPosY = 500.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.maskType = UIMaskType::Circle;
		spec.context  = UIContext::Field;
		spec.texKey   = L"minimap_central";
		spec.zOrder   = 3;
		spec.initPosX = 1075.f;
		spec.initPosY = 510.f;
		ui->RegisterArchetype(L"minimap_central", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX   = 0.95f;
		spec.ratioY   = 0.95f;
		spec.maskType = UIMaskType::Circle;
		spec.context  = UIContext::Field;
		spec.texKey   = L"field_minimap_in";
		spec.zOrder   = 2;
		spec.initPosX = 1075.f;
		spec.initPosY = 510.f;
		ui->RegisterArchetype(L"field_minimap_in", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context  = UIContext::Field;
		spec.texKey   = L"north";
		spec.zOrder   = 10;
		spec.initPosX = 1075.f;
		spec.initPosY = 375.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context  = UIContext::Field;
		spec.texKey   = L"minimap_cursor";
		spec.zOrder   = 10;
		spec.initPosX = 1075.f;
		spec.initPosY = 510.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context  = UIContext::Field;
		spec.texKey   = L"pm_am";
		spec.zOrder   = 10;
		spec.initPosX = 925.f;
		spec.initPosY = 610.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context  = UIContext::Field;
		spec.texKey   = L"night";
		spec.zOrder   = 10;
		spec.initPosX = 930.f;
		spec.initPosY = 545.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX   = 0.15f;
		spec.ratioY   = 0.15f;
		spec.context  = UIContext::Field;
		spec.texKey   = L"minimap_enemy_icon";
		spec.zOrder   = 10;
		spec.startEnabled = false;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec; 
		spec.sizeMode = UISizeMode::Original;
		spec.context  = UIContext::Field;
		spec.texKey   = L"worldmap";
		spec.zOrder   = 100;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::Field;
		spec.zOrder = 110;
		// 신령한 용의 관
		spec.initPosX = -550.f;
		spec.initPosY = 40.f;
		spec.texKey = L"worldmap_icon_0";
		ui->RegisterArchetype(spec.texKey, spec);
		// 수저 성도
		spec.texKey = L"worldmap_icon_1";
		spec.initPosX = -850.f;
		spec.initPosY = -225.f;
		ui->RegisterArchetype(spec.texKey, spec);
		// 전승의 용골 협곡
		spec.texKey = L"worldmap_icon_2";
		spec.initPosX = -400.f;
		spec.initPosY = -500.f;
		ui->RegisterArchetype(spec.texKey, spec);
		// 고대 마나 공방
		spec.texKey = L"worldmap_icon_3";
		spec.initPosX = -200.f;
		spec.initPosY = 500.f;
		ui->RegisterArchetype(spec.texKey, spec);
		// 환상의 땅
		spec.texKey = L"worldmap_icon_4";
		spec.initPosX = 700.f;
		spec.initPosY = -475.f;
		ui->RegisterArchetype(spec.texKey, spec);
		// 지하소녀의 묘지
		spec.texKey = L"worldmap_icon_5";
		spec.initPosX = 400.f;
		spec.initPosY = -200.f;
		ui->RegisterArchetype(spec.texKey, spec);
		// ==============================================================
		spec.zOrder = 111;
		spec.texKey = L"worldmap_clickicon";
		// 신령한 용의관(버튼)
		spec.initPosX = -550.f;
		spec.initPosY = 75.f;
		ui->RegisterArchetype(L"worldmap_clickicon_0", spec);
		// 수지성도 (버튼)
		spec.initPosX = -855.f;
		spec.initPosY = -200.f;
		ui->RegisterArchetype(L"worldmap_clickicon_1", spec);
		// 전승의 용골 협곡 (버튼)
		spec.initPosX = -425.f;
		spec.initPosY = -475.f;
		ui->RegisterArchetype(L"worldmap_clickicon_2", spec);
		// 고대 마나 공방 (버튼)
		spec.initPosX = -200.f;
		spec.initPosY = 475.f;
		ui->RegisterArchetype(L"worldmap_clickicon_3", spec);
		// 환상의 땅 (버튼)
		spec.initPosX = 690.f;
		spec.initPosY = -450.f;
		ui->RegisterArchetype(L"worldmap_clickicon_4", spec);
		// 지하소녀의 묘지 (버튼)
		spec.initPosX =  375.f;
		spec.initPosY = -150.f;
		ui->RegisterArchetype(L"worldmap_clickicon_5", spec);
		// 왕도 아슬라 암 버트 (버튼)
		spec.initPosX = -965.f;
		spec.initPosY = 190.f;
		ui->RegisterArchetype(L"worldmap_clickicon_6", spec);
		// 왕도 남쪽 (버튼)
		spec.initPosX = -950.f;
		spec.initPosY = 450.f;
		ui->RegisterArchetype(L"worldmap_clickicon_7", spec);
		// 왕도 근교 (버튼)
		spec.initPosX = -50.f;
		spec.initPosY = 75.f;
		ui->RegisterArchetype(L"worldmap_clickicon_8", spec);
		// 바람이 부는 골짜기 (버튼)
		spec.initPosX = 650.f;
		spec.initPosY = 25.f;
		ui->RegisterArchetype(L"worldmap_clickicon_9", spec);
		// ===============================================================
		spec.zOrder = 112;
		float offsetY = 75.f;
		spec.texKey = L"worldmap_barback";
		// 신령한 용의관(barback)
		spec.initPosX = -550.f;
		spec.initPosY =  75.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_0", spec);
		// 수지성도 (barback)
		spec.initPosX = -855.f;
		spec.initPosY = -200.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_1", spec);
		// 전승의 용골 협곡 (barback)
		spec.initPosX = -425.f;
		spec.initPosY = -475.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_2", spec);
		// 고대 마나 공방 (barback)
		spec.initPosX = -200.f;
		spec.initPosY =  475.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_3", spec);
		// 환상의 땅 (barback)
		spec.initPosX =  690.f;
		spec.initPosY = -450.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_4", spec);
		// 지하소녀의 묘지 (barback)
		spec.initPosX =  375.f;
		spec.initPosY = -150.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_5", spec);
		// 왕도 아슬라 암 버트 (barback)
		spec.initPosX = -965.f;
		spec.initPosY =  190.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_6", spec);
		// 왕도 남쪽 (barback)
		spec.initPosX = -950.f;
		spec.initPosY =  450.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_7", spec);
		// 왕도 근교 (barback)
		spec.initPosX = -50.f;
		spec.initPosY = 75.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_8", spec);
		// 바람이 부는 골짜기 (barback)
		spec.initPosX = 650.f;
		spec.initPosY = 25.f + offsetY;
		ui->RegisterArchetype(L"worldmap_barback_9", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context = UIContext::Field;

		spec.zOrder   = 110;
		spec.initPosX = -1150.f;
		spec.initPosY = 450.f;
		spec.texKey   = L"worldmap_character_back";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.zOrder   = 120;
		spec.initPosX = -800.f;
		spec.initPosY = 470.f;
		spec.texKey   = L"worldmap_dialogue";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.context  = UIContext::Field;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX   = 0.4f;
		spec.ratioY   = 0.4f;
		spec.initPosX = -1150.f;
		spec.initPosY = 450.f;
		spec.zOrder   = 120;
		spec.texKey   = L"worldmap_patricia";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 140;
		spec.context = UIContext::Field;
		spec.texKey = L"worldmap_select";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 0.4f;
		spec.ratioY = 0.45f;
		spec.zOrder = 140;
		spec.context = UIContext::Field;
		spec.texKey = L"field_select_highlight";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX   = 0.6f;
		spec.ratioY   = 2.f;
		spec.zOrder   = 130;
		spec.initPosX = 785.f;
		spec.initPosY = 140.f;
		spec.context  = UIContext::Field;
		spec.texKey   = L"field_selectbar";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 140;
		spec.initPosY = -500.f;
		spec.context = UIContext::Field;
		spec.texKey = L"maptitle_barback";
		spec.startEnabled = false;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 139;
		spec.initPosY = -500.f;
		spec.context = UIContext::Field;
		spec.texKey = L"maptitle_spinner";
		spec.startEnabled = false;
		ui->RegisterArchetype(spec.texKey, spec);
	}
}

void UIArchetypeLoader::RegisterBattleUI(UIRegistry* ui)
{
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.initPosX = 1175.f;
		spec.initPosY = -100.f;
		spec.zOrder = 101;
		spec.texKey = L"chain";
		ui->RegisterArchetype(L"chain", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.initPosX = 1075.f;
		spec.initPosY = -50.f;
		spec.zOrder = 101;
		spec.texKey = L"chain_x";
		ui->RegisterArchetype(L"chain_x", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.initPosX = 1185.f;
		spec.initPosY = -50.f;
		spec.zOrder = 101;
		spec.texKey = L"chain_damage";
		ui->RegisterArchetype(L"chain_damage", spec);
	}
	{
		UIArchetypeSpec spec;
		
		spec.zOrder   = 100;
		spec.initPosX = 1125.f;
		spec.initPosY = 50.f;
		spec.texKey   = L"battle_damage_glow";
		ui->RegisterArchetype(L"dmgGlow_left", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.flipMode = UIFlipMode::FlipX;
		spec.zOrder   = 100;
		spec.initPosX = -1125.f;
		spec.initPosY = 50.f;
		spec.texKey = L"battle_damage_glow";
		ui->RegisterArchetype(L"dmgGlow_right", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.fillMode = UIFillMode::RingCW;
		spec.sizeMode = UISizeMode::Ratio;
		spec.zOrder   = 1000;
		spec.initPosX = -1140.f;
		spec.initPosY = 570.f;
		spec.ratioX   = 0.45f;
		spec.ratioY   = 0.45f;
		spec.texKey   = L"serri";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	// 1. FatalDrive
	{
		UIArchetypeSpec spec;
		spec.zOrder = 11000;
		spec.startEnabled = false;

		spec.texKey = L"patricia_fataldrive";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"ryza_fataldrive";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"klaudia_fataldrive";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.initPosX = -765.f;
		spec.initPosY = 300.f;
		spec.startEnabled = false;
		spec.texKey = L"fataldrive_barfront";
		spec.fillMode = UIFillMode::RingCW;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.initPosX = -765.f;
		spec.initPosY = 300.f;
		spec.ratioX = 0.2f;
		spec.ratioY = 0.2f;
		spec.texKey = L"fataldrive_barback";
		spec.startEnabled = false;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 1;
		spec.startEnabled = false;
		spec.initPosX = -850.f;
		spec.initPosY = 350.f;
		spec.texKey = L"fataldrive_fatal";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.initPosX = -760.f;
		spec.initPosY = 350.f;
		spec.zOrder = 0;
		spec.texKey = L"fataldrive_redlight";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.zOrder = 1;
		spec.initPosX = -675.f;
		spec.initPosY = 350.f;
		spec.texKey = L"fataldrive_drive";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 10000;
		spec.sizeMode = UISizeMode::Fixed;
		spec.fixedWidth = WinX;
		spec.fixedHeight = WinY * 2.f;
		spec.texKey = L"fataldrive_bg";

		spec.initPosX = WinX * -0.5f;
		ui->RegisterArchetype(L"fataldrive_bg_left", spec);

		spec.initPosX = WinX * 0.5f;
		spec.flipMode = UIFlipMode::FlipX;
		ui->RegisterArchetype(L"fataldrive_bg_right", spec);
	}
	// dmgletter
	{
		UIArchetypeSpec spec;
		spec.zOrder = 99999;
		spec.startEnabled = false;
		for (int i = 0; i <= 9; ++i)
		{
			spec.texKey = L"dmgletter_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
	}      
	{
		UIArchetypeSpec spec;
		spec.zOrder = 99999;
		spec.startEnabled = false;
		spec.sizeMode = UISizeMode::Ratio;
		for (int i = 0; i <= 9; ++i)
		{
			spec.texKey = L"battleletter_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 99999;
		spec.startEnabled = false;
		for (int i = 0; i <= 9; ++i)
		{
			spec.texKey = L"battleletter_large_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
	}
	{
		UIArchetypeSpec spec;
		spec.zOrder = 99999;
		spec.startEnabled = false;
		for (int i = 0; i <= 9; ++i)
		{
			spec.texKey = L"chainletter_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.texKey = L"total_damage";
		ui->RegisterArchetype(spec.texKey, spec);
	}

	// 2. Battle_Character_UI
	{
		UIArchetypeSpec spec;
		spec.zOrder = 1;
		spec.texKey = L"patricia_battleui";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"ryza_battleui";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"klaudia_battleui";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	// 3. barback
	{
		UIArchetypeSpec spec;
		spec.texKey = L"leader_barback_bottom";
		spec.initPosX = -850.f;
		spec.initPosY = 560.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		float offsetX = 68.f;
		UIArchetypeSpec spec;
		spec.initPosX = -948.f;
		spec.initPosY = 562.f;

		spec.texKey = L"tactic_barfront1";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"tactic_barfull";
		ui->RegisterArchetype(L"tactic_barfull1", spec);

		spec.texKey = L"tactic_barfront2";
		spec.initPosX = -950.f + offsetX;
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"tactic_barfull";
		ui->RegisterArchetype(L"tactic_barfull2", spec);

		spec.texKey = L"tactic_barfront3";
		spec.initPosX = -950.f + offsetX * 2.f;
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"tactic_barfull";
		ui->RegisterArchetype(L"tactic_barfull3", spec);

		spec.texKey = L"tactic_barfront4";
		spec.initPosX = -950.f + offsetX * 3.f;
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"tactic_barfull";
		ui->RegisterArchetype(L"tactic_barfull4", spec);

		spec.texKey = L"tactic_barfront5";
		spec.initPosX = -950.f + offsetX * 4.f;
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"tactic_barfull";
		ui->RegisterArchetype(L"tactic_barfull5", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"leader_barback_top";
		spec.initPosX = -875.f;
		spec.initPosY = 500.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"ap_barback";
		spec.initPosX = -675.f;
		spec.initPosY = 425.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"leader_switching_barback";
		spec.initPosX = -1150.f;
		spec.initPosY = 575.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"battle_minimap";
		spec.initPosX = 1050.f;
		spec.initPosY = -500.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.initPosX = 1050.f;
		spec.initPosY = -325.f;

		spec.texKey = L"aggressivemode";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"supportmode";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"timelinebar";
		spec.initPosX = 950.f;
		spec.initPosY = 550.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Fixed;
		spec.texKey = L"tactic_barback";
		spec.initPosY = -75.f;
		spec.fixedWidth = WinX;
		spec.fixedHeight = 300.f;
		spec.startEnabled = false;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"tactic_levelup";
		spec.fixedWidth = WinX;
		spec.fixedHeight = 300.f;
		spec.startEnabled = false;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.texKey = L"hp_font";
		spec.ratioX = 1.25f;
		spec.ratioY = 1.25f;
		spec.initPosX = -950.f;
		spec.initPosY = 510.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.initPosX = -1050.f;
		spec.initPosY = 275.f;

		spec.texKey = L"party_hpbarfront";
		ui->RegisterArchetype(L"party_hpbarfront1", spec);
		spec.texKey = L"party_hpbarback";
		ui->RegisterArchetype(L"party_hpbarback1", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.initPosX = -1100.f;
		spec.initPosY = 75.f;

		spec.texKey = L"party_hpbarfront";
		ui->RegisterArchetype(L"party_hpbarfront2", spec);
		spec.texKey = L"party_hpbarback";
		ui->RegisterArchetype(L"party_hpbarback2", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.initPosX = -1085.f;
		spec.initPosY = 255.f;

		spec.texKey = L"red_barback";
		ui->RegisterArchetype(L"party_redbarback1", spec);
		spec.texKey = L"red_barfront";
		ui->RegisterArchetype(L"party_redbarfront1", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.initPosX = -1135.f;
		spec.initPosY = 55.f;

		spec.texKey = L"red_barback";
		ui->RegisterArchetype(L"party_redbarback2", spec);
		spec.texKey = L"red_barfront";
		ui->RegisterArchetype(L"party_redbarfront2", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 1.3f;
		spec.initPosX = -800.f;
		spec.initPosY = 512.f;

		spec.texKey = L"party_hpbarfront";
		ui->RegisterArchetype(L"leader_hpbarfront", spec);
		spec.texKey = L"party_hpbarback";
		ui->RegisterArchetype(L"leader_hpbarback", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 0.8f;
		spec.initPosX = -880.f;
		spec.initPosY = 490.f;

		spec.texKey = L"red_barback";
		ui->RegisterArchetype(L"leader_redbarback", spec);
		spec.texKey = L"red_barfront";
		ui->RegisterArchetype(L"leader_redbarfront", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"pinch";
		spec.initPosX = -950.f;
		spec.initPosY = 225.f;
		ui->RegisterArchetype(L"pinch_1", spec);

		spec.initPosX = -1025.f;
		spec.initPosY = 25.f;
		ui->RegisterArchetype(L"pinch_2", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 0.8f;
		spec.ratioY = 0.8f;
		spec.initPosX = -740.f;
		spec.initPosY = 490.f;

		spec.texKey = L"waittime_barfront";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"waittime_barfull";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"waittime_barback1";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"waittime_barback2";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"tactic_barback";
		spec.sizeMode = UISizeMode::Fixed;
		spec.fixedWidth = 500.f;
		spec.fixedHeight = 50.f;

		spec.initPosX = -100.f;
		spec.initPosY = 350.f;
		ui->RegisterArchetype(L"input_barback1", spec);

		spec.initPosX = 425.f;
		spec.initPosY = 350.f;
		ui->RegisterArchetype(L"input_barback2", spec);

		spec.initPosX = -100.f;
		spec.initPosY = 425.f;
		ui->RegisterArchetype(L"input_barback3", spec);

		spec.initPosX = 425.f;
		spec.initPosY = 425.f;
		ui->RegisterArchetype(L"input_barback4", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"input_outline";
		spec.sizeMode = UISizeMode::Fixed;
		spec.fixedWidth = 500.f;
		spec.fixedHeight = 50.f;

		spec.initPosX = -100.f;
		spec.initPosY = 350.f;
		ui->RegisterArchetype(L"input_outline1", spec);

		spec.initPosX = 425.f;
		spec.initPosY = 350.f;
		ui->RegisterArchetype(L"input_outline2", spec);

		spec.initPosX = -100.f;
		spec.initPosY = 425.f;
		ui->RegisterArchetype(L"input_outline3", spec);

		spec.initPosX = 425.f;
		spec.initPosY = 425.f;
		ui->RegisterArchetype(L"input_outline4", spec);
	}
	{
		array<_float2, 4> pos = {
			_float2{-310.f, 350.f},
			_float2{-310.f, 425.f},
			_float2{ 215.f, 350.f},
			_float2{ 215.f, 425.f},
		};
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 0.4f;
		spec.ratioY = 0.4f;

		spec.texKey = L"defend_icon";
		spec.initPosX = pos[0].x;
		spec.initPosY = pos[0].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"attack_icon";
		spec.initPosX = pos[1].x;
		spec.initPosY = pos[1].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"itemrush_icon";
		spec.initPosX = pos[2].x;
		spec.initPosY = pos[2].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"flee_icon";
		spec.initPosX = pos[3].x;
		spec.initPosY = pos[3].y;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		array<_float2, 4> pos = {
			_float2{ -260.f, 350.f },
			_float2{ -260.f, 425.f },
			_float2{ 265.f, 350.f },
			_float2{ 265.f, 425.f },
		};

		UIArchetypeSpec spec;
		spec.texKey = L"input_y";
		spec.initPosX = pos[0].x;
		spec.initPosY = pos[0].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"input_b";
		spec.initPosX = pos[1].x;
		spec.initPosY = pos[1].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"input_x";
		spec.initPosX = pos[2].x;
		spec.initPosY = pos[2].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"input_a";
		spec.initPosX = pos[3].x;
		spec.initPosY = pos[3].y;
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey      = L"input_highlight_circle";
		spec.sizeMode    = UISizeMode::Fixed;
		spec.fixedWidth  = 50.f;
		spec.fixedHeight = 50.f;

		spec.imageColor = _float4(1.0f, 0.95f, 0.60f, 1.0f);
		spec.initPosX   = pos[0].x;
		spec.initPosY   = pos[0].y;
		ui->RegisterArchetype(L"input_highlight_circle_yellow", spec);

		spec.imageColor = _float4(1.0f, 0.60f, 0.60f, 1.0f);
		spec.initPosX = pos[1].x;
		spec.initPosY = pos[1].y;
		ui->RegisterArchetype(L"input_highlight_circle_red", spec);

		spec.imageColor = _float4(0.60f, 0.80f, 1.0f, 1.0f);
		spec.initPosX = pos[2].x;
		spec.initPosY = pos[2].y;
		ui->RegisterArchetype(L"input_highlight_circle_blue", spec);

		spec.imageColor = _float4(0.60f, 1.0f, 0.70f, 1.0f);
		spec.initPosX = pos[3].x;
		spec.initPosY = pos[3].y;
		ui->RegisterArchetype(L"input_highlight_circle_green", spec);
	}
	{
		float offsetX = -100.f;
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 0.25f;
		spec.ratioY = 0.25f;
		spec.startEnabled = false;

		spec.initPosX = -200.f + offsetX;
		spec.initPosY = 350.f;
		spec.texKey = L"highlight_y";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.initPosX = 200.f + offsetX;
		spec.initPosY = 350.f;
		spec.texKey = L"highlight_b";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.initPosX = -200.f + offsetX;
		spec.initPosY = 425.f;
		spec.texKey = L"highlight_x";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.initPosX = 200.f + offsetX;
		spec.initPosY = 425.f;
		spec.texKey = L"highlight_a";
		ui->RegisterArchetype(spec.texKey, spec);
	}
	// FATAL + DRIVE
	{
		UIArchetypeSpec spec;
		spec.zOrder = 12000;
		spec.texKey = L"fataldrive_f";
		spec.initPosX = -750.f;
		spec.initPosY = 100.f;
		ui->RegisterArchetype(L"fataldrive_1", spec);

		spec.texKey = L"fataldrive_a";
		spec.initPosX = -625.f;
		spec.initPosY = 125.f;
		ui->RegisterArchetype(L"fataldrive_2", spec);

		spec.texKey = L"fataldrive_t";
		spec.initPosX = -475.f;
		spec.initPosY = 100.f;
		ui->RegisterArchetype(L"fataldrive_3", spec);

		spec.texKey = L"fataldrive_a";
		spec.initPosX = -360.f;
		spec.initPosY = 125.f;
		ui->RegisterArchetype(L"fataldrive_4", spec);

		spec.texKey = L"fataldrive_l";
		spec.initPosX = -200.f;
		spec.initPosY = 100.f;
		ui->RegisterArchetype(L"fataldrive_5", spec);

		spec.texKey = L"fataldrive_d";
		spec.initPosX = 100.f;
		spec.initPosY = 100.f;
		ui->RegisterArchetype(L"fataldrive_6", spec);

		spec.texKey = L"fataldrive_r";
		spec.initPosX = 300.f;
		spec.initPosY = 125.f;
		ui->RegisterArchetype(L"fataldrive_7", spec);

		spec.texKey = L"fataldrive_i";
		spec.initPosX = 405.f;
		spec.initPosY = 105.f;
		ui->RegisterArchetype(L"fataldrive_8", spec);

		spec.texKey = L"fataldrive_v";
		spec.initPosX = 530.f;
		spec.initPosY = 105.f;
		ui->RegisterArchetype(L"fataldrive_9", spec);

		spec.texKey = L"fataldrive_e";
		spec.initPosX = 680.f;
		spec.initPosY = 105.f;
		ui->RegisterArchetype(L"fataldrive_10", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.zOrder = 100;
		for (int i = 1; i <= 4; ++i)
		{
			spec.texKey = L"ryza_skill_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
		for (int i = 1; i <= 4; ++i)
		{
			spec.texKey = L"patricia_skill_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
		for (int i = 1; i <= 4; ++i)
		{
			spec.texKey = L"klaudia_skill_" + to_wstring(i);
			ui->RegisterArchetype(spec.texKey, spec);
		}
	}
	/*{
		UIArchetypeSpec spec;
		spec.zOrder = 1;
		spec.texKey = L"change_character";

		spec.initPosX = -1150.f;
		spec.initPosY = -120.f;
		ui->RegisterArchetype(L"change_character_1", spec);
		spec.initPosX = -950.f;
		spec.initPosY = -120.f;
		ui->RegisterArchetype(L"change_character_2", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.initPosX = -1050.f;
		spec.initPosY = -120.f;
		spec.texKey = L"change_character_back";
		ui->RegisterArchetype(spec.texKey, spec);
	}*/
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 1.6f;
		spec.ratioY = 1.6f;
		spec.zOrder = 10000;
		spec.imageColor = _float4(0.55f, 0.95f, 0.75f, 1.0f);
		spec.texKey = L"leader_icon_highlight";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.startEnabled = false;
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.texKey = L"enemy_A";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"enemy_B";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"enemy_C";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"target_cursor";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"target_barback";
		ui->RegisterArchetype(L"target_barback", spec);
		spec.texKey = L"target_hp_barback";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"target_hp_barfront";
		ui->RegisterArchetype(spec.texKey, spec);
		spec.texKey = L"target_letter";
		ui->RegisterArchetype(spec.texKey, spec);

		spec.texKey = L"target_timeline_ring_front";
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 1.2f;
		spec.ratioY = 1.2f;
		spec.fillMode = UIFillMode::RingCW;
		ui->RegisterArchetype(spec.texKey, spec);
		//spec.texKey = L"target_timeline_ring_back";
		//ui->RegisterArchetype(spec.texKey, spec);
		
		spec.texKey = L"timeline_npc";
		spec.sizeMode = UISizeMode::Ratio;
		spec.ratioX = 0.4f;
		spec.ratioY = 0.4f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"dummy";
		spec.startEnabled = false;
		spec.sizeMode = UISizeMode::Fixed;
		spec.fixedWidth = 50.f;
		spec.fixedHeight = 50.f;
		ui->RegisterArchetype(L"battle_board_icon_base", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.texKey = L"supportmode";
		spec.initPosX = 1050.f;
		spec.initPosY = -325.f;
		ui->RegisterArchetype(spec.texKey, spec);
	}
}

void UIArchetypeLoader::RegisterFontUI(UIRegistry* ui, UISystem* uiSys)
{
	UIArchetypeSpec spec{};
	spec.widgetType   = UIWidgetType::Text;
	spec.context      = UIContext::Field;
	spec.anchor       = UIAnchor::MidCenter;
	spec.pivot        = UIPivot::MidCenter;
	spec.zOrder       = 999;
	spec.startEnabled = false;

	spec.fontKey  = L"GmarketSansTTFMedium_24";       
	ui->RegisterArchetype(L"font_24", spec);

	spec.fontKey  = L"GmarketSansTTFMedium_28";
	ui->RegisterArchetype(L"font_28", spec);

	spec.fontKey  = L"GmarketSansTTFMedium_32";
	ui->RegisterArchetype(L"font_32", spec);

	spec.fontKey = L"GmarketSansTTFMedium_36";
	ui->RegisterArchetype(L"font_36", spec);

	spec.fontKey = L"GmarketSansTTFMedium_40";
	ui->RegisterArchetype(L"font_40", spec);

	spec.fontKey = L"GmarketSansTTFMedium_44";
	ui->RegisterArchetype(L"font_44", spec);

	spec.fontKey = L"GmarketSansTTFMedium_48";
	ui->RegisterArchetype(L"font_48", spec);

	spec.fontKey      = L"GmarketSansTTFMedium_32";
	ui->RegisterArchetype(L"worldmap_label", spec);

	spec.fontKey = L"GmarketSansTTFMedium_32";
	spec.initPosX = -800.f;
	spec.initPosY =  500.f;
	ui->RegisterArchetype(L"worldmap_introtext", spec);

	spec.fontKey = L"GmarketSansTTFMedium_40";
	spec.initPosX = 720.f;
	spec.initPosY = 150.f;
	ui->RegisterArchetype(L"worldmap_select_yes", spec);
	uiSys->SetText(L"worldmap_select_yes", L"예");
	ui->Ensure(L"worldmap_select_yes").selfEnabled = false;

	spec.initPosX = 760.f;
	spec.initPosY = 205.f;
	ui->RegisterArchetype(L"worldmap_select_no", spec);
	uiSys->SetText(L"worldmap_select_no", L"아니요");
	ui->Ensure(L"worldmap_select_no").selfEnabled = false;

	spec.initPosX = 0.f;
	spec.initPosY = -470.f;
	ui->RegisterArchetype(L"maptitle_text", spec);
	ui->Ensure(L"maptitle_text").selfEnabled = false;

	// ==============================================
	spec.widgetType = UIWidgetType::Text;
	spec.context    = UIContext::Battle;  
	spec.anchor     = UIAnchor::MidLeft;    
	spec.pivot      = UIPivot::MidLeft;
	spec.zOrder     = 999;
	spec.startEnabled = false;
	spec.alignH = UITextAlignHorizontal::Left;

	spec.fontKey = L"GmarketSansTTFMedium_32";
	ui->RegisterArchetype(L"battle_input_label", spec);

	{
		UIArchetypeSpec spec{};
		spec.fontKey      = L"GmarketSansTTFMedium_28";
		spec.startEnabled = false;
		spec.widgetType   = UIWidgetType::Text;
		spec.context      = UIContext::Field;
		spec.anchor       = UIAnchor::MidLeft;
		spec.pivot        = UIPivot::MidLeft;
		spec.zOrder       = 10999;
		spec.alignH       = UITextAlignHorizontal::Left;
		spec.initPosX     = 200.f;
		spec.initPosY     = -325.f;
		float offsetY     = 50.f;
		ui->RegisterArchetype(L"dressing_tab_ryza_name", spec);
		spec.initPosY.value() += offsetY;
		ui->RegisterArchetype(L"dressing_tab_klaudia_name", spec);
		spec.initPosY.value() += offsetY;
		ui->RegisterArchetype(L"dressing_tab_patricia_name", spec);
	}
	{
		UIArchetypeSpec spec{};
		spec.fontKey      = L"GmarketSansTTFMedium_24";
		spec.startEnabled = false;
		spec.widgetType   = UIWidgetType::Text;
		spec.context      = UIContext::Field;
		spec.anchor       = UIAnchor::MidCenter;
		spec.pivot        = UIPivot::MidCenter;
		spec.zOrder       = 10999;
		spec.alignH       = UITextAlignHorizontal::Center;
		spec.initPosX     = -700.f;
		spec.initPosY     = -330.f;
		float offsetY     = 50.f;
		ui->RegisterArchetype(L"dressing_tab_ryza_equip", spec);
		spec.initPosY.value() += offsetY;
		ui->RegisterArchetype(L"dressing_tab_klaudia_equip", spec);
		spec.initPosY.value() += offsetY;
		ui->RegisterArchetype(L"dressing_tab_patricia_equip", spec);
	}
	{
		UIArchetypeSpec spec{};
		spec.fontKey      = L"GmarketSansTTFMedium_28";
		spec.startEnabled = false;
		spec.widgetType   = UIWidgetType::Text;
		spec.context      = UIContext::Field;
		spec.anchor       = UIAnchor::MidLeft;
		spec.pivot        = UIPivot::MidLeft;
		spec.zOrder       = 10999;
		spec.alignH       = UITextAlignHorizontal::Left;
		spec.initPosX     = 250.f;
		spec.initPosY     = 125.f;
		ui->RegisterArchetype(L"dressing_row_text_base", spec);
	}
	{
		UIArchetypeSpec spec{};
		spec.fontKey      = L"GmarketSansTTFMedium_24";
		spec.startEnabled = false;
		spec.widgetType   = UIWidgetType::Text;
		spec.context      = UIContext::Battle;
		spec.anchor       = UIAnchor::MidCenter;
		spec.pivot        = UIPivot::MidCenter;
		spec.initPosX     = 0;
		spec.initPosY     = 0;
		spec.zOrder       = 10999;
		ui->RegisterArchetype(L"battle_hp_text_base", spec);
	}
	{
		UIArchetypeSpec spec{};
		spec.fontKey      = L"GmarketSansTTFMedium_24";
		spec.startEnabled = false;
		spec.widgetType   = UIWidgetType::Text;
		spec.context      = UIContext::Battle;
		spec.anchor       = UIAnchor::MidCenter;
		spec.pivot        = UIPivot::MidCenter;
		spec.initPosX     = 1120.f;
		spec.initPosY     = -30.f;
		spec.zOrder       = 10999;
		ui->RegisterArchetype(L"battle_chain_mul", spec);
	}
}

void UIArchetypeLoader::RegisterDressingUI(UIRegistry* ui, UISystem* uiSys)
{
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.zOrder      = 9999;
		spec.sizeMode    = UISizeMode::Fixed;
		spec.context     = UIContext::Field;
		spec.fixedWidth  = WinX;
		spec.fixedHeight = WinY;
		spec.texKey = L"dressing_bg";
		ui->RegisterArchetype(L"dressing_bg", spec);
	}
	{
		UIArchetypeSpec spec;    
		spec.startEnabled = false;
		spec.zOrder = 10010;
		spec.context = UIContext::Field;
		spec.initPosX = -1075.f;
		spec.initPosY = 100.f;
		spec.texKey = L"dressing_circle";
		ui->RegisterArchetype(L"dressing_row_circle_base", spec);

		spec.zOrder = 10011;
		spec.texKey = L"dressing_select";
		ui->RegisterArchetype(L"dressing_row_check_base", spec);

		spec.zOrder = 10020;
		spec.texKey = L"dressing_select_highlight";
		ui->RegisterArchetype(L"dressing_select_highlight", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.startEnabled = false;
		spec.sizeMode = UISizeMode::Ratio;
		spec.context = UIContext::Field;
		spec.ratioX = 2.5f;
		spec.ratioY = 1.3f;
		spec.zOrder = 10001;
		spec.initPosX = -700.f;
		spec.initPosY = -350.f;
		spec.texKey = L"dressing_tab_equipbar";
		ui->RegisterArchetype(L"dressing_tab_equipbar_base", spec);

		spec.ratioX = 1.8f;
		spec.ratioY = 1.f;
		spec.initPosX = -850.f;
		spec.zOrder = 10002;
		spec.texKey = L"dressing_select_highlight";
		ui->RegisterArchetype(L"dressing_tab_highlight_base", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode = UISizeMode::Ratio;
		spec.startEnabled = false;
		spec.context = UIContext::Field;
		spec.zOrder  = 10002;
		spec.texKey  = L"dressing_divider";
		spec.ratioX = 1.3f;

		spec.initPosX = -800.f;
		spec.initPosY = -325.f;  
		ui->RegisterArchetype(L"dressing_divider_tab_base", spec);

		spec.initPosY = 125.f;
		ui->RegisterArchetype(L"dressing_divider_list_base", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode     = UISizeMode::Fixed;
		spec.startEnabled = false;
		spec.context      = UIContext::Field;
		spec.zOrder       = 10000;
		spec.texKey       = L"dressing_paper";

		spec.fixedWidth  = 700.f;
		spec.fixedHeight = 400.f;
		spec.initPosX    = -800.f;
		spec.initPosY    = -200.f;

		ui->RegisterArchetype(L"dressing_paper_tab", spec);

		spec.fixedWidth  = 700.f;
		spec.fixedHeight = 350.f;
		spec.initPosX    = -800.f;
		spec.initPosY    = 200.f;
		ui->RegisterArchetype(L"dressing_paper_list", spec);
	}
	{
		UIArchetypeSpec spec;
		spec.sizeMode    = UISizeMode::Fixed;
		spec.fixedWidth  = 1440.f;
		spec.fixedHeight = 1440.f;
		spec.context     = UIContext::Field;
		spec.zOrder      = 100010;
		spec.texKey      = L"dressing_char_rt";
		ui->RegisterArchetype(L"dressing_char_rt", spec);
	}
}

void UIArchetypeLoader::RegisterOverlayUI(UIRegistry* ui)
{
	{
		UIArchetypeSpec spec;
		spec.zOrder       = 9999;
		spec.sizeMode     = UISizeMode::Fixed;
		spec.context      = UIContext::Always;
		spec.fixedWidth   = WinX + 20;
		spec.fixedHeight  = WinY + 20;


		spec.texKey = L"black";
		ui->RegisterArchetype(L"black", spec);

		spec.startEnabled = false;

		spec.texKey = L"white";
		ui->RegisterArchetype(L"white", spec);
	}
}
