#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL LogoOrchestraSystem : public ISystem
{
public:
	explicit LogoOrchestraSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();

private:
	SystemRegistry&     registry;
	LogoUIOrchestrator* uiOrchestrator{};
	InputService*       input{};
};

NS_END