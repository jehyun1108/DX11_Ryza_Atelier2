#pragma once

#include "BattleFatalDriveData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleFatalDriveSystem : public ISystem
{
public:
	explicit BattleFatalDriveSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();

	void SetConfig(const FatalDriveConfig& cfg) { config = cfg; }
	void StartActivation();

private:
	void EnsureInstances();
	void UpdateRing(float dt);
	void ShowPortrait();

private:
	SystemRegistry&  registry;
	UIRegistry*      uiRegistry{};
	UIAnimSystem*    uiAnimSys{};
	FatalDriveConfig config{};

	bool  active        = false;
	float progress      = 0.f;
	bool  portraitShown = false;
};

NS_END