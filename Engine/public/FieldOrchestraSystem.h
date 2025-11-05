#pragma once

NS_BEGIN(Engine)
class FieldUIOrchestrator;

class ENGINE_DLL FieldOrchestraSystem : public IModeOrchestrator, public ISystem
{
public:
	explicit FieldOrchestraSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter() override;
	void Update(float dt) override;
	void Exit() override;

	void BeginBattle();

private:
	SystemRegistry&          registry;
	FieldUIOrchestrator*     uiOrchestrator{};
	InputService*            input{};
	FieldControllerSystem*   fieldCtrlSys{};
	FieldAnimSystem*         fieldAnimSys{};
	BattleSessionSystem*     sessionSys{};
	CharacterDataSystem*     dataSys{};
	GameModeDirectorSystem*  director{};


	bool prevBattleKeyDown = false;
};

NS_END