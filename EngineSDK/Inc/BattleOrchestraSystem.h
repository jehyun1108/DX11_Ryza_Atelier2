#pragma once

#include "BattleOrchestraData.h"

NS_BEGIN(Engine)
class BattleUIOrchestrator;

class ENGINE_DLL BattleOrchestraSystem : public IModeOrchestrator, public ISystem
{
public:
	explicit BattleOrchestraSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter() override;
	void Update(float dt) override;
	void Exit() override;

	bool BeginBattle(const BattleStartParams& Inparams);

private:
	void WireSubscriptions();   
	void UnwireSubscriptions(); 

	void WireCameraSubscriptions();
	void UnwireCameraSubscriptions();
	
	void PumpSessionEventsToBus();
	void PumpTimelineEventsToBus();

	bool TryFillIntentFromTimeline(EntityID entity, TimelineActionIntent& outIntent) const;
	bool TryFillApSnapShot(EntityID entity, int& outCurAp, int& outMaxAp) const;

private:
	SystemRegistry&            registry;
	BattleEventBus*            eventBus{};
	BattleUIOrchestrator*      uiOrchestrator{};
	InputService*              input{};
	BattleCameraDirector*      camDirector{};
	CamRegistry*               camReg{};
	CameraSystem*              camSys{};
	BattleSessionSystem*       sessionSys{};
	BattleIntroSystem*         introSys{};
	BattleControllerSystem*    ctrlSys{};
	BattleTimelineSystem*      timelineSys{};
	BattleAIControllerSystem*  aiCtrlSys{};
	BattleExecutionSystem*     execSys{};
	TransformSystem*           tfSys{};
	AnimatorSystem*            animator{};
	MoveStateSystem*           moveSys{};
	CharacterDataSystem*       dataSys{};
	BattleTargetSystem*        targetSys{};


	vector<BattleEventListenerId> listenerIds;

	BattleCameraDirector::SeqSampleFunc seqSampler{};
};

NS_END