#pragma once

NS_BEGIN(Engine)
class CharacterUIAgent;
class BattleTimelinePresenter;

class ENGINE_DLL BattleUIOrchestrator
{
public:
	BattleUIOrchestrator(SystemRegistry& registry)
		: registry(registry), bus(registry.Get<BattleEventBus>())
		, uiRegistry(registry.Get<UIRegistry>()), uiSys(registry.Get<UISystem>()),  uiAnimSys(registry.Get<UIAnimSystem>()) {}

	void Enter();
	void Tick(float dt);
	void Exit();

private:
	SystemRegistry&      registry;
	BattleEventBus&      bus;
	UIRegistry&          uiRegistry;
	UISystem&            uiSys;
	UIAnimSystem&        uiAnimSys;

	unique_ptr<BattleTimelinePresenter> timelinePresenter{};
};

NS_END