#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL BattleControllerSystem : public ISystem, public IGuiRenderable
{
public:
	explicit BattleControllerSystem(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;
	void     Update(float dt); 

	void  SetConfig(const ControllerConfig& newConfig) { config = newConfig; }
	const ControllerConfig&  GetConfig()  const        { return config; }
	const ControllerRuntime& GetRuntime() const        { return runtime; }
	
	void OnGaugeBecameFull();
    void OnActionExecutionStarted(EntityID entity, const TimelineActionIntent& startIntent);
	void OnActionExecutionFinished(EntityID entity, const TimelineActionIntent& finishIntent);
    void RenderGui(EntityID id) override;

    void OnComboStepStarted(EntityID entity, SkillSlotTag slot, int stepIdx);

private:
    bool IsGaugeFull(EntityID entity)      const { return timelineSys->IsGaugeFull(entity); }
    bool IsUnitReadyToAct(EntityID entity) const { return timelineSys->IsUnitReadyToAct(entity); }

    TimelineActionIntent BuildIntent_Basic(EntityID leaderEntity);
    TimelineActionIntent BuildIntent_Skill(EntityID leaderEntity, SkillSlotTag slot);
    TimelineActionIntent BuildIntent_Defend(EntityID leaderEntity);
    TimelineActionIntent BuildIntent_Escape(EntityID leaderEntity);
    TimelineActionIntent BuildIntent_ItemRush(EntityID leaderEntity);
    EntityID             ResolveSingleTarget(EntityID leaderEntity) const;

    bool PushToBuffer(const TimelineActionIntent& intent);
    bool HasBuffered() const { return runtime.buffered.hasValue; }
    void ClearBuffer()       { runtime.buffered = {}; }
    void ResetTurnVisuals()  { runtime.queuedSkillSlotFlags = { false, false, false, false }; }

    void HandleLeaderSwitching();
    void HandleDefendHold();
    void HandleEscapeHold(float t);
    void HandleActionMenusAndCommit(bool isReady, bool isComboWindow);
    void HandleSkillPage(bool isReady, bool isComboWindow);
    void HandlePrimaryPage(bool isReady);
    // -----
    void     CleanupPrevLeaderIfDefending(EntityID prevLeader);
    EntityID PickAllyByIdx(int idx) const;

private:
    ControllerConfig      config;
    ControllerRuntime     runtime;

private:
	SystemRegistry&        registry;
    BattleTimelineSystem*  timelineSys{};
    BattleSessionSystem*   sessionSys{};
    InputService*          input{};
    BattleExecutionSystem* execSys{};
    ActionAnimRegistry*    animReg{};
    CharacterDataSystem*   dataSys{};
    SoundSystem*           soundSys{};
};

NS_END