#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL UISystem : public ISystem
{
public:
	explicit UISystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Tick(float dt);

	void ExtractUIProxies(UISnapShot& out);

private:
	SystemRegistry&         registry;
	AssetSystem*            assets{};
	UIRegistry*             uiRegistry{};
	UIAnimSystem*           uiAnimSys{};
	GameModeDirectorSystem* director{};
};

NS_END