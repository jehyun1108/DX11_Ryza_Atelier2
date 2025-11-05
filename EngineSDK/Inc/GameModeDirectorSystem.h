#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL GameModeDirectorSystem : public ISystem
{
public:
	explicit GameModeDirectorSystem(SystemRegistry& registry);
	void     OnBoot() override;

	void     Update(float dt);
	void     RequestSwitch(GameMode nextMode);
	bool     BeginBattle(const BattleStartParams& startParams);
	
	GameMode GetMode() const { return curMode; }
	void     Start()         { fieldSys->Enter(); }

private:
	SystemRegistry&        registry;
	FieldOrchestraSystem*  fieldSys;
	BattleOrchestraSystem* battleSys;
	GameMode               curMode;
	EntityID               playerLeader{};
};

NS_END