#pragma once

NS_BEGIN(Client) // ±âº» ½ºÆå

//UILayer         layer         = UILayer::Widgets;
//UIContext       context       = UIContext::Battle;
//bool            useScissor    = false;
//bool            isInteractive = false;
//UIHitPolicy     hitPolicy     = UIHitPolicy::OpaqueRect;
//UISizeMode      sizeMode      = UISizeMode::Original;
//float           fixedWidth    = 0.f;
//float           fixedHeight   = 0.f;
//float           ratioX        = 1.f;
//float           ratioY        = 1.f;
//UIPivot         pivot         = UIPivot::MidCenter;
//UIAnchor        anchor        = UIAnchor::MidCenter;
//optional<float> initPosX;
//optional<float> initPosY;

class UIArchetypeLoader
{
public:
	inline static void RegisterUIArchetypes(UIRegistry& uiRegistry)
	{
		// 1. FatalDrive
		{
			UIArchetypeSpec spec;
			spec.layer         = UILayer::FatalDrive;
			spec.startEnabled  = false;

			spec.texKey = L"patricia_fataldrive";
			uiRegistry.RegisterArchetype(spec.texKey, spec);

			spec.texKey = L"ryza_fataldrive";
			uiRegistry.RegisterArchetype(spec.texKey, spec);

			spec.texKey = L"klaudia_fataldrive";
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		// 2. Battle_Character_UI
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"patricia_battleui";
			spec.initPosX    = -1125.f;
			spec.initPosY    =  185.f;
			spec.fixedWidth  =  241.f * 0.6f;
			spec.fixedHeight =  200.f * 0.6f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.initPosX = -1050.f;
			spec.initPosY =  415.f;
			spec.texKey   = L"ryza_battleui";
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.initPosX    = -1165.f;
			spec.initPosY    =  -15.f;
			spec.fixedWidth  =  241.f * 0.6f;
			spec.fixedHeight =  200.f * 0.6f;
			spec.texKey      = L"klaudia_battleui";
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		// 3. barback
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"leader_barback_bottom";
			spec.initPosX = -850.f;
			spec.initPosY =  560.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"leader_barback_top";
			spec.initPosX = -875.f;
			spec.initPosY =  500.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"ap_barback";
			spec.initPosX = -675.f;
			spec.initPosY =  425.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"leader_switching_barback";
			spec.initPosX = -1150.f;
			spec.initPosY =  575.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"battle_minimap";
			spec.initPosX = 1050.f;
			spec.initPosY = -500.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.startEnabled = false;
			spec.texKey       = L"aggressivemode";
			spec.initPosX     = 1050.f;
			spec.initPosY     = -325.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"supportmode";
			spec.initPosX = 1050.f;
			spec.initPosY = -325.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey   = L"timelinebar";
			spec.initPosX = 950.f;
			spec.initPosY = 550.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"tactic_barback";
			spec.initPosY    = -75.f;
			spec.fixedWidth  = WinX;
			spec.fixedHeight = 300.f;
			spec.startEnabled = false;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			//spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"tactic_levelup";
			spec.startEnabled = false;
			//spec.fixedWidth  = WinX;
			//spec.fixedHeight = 300.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode = UISizeMode::Fixed;
			spec.texKey      = L"hp_font";
			spec.fixedWidth  = 24.f * 1.5f;
			spec.fixedHeight = 16.f * 1.5f;
			spec.initPosX    = -950.f;
			spec.initPosY    =  510.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"party_hpbarfront";
			spec.initPosX = -1050.f;
			spec.initPosY = 275.f;
			uiRegistry.RegisterArchetype(L"party_hpbarfront1", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"party_hpbarback";
			spec.initPosX = -1050.f;
			spec.initPosY =  275.f;
			uiRegistry.RegisterArchetype(L"party_hpbarback1", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"party_hpbarfront";
			spec.initPosX = -1100.f;
			spec.initPosY =  75.f;
			uiRegistry.RegisterArchetype(L"party_hpbarfront2", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"party_hpbarback";
			spec.initPosX = -1100.f;
			spec.initPosY =  75.f;
			uiRegistry.RegisterArchetype(L"party_hpbarback2", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"party_hpbarfront";
			spec.fixedWidth  = 203.f * 1.3f;
			spec.fixedHeight = 20.f;
			spec.initPosX    = -800.f;
			spec.initPosY    = 512.f;
			uiRegistry.RegisterArchetype(L"leader_hpbarfront", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"party_hpbarback";
			spec.fixedWidth  = 217.f * 1.25f;
			spec.fixedHeight = 25.f;
			spec.initPosX    = -800.f;
			spec.initPosY    = 512.f;
			uiRegistry.RegisterArchetype(L"leader_hpbarback", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode = UISizeMode::Fixed;
			spec.texKey = L"red_barback";
			spec.fixedWidth = 160.f * 0.8f;
			spec.fixedHeight = 16.f;
			spec.initPosX = -880.f;
			spec.initPosY =  490.f;
			uiRegistry.RegisterArchetype(L"leader_redbarback", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode = UISizeMode::Fixed;
			spec.texKey = L"red_barfront";
			spec.fixedWidth = 144.f * 0.8f;
			spec.fixedHeight = 11.f;
			spec.initPosX = -880.f;
			spec.initPosY =  490.f;
			uiRegistry.RegisterArchetype(L"leader_redbarfront", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"red_barback";
			spec.initPosX = -1085.f;
			spec.initPosY =  255.f;
			uiRegistry.RegisterArchetype(L"party_redbarback1", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"red_barfront";
			spec.initPosX = -1085.f;
			spec.initPosY =  255.f;
			uiRegistry.RegisterArchetype(L"party_redbarfront1", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"red_barback";
			spec.initPosX = -1135.f;
			spec.initPosY =  55.f;
			uiRegistry.RegisterArchetype(L"party_redbarback2", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"red_barfront";
			spec.initPosX = -1135.f;
			spec.initPosY =  55.f;
			uiRegistry.RegisterArchetype(L"party_redbarfront2", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"waittime_barfront";
			spec.fixedWidth  = 182.f * 0.8f;
			spec.fixedHeight = 18.f * 0.8f;
			spec.initPosX    = -740.f;
			spec.initPosY    =  490.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode     = UISizeMode::Fixed;
			spec.texKey       = L"waittime_barfull";
			spec.fixedWidth   = 200.f * 0.8f;
			spec.fixedHeight  = 37.f * 0.8f;
			spec.initPosX     = -740.f;
			spec.initPosY     = 490.f;
			//spec.startEnabled = false;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"waittime_barback1";
			spec.fixedWidth  = 181.f * 0.8f;
			spec.fixedHeight = 18.f * 0.8f;
			spec.initPosX    = -740.f;
			spec.initPosY    = 490.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.sizeMode    = UISizeMode::Fixed;
			spec.texKey      = L"waittime_barback2";
			spec.fixedWidth  = 181.f * 0.8f;
			spec.fixedHeight = 23.f * 0.8f;
			spec.initPosX    = -740.f;
			spec.initPosY    = 490.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		 //Input
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_barback";
			spec.initPosX = -200.f;
			spec.initPosY = 350.f;
			uiRegistry.RegisterArchetype(L"input_barback1", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_outline";
			spec.initPosX = -200.f;
			spec.initPosY = 350.f;
			uiRegistry.RegisterArchetype(L"input_outline1", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_barback";
			spec.initPosX = 200.f;
			spec.initPosY = 350.f;
			uiRegistry.RegisterArchetype(L"input_barback2", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_outline";
			spec.initPosX = 200.f;
			spec.initPosY = 350.f;
			uiRegistry.RegisterArchetype(L"input_outline2", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_barback";
			spec.initPosX = -200.f;
			spec.initPosY = 425.f;
			uiRegistry.RegisterArchetype(L"input_barback3", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_outline";
			spec.initPosX = -200.f;
			spec.initPosY = 425.f;
			uiRegistry.RegisterArchetype(L"input_outline3", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_barback";
			spec.initPosX = 200.f;
			spec.initPosY = 425.f;
			uiRegistry.RegisterArchetype(L"input_barback4", spec);
		}
		{
			UIArchetypeSpec spec;
			spec.texKey = L"input_outline";
			spec.initPosX = 200.f;
			spec.initPosY = 425.f;
			uiRegistry.RegisterArchetype(L"input_outline4", spec);
		}

		 // 4. Field_minimap
		{
			UIArchetypeSpec spec;
			spec.context = UIContext::Field;
			spec.texKey  = L"field_minimap";
			spec.initPosX = 1050.f;
			spec.initPosY = 500.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.context = UIContext::Field;
			spec.texKey  = L"field_minimap_in";
			spec.initPosX = 1075.f;
			spec.initPosY = 510.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.context  = UIContext::Field;
			spec.texKey   = L"north";
			spec.initPosX = 1075.f;
			spec.initPosY = 375.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.context  = UIContext::Field;
			spec.texKey   = L"minimap_cursor";
			spec.initPosX = 1075.f;
			spec.initPosY = 510.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.context  = UIContext::Field;
			spec.texKey   = L"pm_am";
			spec.initPosX = 925.f;
			spec.initPosY = 610.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
		{
			UIArchetypeSpec spec;
			spec.context  = UIContext::Field;
			spec.texKey   = L"night";
			spec.initPosX = 930.f;
			spec.initPosY = 545.f;
			uiRegistry.RegisterArchetype(spec.texKey, spec);
		}
	}
};

NS_END