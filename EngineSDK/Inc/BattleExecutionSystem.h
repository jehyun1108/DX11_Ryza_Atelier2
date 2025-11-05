#pragma once

#include "BattleExecutionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleExecutionSystem
{
public:
	explicit BattleExecutionSystem(SystemRegistry& registry) : registry(registry) {}

	bool BeginAction(EntityID entity, const TimelineActionIntent& intent);
	void Tick(float dt);

    const AnimChainSpec* TryGetChain(CharacterID ch, AnimContext cx, SpecialAnimTag tag) const;

private:
    bool PlayStage(EntityID entity, ExecutionUnitRunTime& runtime, const AnimChainSpec& chain);
    bool IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& runtime, const AnimChainSpec& chain) const;

    void BuildPlanForAttack(EntityID entity, ExecutionUnitRunTime& rt, const TimelineActionIntent& intent);
    void BuildPlanForDefend(EntityID entity, ExecutionUnitRunTime& rt);
    void BuildPlanForEscape(EntityID entity, ExecutionUnitRunTime& rt);
    
    bool AdvanceIfStageFinished(EntityID entity, ExecutionUnitRunTime& rt, float dt);
    bool AdvanceChainOrFinish(EntityID entity,   ExecutionUnitRunTime& rt);

private:
    Handle ResolveAnimHandle(EntityID entity) const;
    Handle ResolveTfHandle(EntityID entity) const;
    void   ConsumePulse(EntityID entity,  ExecutionUnitRunTime& rt, float dt);

    void   MaintainFacing(EntityID self,  ExecutionUnitRunTime& rt);
    void   FinishAndIdle(EntityID entity, ExecutionUnitRunTime& rt);

private:
	SystemRegistry& registry;
    unordered_map<EntityID, ExecutionUnitRunTime> runtimeByEntity;
};

NS_END