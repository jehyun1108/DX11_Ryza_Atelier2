#pragma once

#include "UIArchetypeLoader.h"

NS_BEGIN(Client)

class UILoader
{
public:
	static void RegisterUIResources(AssetSystem* assets);
	static void InitFonts(SystemRegistry& registry);
	static void RegisterLoadingUI(AssetSystem* assets);
	static void RegisterOverlayUI(AssetSystem* assets);
	static void RegisterDressingUI(AssetSystem* assets);
	static void RegisterDressingTextures(AssetSystem* assets);

private:
	static void RegisterFieldUI(AssetSystem* assets);
	static void RegisterBattleUI(AssetSystem* assets);
	static void RegisterFont(AssetSystem* assets);
};

NS_END