#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL GameModeDirectorSystem
{
public:
	explicit GameModeDirectorSystem(SystemRegistry& registry, EntityID fieldLeaderEntity);

	void Update(float dt);
	void RequestSwitch(GameMode nextMode);
	bool BeginBattle(const BattleStartParams& startParams);
	GameMode GetMode() const { return curMode; }

private:
	SystemRegistry&       registry;
	FieldOrchestraSystem  fieldSys;
	BattleOrchestraSystem battleSys;
	GameMode curMode;
	EntityID playerLeader{};
};

NS_END