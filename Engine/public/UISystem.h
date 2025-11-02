#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL UISystem
{
public:
	explicit UISystem(SystemRegistry& registry, AssetSystem& assets, UIRegistry& uiRegistry, UIAnimSystem& uiAnimSys) 
		: registry(registry), assets(assets), uiRegistry(uiRegistry), uiAnimSys(uiAnimSys) {}

	void Tick(float dt);

	void ExtractUIProxies(UISnapShot& out);

private:
	SystemRegistry& registry;
	AssetSystem& assets;
	UIRegistry&  uiRegistry;
	UIAnimSystem& uiAnimSys;
};

NS_END