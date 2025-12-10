#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL BattleExecutionSystem : public ISystem, public IGuiRenderable
{
public:
	explicit BattleExecutionSystem(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

	bool BeginAction(EntityID entity, const TimelineActionIntent& intent, bool useWrapper);
	void Tick(float dt);

    bool                 IsComboInputOpen(EntityID entity) const;
    const AnimChainSpec& GetChain(CharacterID id, AnimContext ctx, SpecialAnimTag tag) const { return actionReg->GetSpecial(id, tag); }
    void                 NotifyComboQueued(EntityID entity, SpecialAnimTag nextTag);

    void RenderGui(EntityID id) override;
    bool IsComboStepTag(SpecialAnimTag t) const;
    bool CanQueueCombo(EntityID entity) const;
    bool IsActing(EntityID entity) const;

    void OnUnitRemoved(EntityID entity);
    int  GetComboCount() const { return comboCount; }
    void ResetCombo()          { comboCount = 0; }

private:
    bool PlayStage(EntityID entity, ExecutionUnitRunTime& runtime, const AnimChainSpec& chain);
    bool IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& runtime, const AnimChainSpec& chain) const;

    void BuildPlanForAttack(EntityID entity, ExecutionUnitRunTime& rt, const TimelineActionIntent& intent, bool useWrapper);
    void BuildPlanForDefend(EntityID entity, ExecutionUnitRunTime& rt);
    void BuildPlanForEscape(EntityID entity, ExecutionUnitRunTime& rt);
    
    bool AdvanceIfStageFinished(EntityID entity, ExecutionUnitRunTime& rt, float dt);
    bool AdvanceChainOrFinish(EntityID entity,   ExecutionUnitRunTime& rt);

private:
    void   ConsumePulse(EntityID entity,  ExecutionUnitRunTime& rt, float dt);
    void   MaintainFacing(EntityID self,  ExecutionUnitRunTime& rt);
    void   FinishAndIdle(EntityID entity, ExecutionUnitRunTime& rt);
    void   EmitHit(EntityID attacker, const ExecutionUnitRunTime& rt, EntityID target, float dmgRatio, bool critical);
    float  EstimateChainDuration(EntityID entity, const ExecutionUnitRunTime& rt, const AnimChainSpec& chain) const;
    void   UpdateSfxSequence(ExecutionUnitRunTime& rt);
    float  ResolveTagDmgMul(CharacterID ch, SpecialAnimTag tag) const;

private:
    void SetUpSfxForCurTag(EntityID entity, ExecutionUnitRunTime& rt);

private:
    unordered_map<EntityID, ExecutionUnitRunTime> runtimeByEntity;
    int  comboCount = 0;
    int  comboMaxCount = 12;

private:
	SystemRegistry&         registry;
    ActionAnimRegistry*     actionReg{};
    CharacterDataSystem*    dataSys{};
    BattleTimelineSystem*   timelineSys{};
    AnimDataSystem*         animDataSys{};
    AnimatorSystem*         animator{};
    TransformSystem*        tfSys{};
    FacingForceService*     faceSrv{};
    BattleAttributeSystem*  attrSys{};
    BattleEventBus*         eventBus{};
    ActionCamRegistry*      actionCamReg{};
    BattleTargetSystem*     targetSys{};
    BattleControllerSystem* ctrlSys{};
    CamRegistry*            camReg{};
    EffectSystem*           effectSys{};
    ActionFxRegistry*       fxReg{};
    BattleSessionSystem*    sessionSys{};
    SoundSystem*            soundSys{};
    BattleDamagePresenter*  dmgPresenter{};
    BattleTacticSystem*     tacticSys{};
};

NS_END