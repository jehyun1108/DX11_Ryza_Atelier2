#pragma once

#include "BattleExecutionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleExecutionSystem
{
public:
	explicit BattleExecutionSystem(SystemRegistry& registry) : registry(registry) {}

	// Timeline 에서 ActionCommitted 이벤트를 받았을때 호출
	bool BeginAction(EntityID entity, const TimelineActionIntent& intent, ExecutionUnitRunTime& runtime);
	void Tick(EntityID entity, float dt, ExecutionUnitRunTime& runtime);

private:
	const AnimChainSpec* ResolveActiveChain(const ActionAnimSpec& spec, const TimelineActionIntent& intent) const;
	bool   PlayStage(EntityID entity, ExecutionUnitRunTime& runtime, const AnimChainSpec& chain);
	bool   IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& runtime) const;
	Handle ResolveAnimHandle(EntityID entity) const;

private:
	SystemRegistry& registry;
};

NS_END