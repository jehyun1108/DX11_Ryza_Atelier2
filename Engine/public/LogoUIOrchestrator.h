#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL LogoUIOrchestrator : public ISystem
{
public:
	explicit LogoUIOrchestrator(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     Enter();
	void     Tick(float dt);
	void     Exit();

private:
	SystemRegistry&         registry;
	UIRegistry*             uiRegistry{};
	UISystem*               uiSys{};
	UIAnimSystem*           uiAnimSys{};
	InputService*           input{};
	GameModeDirectorSystem* director{};
	LogoMenuPresenter*      menuPresenter{};
	SoundSystem*            soundSys{};
};

NS_END