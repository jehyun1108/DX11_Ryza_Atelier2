#pragma once

#include "BattleOrchestraData.h"

NS_BEGIN(Engine)
class BattleUIOrchestrator;

class ENGINE_DLL BattleOrchestraSystem : public IModeOrchestrator
{
public:
	explicit BattleOrchestraSystem(SystemRegistry& registry) : registry(registry) {}

	void Enter() override;
	void Update(float dt) override;
	void Exit() override;

	bool BeginBattle(const BattleStartParams& Inparams);

private:
	void WireSubscriptions();   
	void UnwireSubscriptions(); 
	
	void PumpSessionEventsToBus();
	void PumpTimelineEventsToBus();

	bool TryFillIntentFromTimeline(EntityID entity, TimelineActionIntent& outIntent) const;
	bool TryFillApSnapShot(EntityID entity, int& outCurAp, int& outMaxAp) const;

private:
	SystemRegistry& registry;
	BattleEventBus eventBus;
	vector<BattleEventListenerId> listenerIds;
	unique_ptr<BattleUIOrchestrator> uiOrchestrator{};
};

NS_END