#pragma once

#include "BattleControllerData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleControllerSystem 
{
public:
	explicit BattleControllerSystem(SystemRegistry& registry) : registry(registry) {}

	void Update(EntityID leaderEntity, float dt); 

	void  SetConfig(const ControllerConfig& newConfig) { config = newConfig; }
	const ControllerConfig&  GetConfig()     const     { return config; }
	const ControllerRuntime* TryGetRuntime() const     { return &runtime; }
	
	// Timeline °Ê Controller
	void OnGaugeBecameFull();
	
	//  Execution °Ê Controller
	void OnActionExecutionStarted(const TimelineActionIntent& startIntent)   { runtime.isExecuting = true; }
	void OnActionExecutionFinished(const TimelineActionIntent& finishIntent);

	void HandlePrimaryKeyDown(KEY pressedKey);
	void HandleDefendHoldBegin()  { runtime.isDefendingHold = true; }
	void HandleDefendHoldEnd()    { runtime.isDefendingHold = false; }
	void EnterSelectingSkill()    { runtime.mode = ControllerMode::Skill; }
	void ExitSelectingSkill()     { runtime.mode = ControllerMode::Idle; }
	void HandleSkillSlotKeyDown(KEY pressedKey);

	// Escape, Leader
	void TryEscape();
	void StartSwapLeader(EntityID newLeaderEntity);
	void SubmitIntent(const TimelineActionIntent& intent);

private:
	// Query
	bool IsGaugeFull(EntityID entity) const;

	// Intent 
	bool BuildIntent_Basic(EntityID leaderEntity, TimelineActionIntent& outIntent);
	bool BuildIntent_Skill(EntityID leaderEntity, SpecialAnimTag specialTag, TimelineActionIntent& outIntent);
	bool BuildIntent_Defend(EntityID leaderEntity, TimelineActionIntent& outIntent);
	bool BuildIntent_Escape(EntityID leaderEntity, TimelineActionIntent& outIntent);
	bool ResolveSingleTarget(EntityID leaderEntity, EntityID& outTarget) const;

	// Submit Policy
	bool SubmitAccordingToPolicy(const TimelineActionIntent& intent);
	void PushToBuffer(const TimelineActionIntent& intent);
	bool HasBuffered() const { return runtime.buffered.hasValue; }
	void ClearBuffer() { runtime.buffered = {}; }

	// ≈œ
	bool IsSkillAvailableThisTurn(SpecialAnimTag tag) const { return (runtime.turn.usedTagsThisTurn.find(tag) == runtime.turn.usedTagsThisTurn.end()); }
	void MarkSkillUsedThisTurn(SpecialAnimTag tag)          { runtime.turn.usedTagsThisTurn.insert(tag); }
	void ResetTurnVisuals()                                 { runtime.queuedSkillSlotFlags = { false, false, false, false }; }

	// Slot «•Ω√
	int  FindQuickSlotIdxByKey(SpecialAnimTag tag) const;
	void MarkSkillSlotQueuedByKey(SpecialAnimTag tag);

private:
	SystemRegistry&   registry;
	ControllerConfig  config;
	ControllerRuntime runtime;
};

NS_END