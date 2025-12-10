#pragma once

#include "BattleTacticData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleTacticSystem : public ISystem
{
public:
	explicit BattleTacticSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     Reset();
	void     GainPip(int n = 1);

	const PlayerTacticState& GetState() const { return state; }

private:
	void Emit(TacticEventType type);

private:
	SystemRegistry&    registry;
	BattleEventBus*    eventBus{};
	PlayerTacticState  state{};
};

NS_END