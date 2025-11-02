#pragma once

NS_BEGIN(Engine)
class CharacterUIAgent;

class ENGINE_DLL BattleUIOrchestrator
{
public:
	BattleUIOrchestrator(SystemRegistry& registry, BattleEventBus& bus)
		: registry(registry), bus(bus) , uiRegistry(registry.Get<UIRegistry>()), uiSys(registry.Get<UISystem>()),  uiAnimSys(registry.Get<UIAnimSystem>()) {}

	void Enter();
	void Tick(float dt);
	void Exit();

private:
	SystemRegistry&      registry;
	BattleEventBus&      bus;
	UIRegistry&          uiRegistry;
	UISystem&            uiSys;
	UIAnimSystem&        uiAnimSys;
};

NS_END