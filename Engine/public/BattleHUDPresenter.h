#pragma once

#include "BattleHUDPresenterData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleHUDPresenter : public ISystem
{
public:
	explicit BattleHUDPresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     Enter();
	void     Tick(float dt);
	void     Exit() { UnWireSubs(); }

private:
	void EnsureInstances();
	void EnsureBars();
	void EnsureTacticBars();

	void WireSubs();
	void UnWireSubs();

	void RefreshLeaderAndParty();
	void ApplyLeaderPortrait();
	void ApplyPartyPortraits();

	void SyncAllImmediate();
	void UpdateHPFront();
	void UpdateStunFront();
	void UpdateTexts();

	void ApplyTacticFromState(int level, int pips, bool maxBlink);
	void OnTacticEvent(const EventPayload_Tactic& p);
	void TickTacticReveal(float dt);
	void TickTacticBlink(float dt);
	void TickPinch(float dt);
	void TickDamageGlow(float dt);

	void ShowLevelUpBanner();
	void TickLevelUpBanner(float dt);

	void OnDamageShake(EntityID attacker, EntityID target, int dmg, bool critical);
	bool IsEntityOnRightSide(EntityID entity) const;

	void SetNumberSlot(DigitSlot& slot, int value);

private:
	HUDConfig             config{};
	HUDRuntime            rt{};
	vector<_uint>         listenerIds;
	bool                  wired = false;

private:
	SystemRegistry&        registry;
	UIRegistry*            uiRegistry{};
	UIAnimSystem*          uiAnimSys{};
	CharacterDataSystem*   dataSys{};
	BattleTimelineSystem*  timelineSys{};
	BattleSessionSystem*   sessionSys{};
	BattleEventBus*        eventBus{};
	BattleTacticSystem*    tacticSys{};
	BattleAttributeSystem* attributeSys{};
};

NS_END