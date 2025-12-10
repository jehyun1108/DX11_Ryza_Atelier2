#pragma once

#include "InputPresenterData.h"

NS_BEGIN(Engine)

class ENGINE_DLL PlayerInputPresenter : public ISystem
{
public:
	explicit PlayerInputPresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();
	void SetAnyPrimaryKeyHeld(bool on) { rt.anyKeyHeld = on; }

private:
	void EnsureSlots(); 
	void ApplyVisibility(bool ready);
	void ApplyEnable();

	void WireSubs();
	void UnWireSubs();

	void OnLeaderChanged();
	void OnFullGauge();
	void OnApChanged();
	void OnActionCommitted() {}
	void OnActionFinished();

	bool ResolveReady() const;
	bool DefendAllowed() const;

	void SetAlphaSlot(int idx, float a);
	void EnableSlot(int idx, bool on);

	void RefreshViewForLeader(CommandMenuPage page, CharacterID characterId);
	void SetUpPrimaryView();
	void SetUpSkillView(CharacterID characterId);

	void UpdateHighlights(float dt);
	
private:
	PlayerInputPresenterConfig  cfg{};
	PlayerInputPresenterRuntime rt{};
	vector<_uint>               listenerIds;
	bool                        wired = false;
	unordered_map<CharacterID, SkillViewSet> skillViews;

	CommandMenuPage lastPage = CommandMenuPage::Hidden;
	CharacterID     lastChar = CharacterID::Unknown;

private:
	SystemRegistry&             registry;
	UIRegistry*                 uiRegistry{};
	UIAnimSystem*               uiAnimSys{};
	BattleTimelineSystem*       timelineSys{};
	BattleEventBus*             eventBus{};
	BattleControllerSystem*     ctrlSys{};
	UISystem*                   uiSys{};
	CharacterDataSystem*        dataSys{};
};

NS_END