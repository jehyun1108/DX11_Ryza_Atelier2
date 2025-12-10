#pragma once

NS_BEGIN(Client) 

class UIArchetypeLoader
{
public:
	static void RegisterUIResources(UIRegistry* uiRegistry, UISystem* uiSys);
	static void RegisterLoadingUI(UIRegistry* ui);
	static void RegisterOverlayUI(UIRegistry* ui);

private:
	static void RegisterFieldUI(UIRegistry* ui, UISystem* uiSys);
	static void RegisterBattleUI(UIRegistry* ui);
	static void RegisterFontUI(UIRegistry* ui, UISystem* uiSys);
	static void RegisterDressingUI(UIRegistry* ui, UISystem* uiSys);
};

NS_END