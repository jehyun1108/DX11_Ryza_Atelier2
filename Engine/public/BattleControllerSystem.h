#pragma once

#include "BattleControllerData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleControllerSystem : public ISystem
{
public:
	explicit BattleControllerSystem(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

	void  Update(EntityID leaderEntity, float dt); 
	void  SetConfig(const ControllerConfig& newConfig) { config = newConfig; }
	const ControllerConfig&  GetConfig()     const     { return config; }
	const ControllerRuntime* TryGetRuntime() const     { return &runtime; }
	
	void OnGaugeBecameFull();
	void OnActionExecutionStarted(const TimelineActionIntent& startIntent) { runtime.isExecuting = true; }
	void OnActionExecutionFinished(const TimelineActionIntent& finishIntent);
    void SubmitIntent(const TimelineActionIntent& intent)                  { (void)SubmitAccordingToPolicy(intent); }

private:
    bool IsGaugeFull(EntityID entity)      const { return timelineSys->IsGaugeFull(entity); }
    bool IsUnitReadyToAct(EntityID entity) const { return timelineSys->IsUnitReadyToAct(entity); }

    bool BuildIntent_Basic(EntityID leaderEntity, TimelineActionIntent& outIntent);
    bool BuildIntent_Skill(EntityID leaderEntity, SpecialAnimTag tag, TimelineActionIntent& outIntent);
    bool BuildIntent_Defend(EntityID leaderEntity, TimelineActionIntent& outIntent);
    bool BuildIntent_Escape(EntityID leaderEntity, TimelineActionIntent& outIntent);
    bool ResolveSingleTarget(EntityID leaderEntity, EntityID& outTarget) const;

    bool SubmitAccordingToPolicy(const TimelineActionIntent& intent) { return timelineSys->TryCommitIntent(runtime.leaderEntity, intent); }
    void PushToBuffer(const TimelineActionIntent& intent);
    bool HasBuffered()                                const { return runtime.buffered.hasValue; }
    void ClearBuffer()                                      { runtime.buffered = {}; }
    bool IsSkillAvailableThisTurn(SpecialAnimTag tag) const { return (runtime.turn.usedTagsThisTurn.find(tag) == runtime.turn.usedTagsThisTurn.end()); }
    void MarkSkillUsedThisTurn(SpecialAnimTag tag)          { runtime.turn.usedTagsThisTurn.insert(tag); }
    void ResetTurnVisuals()                                 { runtime.queuedSkillSlotFlags = { false, false, false, false }; }
    // ----------------------------------------------------------------------------------------------------------------
    void HandleLeaderSwitching();
    void HandleDefendHold(float t);
    void HandleEscapeHold(float t);
    void HandleActionMenusAndCommit(bool isReady);
    void HandleSkillPage(bool isReady);
    void HandlePrimaryPage(bool isReady);
    // -----
    void     CleanupPrevLeaderIfDefending(EntityID prevLeader);
    EntityID PickAllyByIdx(int idx) const;

private:
	SystemRegistry&        registry;
    BattleTimelineSystem*  timelineSys{};
    BattleSessionSystem*   sessionSys{};
    InputService*          input{};
    BattleExecutionSystem* execSys{};

	ControllerConfig      config;
	ControllerRuntime     runtime;
};

NS_END