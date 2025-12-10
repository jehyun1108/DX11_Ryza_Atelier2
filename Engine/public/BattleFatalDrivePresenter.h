#pragma once

#include "BattleFatalDriveData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleFatalDrivePresenter : public ISystem
{
public:
	explicit BattleFatalDrivePresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();

	void SetConfig(const FatalDriveConfig& cfg) { config = cfg; }
	void StartActivation();

private:
	void EnsureInstances();
	void ShowPortrait();

	void WireSubs();
	void UnWireSubs();
	void OnBusEvent(const BattleEvent& event);

	void ShowFatalText();
	void HideFatalText();

	void BeginLetterReveal();
	void TickLetterReveal(float dt);
	void StartLetterAt(int idx);
	void FadeOutLetters(); 
	void HideLetters();

private:
	FatalDriveRuntime rt{};
	LetterRevealSpec  letterSpec{};
	FatalDriveConfig  config{};

	bool wired = false;
	vector<_uint> listenerIds;

private:
	SystemRegistry&       registry;
	UIRegistry*           uiRegistry{};
	UIAnimSystem*         uiAnimSys{};
	BattleEventBus*       eventBus{};
	BattleTimelineSystem* timelineSys{};
	CharacterDataSystem*  dataSys{};
};

NS_END