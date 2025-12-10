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
	void WireSubs();
	void UnwireSubs();

	void WireCamSubs();
	void UnwireCamSubs() {}
	
	void PumpSessionEventsToBus();
	void PumpTimelineEventsToBus();

	void TickHitReacts(float dt);
	bool TryFillIntentFromTimeline(EntityID entity, TimelineActionIntent& outIntent) const;
	bool TryFillApSnapShot(EntityID entity, int& outCurAp, int& outMaxAp) const;
	void PlayHitReaction(EntityID target, const EventPayload_Damage& dmg);
	bool IsLastEnemyDown() const;

private:
	struct HitReactRuntime
	{
		EntityID entity = 0u;
		float    remaining = 0.f;
	};

	vector<_uint> listenerIds;
	BattleCameraDirector::SeqSampleFunc seqSampler{};

	bool victorySequenceStarted = false;
	vector<EntityID> deadEntities;
	vector<HitReactRuntime> hitReacts;

private:
	SystemRegistry&           registry;
	BattleEventBus*           eventBus{};
	BattleUIOrchestrator*     uiOrchestrator{};
	InputService*             input{};
	BattleCameraDirector*     camDirector{};
	CamRegistry*              camReg{};
	CameraSystem*             camSys{};
	BattleSessionSystem*      sessionSys{};
	BattleIntroSystem*        introSys{};
	BattleControllerSystem*   ctrlSys{};
	BattleTimelineSystem*     timelineSys{};
	BattleAIControllerSystem* aiCtrlSys{};
	BattleExecutionSystem*    execSys{};
	TransformSystem*          tfSys{};
	AnimatorSystem*           animator{};
	MoveStateSystem*          moveSys{};
	CharacterDataSystem*      dataSys{};
	BattleTargetSystem*       targetSys{};
	BattleTacticSystem*       tacticSys{};
	BattleAttributeSystem*    attributeSys{};
	AnimDataSystem*           animSys{};
	BattleFormationSystem*    formSys{};
	ScreenFadeSystem*         fadeSys{};
	EntityMgr*                entityMgr{};
	BattleRewardPresenter*    rewardPresenter{};
	SoundSystem*              soundSys{};
	BattleDamagePresenter*    dmgPresenter{};
}; 

NS_END