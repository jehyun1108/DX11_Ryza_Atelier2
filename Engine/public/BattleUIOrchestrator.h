#pragma once

NS_BEGIN(Engine)
class CharacterUIAgent;
class BattleTimelinePresenter;

class ENGINE_DLL BattleUIOrchestrator : public ISystem
{
public:
	explicit BattleUIOrchestrator(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();

private:
	SystemRegistry&              registry;
	BattleEventBus*              eventBus{};
	UIRegistry*                  uiRegistry{};
	UISystem*                    uiSys{};
	UIAnimSystem*                uiAnimSys{};
	BattleTimelinePresenter*     timelinePresenter{};
	PlayerInputPresenter*        inputPresenter{};
	BattleHUDPresenter*          HUDPresenter{};
	BattleFatalDrivePresenter*   fatalPresenter{};
	BattleDamagePresenter*       dmgPresenter{};
	BattleTargetHUDPresenter*    targetHUDPresenter{};
	BattleBoardPresenter*        boardPresenter{};
};

NS_END