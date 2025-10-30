#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FieldOrchestraSystem : public IModeOrchestrator
{
public:
	explicit FieldOrchestraSystem(SystemRegistry& registry) 
		: registry(registry) {}

	void Enter() override;
	void Update(float dt) override;
	void Exit() override;

	void BeginBattle();

private:
	SystemRegistry& registry;

	bool prevBattleKeyDown = false;
};

NS_END